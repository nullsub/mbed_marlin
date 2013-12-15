/* -*- c++ -*- */

/*
   Reprap firmware based on Sprinter and grbl.
   Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
   This firmware is a mashup between Sprinter and grbl.
   (https://github.com/kliment/Sprinter)
   (https://github.com/simen/grbl/tree)

   It has preliminary support for Matthew Roberts advance algorithm
http://reprap.org/pipermail/reprap-dev/2011-May/003323.html
 */

#include "Marlin.h"

#include "planner.h"
#include "stepper.h"
#include "temperature.h"
#include "motion_control.h"
#include "language.h"

#if defined(DIGIPOTSS_PIN) && DIGIPOTSS_PIN > -1
#include <SPI.h>
#endif

#define VERSION_STRING  "1.0.0"

// look here for descriptions of gcodes: http://linuxcnc.org/handbook/gcode/g-code.html
// http://objects.reprap.org/wiki/Mendel_User_Manual:_RepRapGCodes

float homing_feedrate[] = HOMING_FEEDRATE;
bool axis_relative_modes[] = AXIS_RELATIVE_MODES;
int feedmultiply=100; //100->1 200->2
int saved_feedmultiply;
int extrudemultiply=100; //100->1 200->2
float current_position[NUM_AXIS] = { 0.0, 0.0, 0.0, 0.0 };
float add_homeing[3]={0,0,0};
float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };
float base_home_pos[3] = { X_HOME_POS, Y_HOME_POS, Z_HOME_POS };
float max_length[3] = { X_MAX_LENGTH, Y_MAX_LENGTH, Z_MAX_LENGTH };
float home_dir[3] = { X_HOME_DIR, Y_HOME_DIR, Z_HOME_DIR };
float home_retract_mm[3] = { X_HOME_RETRACT_MM, Y_HOME_RETRACT_MM, Z_HOME_RETRACT_MM };

// Extruder offset
#if EXTRUDERS > 1
#ifndef DUAL_X_CARRIAGE
#define NUM_EXTRUDER_OFFSETS 2 // only in XY plane
#else
#define NUM_EXTRUDER_OFFSETS 3 // supports offsets in XYZ plane
#endif
float extruder_offset[NUM_EXTRUDER_OFFSETS][EXTRUDERS] = {
#if defined(EXTRUDER_OFFSET_X) && defined(EXTRUDER_OFFSET_Y)
	EXTRUDER_OFFSET_X, EXTRUDER_OFFSET_Y
#endif
};
#endif
uint8_t active_extruder = 0;
int fanSpeed=0;

#ifdef FWRETRACT
bool autoretract_enabled=true;
bool retracted=false;
float retract_length=3, retract_feedrate=17*60, retract_zlift=0.8;
float retract_recover_length=0, retract_recover_feedrate=8*60;
#endif


//===========================================================================
//=============================private variables=============================
//===========================================================================
const char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
static float destination[NUM_AXIS] = {  0.0, 0.0, 0.0, 0.0};
static float offset[3] = {0.0, 0.0, 0.0};
static bool home_all_axis = true;
static float feedrate = 1500.0, next_feedrate, saved_feedrate;
static long gcode_N, gcode_LastN, Stopped_gcode_LastN = 0;

static bool relative_mode = false;  //Determines Absolute or Relative Coordinates

static char cmdbuffer[BUFSIZE][MAX_CMD_SIZE];
static bool fromsd[BUFSIZE];
static int bufindr = 0;
static int bufindw = 0;
static int buflen = 0;
//static int i = 0;
static char serial_char;
static int serial_count = 0;
static bool comment_mode = false;
static char *strchr_pointer; // just a pointer to find chars in the cmd string like X, Y, Z, E, etc

const int sensitive_pins[] = SENSITIVE_PINS; // Sensitive pin list for M42

//static float tt = 0;
//static float bt = 0;

//Inactivity shutdown variables
static unsigned long previous_millis_cmd = 0;
static unsigned long max_inactive_time = 0;
static unsigned long stepper_inactive_time = DEFAULT_STEPPER_DEACTIVE_TIME*1000l;

unsigned long starttime=0;
unsigned long stoptime=0;

static uint8_t tmp_extruder;


bool Stopped=false;

bool CooldownNoWait = true;
bool target_direction;

//===========================================================================
//=============================ROUTINES=============================
//===========================================================================

void get_arc_coordinates();
bool setTargetedHotend(int code);

void serial_echopair_P(const char *s_P, float v)
{ serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char *s_P, double v)
{ serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char *s_P, unsigned long v)
{ serialprintPGM(s_P); SERIAL_ECHO(v); }

extern "C"{
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

int freeMemory() {
	/*  int free_memory;

	    if((int)__brkval == 0)
	    free_memory = ((int)&free_memory) - ((int)&__bss_end);
	    else
	    free_memory = ((int)&free_memory) - ((int)__brkval);
	 */
	//return free_memory;
	return 1;
}
}

//adds an command to the main command buffer
//thats really done in a non-safe way.
//needs overworking someday
void enquecommand(const char *cmd)
{
	if(buflen < BUFSIZE)
	{
		//this is dangerous if a mixing of serial and this happsens
		strcpy(&(cmdbuffer[bufindw][0]),cmd);
		SERIAL_ECHO_START;
		SERIAL_ECHOPGM("enqueing \"");
		SERIAL_ECHO(cmdbuffer[bufindw]);
		SERIAL_ECHOLNPGM("\"");
		bufindw= (bufindw + 1)%BUFSIZE;
		buflen += 1;
	}
}

void enquecommand_P(const char *cmd)
{
	if(buflen < BUFSIZE)
	{
		//this is dangerous if a mixing of serial and this happsens
		strcpy(&(cmdbuffer[bufindw][0]),cmd);
		SERIAL_ECHO_START;
		SERIAL_ECHOPGM("enqueing \"");
		SERIAL_ECHO(cmdbuffer[bufindw]);
		SERIAL_ECHOLNPGM("\"");
		bufindw= (bufindw + 1)%BUFSIZE;
		buflen += 1;
	}
}

void setup_killpin()
{
#if defined(KILL_PIN) && KILL_PIN > -1
	pinMode(KILL_PIN,INPUT);
	WRITE(KILL_PIN,HIGH);
#endif
}

void setup_powerhold()
{
#if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
	SET_OUTPUT(SUICIDE_PIN);
	WRITE(SUICIDE_PIN, HIGH);
#endif
#if defined(PS_ON_PIN) && PS_ON_PIN > -1
	SET_OUTPUT(PS_ON_PIN);
	WRITE(PS_ON_PIN, PS_ON_AWAKE);
#endif
}

void suicide()
{
#if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
	SET_OUTPUT(SUICIDE_PIN);
	WRITE(SUICIDE_PIN, LOW);
#endif
}

void setup()
{
	setup_killpin();
	setup_powerhold();
	SERIAL_PROTOCOLLNPGM("start");
	SERIAL_ECHO_START;

	SERIAL_ECHOPGM(MSG_MARLIN);
	SERIAL_ECHOLNPGM(VERSION_STRING);
#ifdef STRING_VERSION_CONFIG_H
#ifdef STRING_CONFIG_H_AUTHOR
	SERIAL_ECHO_START;
	SERIAL_ECHOPGM(MSG_CONFIGURATION_VER);
	SERIAL_ECHOPGM(STRING_VERSION_CONFIG_H);
	SERIAL_ECHOPGM(MSG_AUTHOR);
	SERIAL_ECHOLNPGM(STRING_CONFIG_H_AUTHOR);
	SERIAL_ECHOPGM("Compiled: ");
	SERIAL_ECHOLNPGM(__DATE__);
#endif
#endif
	SERIAL_ECHO_START;
	SERIAL_ECHOPGM(MSG_FREE_MEMORY);
	SERIAL_ECHO(freeMemory());
	SERIAL_ECHOPGM(MSG_PLANNER_BUFFER_BYTES);
	SERIAL_ECHOLN((int)sizeof(block_t)*BLOCK_BUFFER_SIZE);
	for(int8_t i = 0; i < BUFSIZE; i++)
	{
		fromsd[i] = false;
	}

	// loads data from EEPROM if available else uses defaults (and resets step acceleration rate)
	Config_RetrieveSettings();

	tp_init();    // Initialize temperature loop
	plan_init();  // Initialize planner;
	st_init();    // Initialize stepper, this enables interrupts!

#if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1
	SET_OUTPUT(CONTROLLERFAN_PIN); //Set pin used for driver cooling fan
#endif
}


void loop()
{
	if(buflen < (BUFSIZE-1))
		get_command();
	if(buflen)
	{
		process_commands();
		buflen = (buflen-1);
		bufindr = (bufindr + 1)%BUFSIZE;
	}
	//check heater every n milliseconds
	manage_heater();
	manage_inactivity();
	checkHitEndstops();
}

void get_command()
{
	while( MYSERIAL.readable() && buflen < BUFSIZE) {
		serial_char = MYSERIAL.getc();
		if(serial_char == '\n' ||
				serial_char == '\r' ||
				(serial_char == ':' && comment_mode == false) ||
				serial_count >= (MAX_CMD_SIZE - 1) )
		{
			if(!serial_count) { //if empty line
				comment_mode = false; //for new command
				return;
			}
			cmdbuffer[bufindw][serial_count] = 0; //terminate string
			if(!comment_mode){
				comment_mode = false; //for new command
				fromsd[bufindw] = false;
				if(strchr(cmdbuffer[bufindw], 'N') != NULL)
				{
					strchr_pointer = strchr(cmdbuffer[bufindw], 'N');
					gcode_N = (strtol(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL, 10));
					if(gcode_N != gcode_LastN+1 && (strstr(cmdbuffer[bufindw], PSTR("M110")) == NULL) ) {
						SERIAL_ERROR_START;
						SERIAL_ERRORPGM(MSG_ERR_LINE_NO);
						SERIAL_ERRORLN(gcode_LastN);
						//Serial.println(gcode_N);
						FlushSerialRequestResend();
						serial_count = 0;
						return;
					}

					if(strchr(cmdbuffer[bufindw], '*') != NULL)
					{
						char checksum = 0;
						int count = 0;
						while(cmdbuffer[bufindw][count] != '*') checksum = checksum^cmdbuffer[bufindw][count++];
						strchr_pointer = strchr(cmdbuffer[bufindw], '*');

						if( (int)(strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)) != checksum) {
							SERIAL_ERROR_START;
							SERIAL_ERRORPGM(MSG_ERR_CHECKSUM_MISMATCH);
							SERIAL_ERRORLN(gcode_LastN);
							FlushSerialRequestResend();
							serial_count = 0;
							return;
						}
						//if no errors, continue parsing
					}
					else
					{
						SERIAL_ERROR_START;
						SERIAL_ERRORPGM(MSG_ERR_NO_CHECKSUM);
						SERIAL_ERRORLN(gcode_LastN);
						FlushSerialRequestResend();
						serial_count = 0;
						return;
					}

					gcode_LastN = gcode_N;
					//if no errors, continue parsing
				}
				else  // if we don't receive 'N' but still see '*'
				{
					if((strchr(cmdbuffer[bufindw], '*') != NULL))
					{
						SERIAL_ERROR_START;
						SERIAL_ERRORPGM(MSG_ERR_NO_LINENUMBER_WITH_CHECKSUM);
						SERIAL_ERRORLN(gcode_LastN);
						serial_count = 0;
						return;
					}
				}
				if((strchr(cmdbuffer[bufindw], 'G') != NULL)){
					strchr_pointer = strchr(cmdbuffer[bufindw], 'G');
					switch((int)((strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)))){
						case 0:
						case 1:
						case 2:
						case 3:
							if(Stopped == false) { // If printer is stopped by an error the G[0-3] codes are ignored.
								SERIAL_PROTOCOLLNPGM(MSG_OK);
							}
							else {
								SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);
								LCD_MESSAGEPGM(MSG_STOPPED);
							}
							break;
						default:
							break;
					}

				}
				bufindw = (bufindw + 1)%BUFSIZE;
				buflen += 1;
			}
			serial_count = 0; //clear buffer
		}
		else
		{
			if(serial_char == ';') comment_mode = true;
			if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
		}
	}
}


float code_value()
{
	return (strtod(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL));
}

long code_value_long()
{
	return (strtol(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL, 10));
}

bool code_seen(char code)
{
	strchr_pointer = strchr(cmdbuffer[bufindr], code);
	return (strchr_pointer != NULL);  //Return True if a character was found
}


#ifdef DUAL_X_CARRIAGE
#if EXTRUDERS == 1 || defined(COREXY) \
	       || !defined(X2_ENABLE_PIN) || !defined(X2_STEP_PIN) || !defined(X2_DIR_PIN) \
|| !defined(X2_HOME_POS) || !defined(X2_MIN_POS) || !defined(X2_MAX_POS) \
|| !defined(X_MAX_PIN) || X_MAX_PIN < 0
#error "Missing or invalid definitions for DUAL_X_CARRIAGE mode."
#endif
#if X_HOME_DIR != -1 || X2_HOME_DIR != 1
#error "Please use canonical x-carriage assignment" // the x-carriages are defined by their homing directions
#endif

#define DXC_FULL_CONTROL_MODE 0
#define DXC_AUTO_PARK_MODE    1
#define DXC_DUPLICATION_MODE  2
static int dual_x_carriage_mode = DEFAULT_DUAL_X_CARRIAGE_MODE;

static float x_home_pos(int extruder) {
	if (extruder == 0)
		return base_home_pos[X_AXIS] + add_homeing[X_AXIS];
	else
		// In dual carriage mode the extruder offset provides an override of the
		// second X-carriage offset when homed - otherwise X2_HOME_POS is used.
		// This allow soft recalibration of the second extruder offset position without firmware reflash
		// (through the M218 command).
		return (extruder_offset[X_AXIS][1] > 0) ? extruder_offset[X_AXIS][1] : X2_HOME_POS;
}

static int x_home_dir(int extruder) {
	return (extruder == 0) ? X_HOME_DIR : X2_HOME_DIR;
}

static float inactive_extruder_x_pos = X2_MAX_POS; // used in mode 0 & 1
static bool active_extruder_parked = false; // used in mode 1 & 2
static float raised_parked_position[NUM_AXIS]; // used in mode 1
static unsigned long delayed_move_time = 0; // used in mode 1
static float duplicate_extruder_x_offset = DEFAULT_DUPLICATION_X_OFFSET; // used in mode 2
static float duplicate_extruder_temp_offset = 0; // used in mode 2
bool extruder_duplication_enabled = false; // used in mode 2
#endif //DUAL_X_CARRIAGE

static void axis_is_at_home(int axis) {
#ifdef DUAL_X_CARRIAGE
	if (axis == X_AXIS) {
		if (active_extruder != 0) {
			current_position[X_AXIS] = x_home_pos(active_extruder);
			min_pos[X_AXIS] =          X2_MIN_POS;
			max_pos[X_AXIS] =          max(extruder_offset[X_AXIS][1], X2_MAX_POS);
			return;
		}
		else if (dual_x_carriage_mode == DXC_DUPLICATION_MODE && active_extruder == 0) {
			current_position[X_AXIS] = base_home_pos[X_AXIS] + add_homeing[X_AXIS];
			min_pos[X_AXIS] =          min_pos[X_AXIS] + add_homeing[X_AXIS];
			max_pos[X_AXIS] =          min(base_max_pos(X_AXIS) + add_homeing[X_AXIS],
					max(extruder_offset[X_AXIS][1], X2_MAX_POS) - duplicate_extruder_x_offset);
			return;
		}
	}
#endif
	//did not use BASE_MIN_POS. is that a problem?
	current_position[axis] = base_home_pos[axis] + add_homeing[axis];
	min_pos[axis] =          min_pos[axis] + add_homeing[axis];
	max_pos[axis] =          max_pos[axis] + add_homeing[axis];
}

static void homeaxis(int axis) {
#define HOMEAXIS_DO(LETTER) \
	((LETTER##_MIN_PIN > -1 && LETTER##_HOME_DIR==-1) || (LETTER##_MAX_PIN > -1 && LETTER##_HOME_DIR==1))

	if (axis==X_AXIS ? HOMEAXIS_DO(X) :
			axis==Y_AXIS ? HOMEAXIS_DO(Y) :
			axis==Z_AXIS ? HOMEAXIS_DO(Z) :
			0) {
		int axis_home_dir = home_dir[axis];
#ifdef DUAL_X_CARRIAGE
		if (axis == X_AXIS)
			axis_home_dir = x_home_dir(active_extruder);
#endif

		// Engage Servo endstop if enabled
#ifdef SERVO_ENDSTOPS
		if (servo_endstops[axis] > -1) {
			servos[servo_endstops[axis]].write(servo_endstop_angles[axis * 2]);
		}
#endif

		current_position[axis] = 0;
		plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
		destination[axis] = 1.5 * max_length[axis] * axis_home_dir;
		feedrate = homing_feedrate[axis];
		plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		st_synchronize();

		current_position[axis] = 0;
		plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
		destination[axis] = -home_retract_mm[axis] * axis_home_dir;
		plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		st_synchronize();

		destination[axis] = 2*home_retract_mm[axis] * axis_home_dir;
		feedrate = homing_feedrate[axis]/2 ;
		plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		st_synchronize();
		axis_is_at_home(axis);
		destination[axis] = current_position[axis];
		feedrate = 0.0;
		endstops_hit_on_purpose();

		// Retract Servo endstop if enabled
#ifdef SERVO_ENDSTOPS
		if (servo_endstops[axis] > -1) {
			servos[servo_endstops[axis]].write(servo_endstop_angles[axis * 2 + 1]);
		}
#endif
	}
}
#define HOMEAXIS(LETTER) homeaxis(LETTER##_AXIS)

void process_commands()
{
	unsigned long codenum; //throw away variable
	char *starpos = NULL;

	if(code_seen('G'))
	{
		switch((int)code_value())
		{
			case 0: // G0 -> G1
			case 1: // G1
				if(Stopped == false) {
					get_coordinates(); // For X Y Z E F
					prepare_move();
					//ClearToSend();
					return;
				}
				//break;
			case 2: // G2  - CW ARC
				if(Stopped == false) {
					get_arc_coordinates();
					prepare_arc_move(true);
					return;
				}
			case 3: // G3  - CCW ARC
				if(Stopped == false) {
					get_arc_coordinates();
					prepare_arc_move(false);
					return;
				}
			case 4: // G4 dwell
				LCD_MESSAGEPGM(MSG_DWELL);
				codenum = 0;
				if(code_seen('P')) codenum = code_value(); // milliseconds to wait
				if(code_seen('S')) codenum = code_value() * 1000; // seconds to wait

				st_synchronize();
				codenum += millis();  // keep track of when we started waiting
				previous_millis_cmd = millis();
				while(millis()  < codenum ){
					manage_heater();
					manage_inactivity();
				}
				break;
#ifdef FWRETRACT
			case 10: // G10 retract
				if(!retracted)
				{
					destination[X_AXIS]=current_position[X_AXIS];
					destination[Y_AXIS]=current_position[Y_AXIS];
					destination[Z_AXIS]=current_position[Z_AXIS];
					current_position[Z_AXIS]+=-retract_zlift;
					destination[E_AXIS]=current_position[E_AXIS]-retract_length;
					feedrate=retract_feedrate;
					retracted=true;
					prepare_move();
				}

				break;
			case 11: // G10 retract_recover
				if(!retracted)
				{
					destination[X_AXIS]=current_position[X_AXIS];
					destination[Y_AXIS]=current_position[Y_AXIS];
					destination[Z_AXIS]=current_position[Z_AXIS];

					current_position[Z_AXIS]+=retract_zlift;
					current_position[E_AXIS]+=-retract_recover_length;
					feedrate=retract_recover_feedrate;
					retracted=false;
					prepare_move();
				}
				break;
#endif //FWRETRACT
			case 28: //G28 Home all Axis one at a time
				saved_feedrate = feedrate;
				saved_feedmultiply = feedmultiply;
				feedmultiply = 100;
				previous_millis_cmd = millis();

				enable_endstops(true);

				for(int8_t i=0; i < NUM_AXIS; i++) {
					destination[i] = current_position[i];
				}
				feedrate = 0.0;

				home_all_axis = !((code_seen(axis_codes[0])) || (code_seen(axis_codes[1])) || (code_seen(axis_codes[2])));

#if Z_HOME_DIR > 0                      // If homing away from BED do Z first
				if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) {
					HOMEAXIS(Z);
				}
#endif

#ifdef QUICK_HOME
				if((home_all_axis)||( code_seen(axis_codes[X_AXIS]) && code_seen(axis_codes[Y_AXIS])) )  //first diagonal move
				{
					current_position[X_AXIS] = 0;current_position[Y_AXIS] = 0;

#ifndef DUAL_X_CARRIAGE
					int x_axis_home_dir = home_dir(X_AXIS);
#else
					int x_axis_home_dir = x_home_dir(active_extruder);
					extruder_duplication_enabled = false;
#endif

					plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
					destination[X_AXIS] = 1.5 * max_length[X_AXIS] * x_axis_home_dir;destination[Y_AXIS] = 1.5 * max_length[Y_AXIS] * home_dir(Y_AXIS);
					feedrate = homing_feedrate[X_AXIS];
					if(homing_feedrate[Y_AXIS]<feedrate)
						feedrate =homing_feedrate[Y_AXIS];
					plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
					st_synchronize();

					axis_is_at_home(X_AXIS);
					axis_is_at_home(Y_AXIS);
					plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
					destination[X_AXIS] = current_position[X_AXIS];
					destination[Y_AXIS] = current_position[Y_AXIS];
					plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
					feedrate = 0.0;
					st_synchronize();
					endstops_hit_on_purpose();

					current_position[X_AXIS] = destination[X_AXIS];
					current_position[Y_AXIS] = destination[Y_AXIS];
					current_position[Z_AXIS] = destination[Z_AXIS];
				}
#endif

				if((home_all_axis) || (code_seen(axis_codes[X_AXIS])))
				{
#ifdef DUAL_X_CARRIAGE
					int tmp_extruder = active_extruder;
					extruder_duplication_enabled = false;
					active_extruder = !active_extruder;
					HOMEAXIS(X);
					inactive_extruder_x_pos = current_position[X_AXIS];
					active_extruder = tmp_extruder;
					HOMEAXIS(X);
					// reset state used by the different modes
					memcpy(raised_parked_position, current_position, sizeof(raised_parked_position));
					delayed_move_time = 0;
					active_extruder_parked = true;
#else
					HOMEAXIS(X);
#endif
				}

				if((home_all_axis) || (code_seen(axis_codes[Y_AXIS]))) {
					HOMEAXIS(Y);
				}

#if Z_HOME_DIR < 0                      // If homing towards BED do Z last
				if((home_all_axis) || (code_seen(axis_codes[Z_AXIS]))) {
					HOMEAXIS(Z);
				}
#endif

				if(code_seen(axis_codes[X_AXIS]))
				{
					if(code_value_long() != 0) {
						current_position[X_AXIS]=code_value()+add_homeing[0];
					}
				}

				if(code_seen(axis_codes[Y_AXIS])) {
					if(code_value_long() != 0) {
						current_position[Y_AXIS]=code_value()+add_homeing[1];
					}
				}

				if(code_seen(axis_codes[Z_AXIS])) {
					if(code_value_long() != 0) {
						current_position[Z_AXIS]=code_value()+add_homeing[2];
					}
				}
				plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

#ifdef ENDSTOPS_ONLY_FOR_HOMING
				enable_endstops(false);
#endif

				feedrate = saved_feedrate;
				feedmultiply = saved_feedmultiply;
				previous_millis_cmd = millis();
				endstops_hit_on_purpose();
				break;
			case 90: // G90
				relative_mode = false;
				break;
			case 91: // G91
				relative_mode = true;
				break;
			case 92: // G92
				if(!code_seen(axis_codes[E_AXIS]))
					st_synchronize();
				for(int8_t i=0; i < NUM_AXIS; i++) {
					if(code_seen(axis_codes[i])) {
						if(i == E_AXIS) {
							current_position[i] = code_value();
							plan_set_e_position(current_position[E_AXIS]);
						}
						else {
							current_position[i] = code_value()+add_homeing[i];
							plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
						}
					}
				}
				break;
		}
	}

	else if(code_seen('M'))
	{
		switch( (int)code_value() )
		{
			case 17:
				LCD_MESSAGEPGM(MSG_NO_MOVE);
				enable_x();
				enable_y();
				enable_z();
				enable_e0();
				enable_e1();
				enable_e2();
				break;
			case 31: //M31 take time since the start of the SD print or an M109 command
				{
					stoptime=millis();
					char time[30];
					unsigned long t=(stoptime-starttime)/1000;
					int sec,min;
					min=t/60;
					sec=t%60;
					sprintf(time, PSTR("%i min, %i sec"), min, sec);
					SERIAL_ECHO_START;
					SERIAL_ECHOLN(time);
					autotempShutdown();
				}
				break;
			case 42: //M42 -Change pin status via gcode
				if (code_seen('S'))
				{
					int pin_status = code_value();
					int pin_number = LED_PIN;
					if (code_seen('P') && pin_status >= 0 && pin_status <= 255)
						pin_number = code_value();
					for(int8_t i = 0; i < (int8_t)sizeof(sensitive_pins); i++)
					{
						if (sensitive_pins[i] == pin_number)
						{
							pin_number = -1;
							break;
						}
					}
#if defined(FAN_PIN) && FAN_PIN > -1
					if (pin_number == FAN_PIN)
						fanSpeed = pin_status;
#endif
					if (pin_number > -1) {
						/*
						   pinMode(pin_number, OUTPUT);
						   digitalWrite(pin_number, pin_status);
						   analogWrite(pin_number, pin_status);
						 */
					}
				}
				break;
			case 104: // M104
				if(setTargetedHotend(104)){
					break;
				}
				if (code_seen('S')) setTargetHotend(code_value(), tmp_extruder);
#ifdef DUAL_X_CARRIAGE
				if (dual_x_carriage_mode == DXC_DUPLICATION_MODE && tmp_extruder == 0)
					setTargetHotend1(code_value() == 0.0 ? 0.0 : code_value() + duplicate_extruder_temp_offset);
#endif
				setWatch();
				break;
			case 140: // M140 set bed temp
				if (code_seen('S')) setTargetBed(code_value());
				break;
			case 105 : // M105
				if(setTargetedHotend(105)){
					break;
				}
#if defined(TEMP_0_PIN) && TEMP_0_PIN > -1
				SERIAL_PROTOCOLPGM("ok T:");
				SERIAL_PROTOCOL_F(degHotend(tmp_extruder),1);
				SERIAL_PROTOCOLPGM(" /");
				SERIAL_PROTOCOL_F(degTargetHotend(tmp_extruder),1);
#if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1
				SERIAL_PROTOCOLPGM(" B:");
				SERIAL_PROTOCOL_F(degBed(),1);
				SERIAL_PROTOCOLPGM(" /");
				SERIAL_PROTOCOL_F(degTargetBed(),1);
#endif //TEMP_BED_PIN
				for (int8_t cur_extruder = 0; cur_extruder < EXTRUDERS; ++cur_extruder) {
					SERIAL_PROTOCOLPGM(" T");
					SERIAL_PROTOCOL(cur_extruder);
					SERIAL_PROTOCOLPGM(":");
					SERIAL_PROTOCOL_F(degHotend(cur_extruder),1);
					SERIAL_PROTOCOLPGM(" /");
					SERIAL_PROTOCOL_F(degTargetHotend(cur_extruder),1);
				}
#else
				SERIAL_ERROR_START;
				SERIAL_ERRORLNPGM(MSG_ERR_NO_THERMISTORS);
#endif

				SERIAL_PROTOCOLPGM(" @:");
				SERIAL_PROTOCOL(getHeaterPower(tmp_extruder));

				SERIAL_PROTOCOLPGM(" B@:");
				SERIAL_PROTOCOL(getHeaterPower(-1));

				SERIAL_PROTOCOLLN("");
				return;
				break;
			case 109:
				{// M109 - Wait for extruder heater to reach target.
					if(setTargetedHotend(109)){
						break;
					}
					LCD_MESSAGEPGM(MSG_HEATING);
#ifdef AUTOTEMP
					autotemp_enabled=false;
#endif
					if (code_seen('S')) {
						setTargetHotend(code_value(), tmp_extruder);
#ifdef DUAL_X_CARRIAGE
						if (dual_x_carriage_mode == DXC_DUPLICATION_MODE && tmp_extruder == 0)
							setTargetHotend1(code_value() == 0.0 ? 0.0 : code_value() + duplicate_extruder_temp_offset);
#endif
						CooldownNoWait = true;
					} else if (code_seen('R')) {
						setTargetHotend(code_value(), tmp_extruder);
#ifdef DUAL_X_CARRIAGE
						if (dual_x_carriage_mode == DXC_DUPLICATION_MODE && tmp_extruder == 0)
							setTargetHotend1(code_value() == 0.0 ? 0.0 : code_value() + duplicate_extruder_temp_offset);
#endif
						CooldownNoWait = false;
					}
#ifdef AUTOTEMP
					if (code_seen('S')) autotemp_min=code_value();
					if (code_seen('B')) autotemp_max=code_value();
					if (code_seen('F'))
					{
						autotemp_factor=code_value();
						autotemp_enabled=true;
					}
#endif

					setWatch();
					codenum = millis();

					/* See if we are heating up or cooling down */
					target_direction = isHeatingHotend(tmp_extruder); // true if heating, false if cooling

#ifdef TEMP_RESIDENCY_TIME
					long residencyStart;
					residencyStart = -1;
					/* continue to loop until we have reached the target temp
					   _and_ until TEMP_RESIDENCY_TIME hasn't passed since we reached it */
					while((residencyStart == -1) ||
							(residencyStart >= 0 && (((unsigned int) (millis() - residencyStart)) < (TEMP_RESIDENCY_TIME * 1000UL))) ) {
#else
						while ( target_direction ? (isHeatingHotend(tmp_extruder)) : (isCoolingHotend(tmp_extruder)&&(CooldownNoWait==false)) ) {
#endif //TEMP_RESIDENCY_TIME
							if( (millis() - codenum) > 1000UL )
							{ //Print Temp Reading and remaining time every 1 second while heating up/cooling down
								SERIAL_PROTOCOLPGM("T:");
								SERIAL_PROTOCOL_F(degHotend(tmp_extruder),1);
								SERIAL_PROTOCOLPGM(" E:");
								SERIAL_PROTOCOL((int)tmp_extruder);
#ifdef TEMP_RESIDENCY_TIME
								SERIAL_PROTOCOLPGM(" W:");
								if(residencyStart > -1)
								{
									codenum = ((TEMP_RESIDENCY_TIME * 1000UL) - (millis() - residencyStart)) / 1000UL;
									SERIAL_PROTOCOLLN( codenum );
								}
								else
								{
									SERIAL_PROTOCOLLN( "?" );
								}
#else
								SERIAL_PROTOCOLLN("");
#endif
								codenum = millis();
							}
							manage_heater();
							manage_inactivity();
#ifdef TEMP_RESIDENCY_TIME
							/* start/restart the TEMP_RESIDENCY_TIME timer whenever we reach target temp for the first time
							   or when current temp falls outside the hysteresis after target temp was reached */
							if ((residencyStart == -1 &&  target_direction && (degHotend(tmp_extruder) >= (degTargetHotend(tmp_extruder)-TEMP_WINDOW))) ||
									(residencyStart == -1 && !target_direction && (degHotend(tmp_extruder) <= (degTargetHotend(tmp_extruder)+TEMP_WINDOW))) ||
									(residencyStart > -1 && labs(degHotend(tmp_extruder) - degTargetHotend(tmp_extruder)) > TEMP_HYSTERESIS) )
							{
								residencyStart = millis();
							}
#endif //TEMP_RESIDENCY_TIME
						}
						LCD_MESSAGEPGM(MSG_HEATING_COMPLETE);
						starttime=millis();
						previous_millis_cmd = millis();
					}
					break;
					case 190: // M190 - Wait for bed heater to reach target.
#if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1
					LCD_MESSAGEPGM(MSG_BED_HEATING);
					if (code_seen('S')) {
						setTargetBed(code_value());
						CooldownNoWait = true;
					} else if (code_seen('R')) {
						setTargetBed(code_value());
						CooldownNoWait = false;
					}
					codenum = millis();

					target_direction = isHeatingBed(); // true if heating, false if cooling

					while ( target_direction ? (isHeatingBed()) : (isCoolingBed()&&(CooldownNoWait==false)) )
					{
						if(( millis() - codenum) > 1000 ) //Print Temp Reading every 1 second while heating up.
						{
							float tt=degHotend(active_extruder);
							SERIAL_PROTOCOLPGM("T:");
							SERIAL_PROTOCOL(tt);
							SERIAL_PROTOCOLPGM(" E:");
							SERIAL_PROTOCOL((int)active_extruder);
							SERIAL_PROTOCOLPGM(" B:");
							SERIAL_PROTOCOL_F(degBed(),1);
							SERIAL_PROTOCOLLN("");
							codenum = millis();
						}
						manage_heater();
						manage_inactivity();
					}
					LCD_MESSAGEPGM(MSG_BED_DONE);
					previous_millis_cmd = millis();
#endif
					break;

#if defined(FAN_PIN) && FAN_PIN > -1
					case 106: //M106 Fan On
					if (code_seen('S')){
						//fanSpeed=constrain(code_value(),0,255);
					}
					else {
						fanSpeed=255;
					}
					break;
					case 107: //M107 Fan Off
					fanSpeed = 0;
					break;
#endif //FAN_PIN

#if defined(PS_ON_PIN) && PS_ON_PIN > -1
					case 80: // M80 - Turn on Power Supply
					SET_OUTPUT(PS_ON_PIN); //GND
					WRITE(PS_ON_PIN, PS_ON_AWAKE);
					break;
#endif

					case 81: // M81 - Turn off Power Supply
					disable_heater();
					st_synchronize();
					disable_e0();
					disable_e1();
					disable_e2();
					finishAndDisableSteppers();
					fanSpeed = 0;
					delay_ms(1000); // Wait a little before to switch off
#if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
					st_synchronize();
					suicide();
#elif defined(PS_ON_PIN) && PS_ON_PIN > -1
					SET_OUTPUT(PS_ON_PIN);
					WRITE(PS_ON_PIN, PS_ON_ASLEEP);
#endif
					break;

					case 82:
					axis_relative_modes[3] = false;
					break;
					case 83:
					axis_relative_modes[3] = true;
					break;
					case 18: //compatibility
					case 84: // M84
					if(code_seen('S')){
						stepper_inactive_time = code_value() * 1000;
					}
					else
					{
						bool all_axis = !((code_seen(axis_codes[0])) || (code_seen(axis_codes[1])) || (code_seen(axis_codes[2]))|| (code_seen(axis_codes[3])));
						if(all_axis)
						{
							st_synchronize();
							disable_e0();
							disable_e1();
							disable_e2();
							finishAndDisableSteppers();
						}
						else
						{
							st_synchronize();
							if(code_seen('X')) disable_x();
							if(code_seen('Y')) disable_y();
							if(code_seen('Z')) disable_z();
#if ((E0_ENABLE_PIN != X_ENABLE_PIN) && (E1_ENABLE_PIN != Y_ENABLE_PIN)) // Only enable on boards that have seperate ENABLE_PINS
							if(code_seen('E')) {
								disable_e0();
								disable_e1();
								disable_e2();
							}
#endif
						}
					}
					break;
					case 85: // M85
					code_seen('S');
					max_inactive_time = code_value() * 1000;
					break;
					case 92: // M92
					for(int8_t i=0; i < NUM_AXIS; i++)
					{
						if(code_seen(axis_codes[i]))
						{
							if(i == 3) { // E
								float value = code_value();
								if(value < 20.0) {
									float factor = axis_steps_per_unit[i] / value; // increase e constants if M92 E14 is given for netfab.
									max_e_jerk *= factor;
									max_feedrate[i] *= factor;
									axis_steps_per_sqr_second[i] *= factor;
								}
								axis_steps_per_unit[i] = value;
							}
							else {
								axis_steps_per_unit[i] = code_value();
							}
						}
					}
					break;
					case 115: // M115
					SERIAL_PROTOCOLPGM(MSG_M115_REPORT);
					break;
					case 117: // M117 display message
					starpos = (strchr(strchr_pointer + 5,'*'));
					if(starpos!=NULL)
						*(starpos-1)='\0';
					break;
					case 114: // M114
					SERIAL_PROTOCOLPGM("X:");
					SERIAL_PROTOCOL(current_position[X_AXIS]);
					SERIAL_PROTOCOLPGM("Y:");
					SERIAL_PROTOCOL(current_position[Y_AXIS]);
					SERIAL_PROTOCOLPGM("Z:");
					SERIAL_PROTOCOL(current_position[Z_AXIS]);
					SERIAL_PROTOCOLPGM("E:");
					SERIAL_PROTOCOL(current_position[E_AXIS]);

					SERIAL_PROTOCOLPGM(MSG_COUNT_X);
					SERIAL_PROTOCOL(float(st_get_position(X_AXIS))/axis_steps_per_unit[X_AXIS]);
					SERIAL_PROTOCOLPGM("Y:");
					SERIAL_PROTOCOL(float(st_get_position(Y_AXIS))/axis_steps_per_unit[Y_AXIS]);
					SERIAL_PROTOCOLPGM("Z:");
					SERIAL_PROTOCOL(float(st_get_position(Z_AXIS))/axis_steps_per_unit[Z_AXIS]);

					SERIAL_PROTOCOLLN("");
					break;
					case 120: // M120
					enable_endstops(false) ;
					break;
					case 121: // M121
					enable_endstops(true) ;
					break;
					case 119: // M119
					SERIAL_PROTOCOLLN(MSG_M119_REPORT);
#if defined(X_MIN_PIN) && X_MIN_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_X_MIN);
					SERIAL_PROTOCOLLN(((READ(X_MIN_PIN)^X_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
#if defined(X_MAX_PIN) && X_MAX_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_X_MAX);
					SERIAL_PROTOCOLLN(((READ(X_MAX_PIN)^X_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
#if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_Y_MIN);
					SERIAL_PROTOCOLLN(((READ(Y_MIN_PIN)^Y_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
#if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_Y_MAX);
					SERIAL_PROTOCOLLN(((READ(Y_MAX_PIN)^Y_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
#if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_Z_MIN);
					SERIAL_PROTOCOLLN(((READ(Z_MIN_PIN)^Z_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
#if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
					SERIAL_PROTOCOLPGM(MSG_Z_MAX);
					SERIAL_PROTOCOLLN(((READ(Z_MAX_PIN)^Z_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
#endif
					break;
					//TODO: update for all axis, use for loop
					case 201: // M201
					for(int8_t i=0; i < NUM_AXIS; i++)
					{
						if(code_seen(axis_codes[i]))
						{
							max_acceleration_units_per_sq_second[i] = code_value();
						}
					}
					// steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
					reset_acceleration_rates();
					break;
#if 0 // Not used for Sprinter/grbl gen6
					case 202: // M202
					for(int8_t i=0; i < NUM_AXIS; i++) {
						if(code_seen(axis_codes[i])) axis_travel_steps_per_sqr_second[i] = code_value() * axis_steps_per_unit[i];
					}
					break;
#endif
					case 203: // M203 max feedrate mm/sec
					for(int8_t i=0; i < NUM_AXIS; i++) {
						if(code_seen(axis_codes[i])) max_feedrate[i] = code_value();
					}
					break;
					case 204: // M204 acclereration S normal moves T filmanent only moves
					{
						if(code_seen('S')) acceleration = code_value() ;
						if(code_seen('T')) retract_acceleration = code_value() ;
					}
					break;
					case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
					{
						if(code_seen('S')) minimumfeedrate = code_value();
						if(code_seen('T')) mintravelfeedrate = code_value();
						if(code_seen('B')) minsegmenttime = code_value() ;
						if(code_seen('X')) max_xy_jerk = code_value() ;
						if(code_seen('Z')) max_z_jerk = code_value() ;
						if(code_seen('E')) max_e_jerk = code_value() ;
					}
					break;
					case 206: // M206 additional homeing offset
					for(int8_t i=0; i < 3; i++)
					{
						if(code_seen(axis_codes[i])) add_homeing[i] = code_value();
					}
					break;
#ifdef DELTA
					case 666: // M666 set delta endstop adjustemnt
					for(int8_t i=0; i < 3; i++)
					{
						if(code_seen(axis_codes[i])) endstop_adj[i] = code_value();
					}
					break;
#endif
#ifdef FWRETRACT
					case 207: //M207 - set retract length S[positive mm] F[feedrate mm/sec] Z[additional zlift/hop]
					{
						if(code_seen('S'))
						{
							retract_length = code_value() ;
						}
						if(code_seen('F'))
						{
							retract_feedrate = code_value() ;
						}
						if(code_seen('Z'))
						{
							retract_zlift = code_value() ;
						}
					}break;
					case 208: // M208 - set retract recover length S[positive mm surplus to the M207 S*] F[feedrate mm/sec]
					{
						if(code_seen('S'))
						{
							retract_recover_length = code_value() ;
						}
						if(code_seen('F'))
						{
							retract_recover_feedrate = code_value() ;
						}
					}break;
					case 209: // M209 - S<1=true/0=false> enable automatic retract detect if the slicer did not support G10/11: every normal extrude-only move will be classified as retract depending on the direction.
					{
						if(code_seen('S'))
						{
							int t= code_value() ;
							switch(t)
							{
								case 0: autoretract_enabled=false;retracted=false;break;
								case 1: autoretract_enabled=true;retracted=false;break;
								default:
									SERIAL_ECHO_START;
									SERIAL_ECHOPGM(MSG_UNKNOWN_COMMAND);
									SERIAL_ECHO(cmdbuffer[bufindr]);
									SERIAL_ECHOLNPGM("\"");
							}
						}

					}break;
#endif // FWRETRACT
#if EXTRUDERS > 1
					case 218: // M218 - set hotend offset (in mm), T<extruder_number> X<offset_on_X> Y<offset_on_Y>
					{
						if(setTargetedHotend(218)){
							break;
						}
						if(code_seen('X'))
						{
							extruder_offset[X_AXIS][tmp_extruder] = code_value();
						}
						if(code_seen('Y'))
						{
							extruder_offset[Y_AXIS][tmp_extruder] = code_value();
						}
#ifdef DUAL_X_CARRIAGE
						if(code_seen('Z'))
						{
							extruder_offset[Z_AXIS][tmp_extruder] = code_value();
						}
#endif
						SERIAL_ECHO_START;
						SERIAL_ECHOPGM(MSG_HOTEND_OFFSET);
						for(tmp_extruder = 0; tmp_extruder < EXTRUDERS; tmp_extruder++)
						{
							SERIAL_ECHO(" ");
							SERIAL_ECHO(extruder_offset[X_AXIS][tmp_extruder]);
							SERIAL_ECHO(",");
							SERIAL_ECHO(extruder_offset[Y_AXIS][tmp_extruder]);
#ifdef DUAL_X_CARRIAGE
							SERIAL_ECHO(",");
							SERIAL_ECHO(extruder_offset[Z_AXIS][tmp_extruder]);
#endif
						}
						SERIAL_ECHOLN("");
					}break;
#endif
					case 220: // M220 S<factor in percent>- set speed factor override percentage
					{
						if(code_seen('S'))
						{
							feedmultiply = code_value() ;
						}
					}
					break;
					case 221: // M221 S<factor in percent>- set extrude factor override percentage
					{
						if(code_seen('S'))
						{
							extrudemultiply = code_value() ;
						}
					}
					break;

#if LARGE_FLASH == true && ( BEEPER > 0 || defined(ULTRALCD) )
					case 300: // M300
					{
						int beepS = code_seen('S') ? code_value() : 110;
						int beepP = code_seen('P') ? code_value() : 1000;
						delay_ms(beepP);
					}
					break;
#endif // M300

#ifdef PIDTEMP
					case 301: // M301
					{
						if(code_seen('P')) Kp = code_value();
						if(code_seen('I')) Ki = scalePID_i(code_value());
						if(code_seen('D')) Kd = scalePID_d(code_value());

#ifdef PID_ADD_EXTRUSION_RATE
						if(code_seen('C')) Kc = code_value();
#endif

						updatePID();
						SERIAL_PROTOCOL(MSG_OK);
						SERIAL_PROTOCOL(" p:");
						SERIAL_PROTOCOL(Kp);
						SERIAL_PROTOCOL(" i:");
						SERIAL_PROTOCOL(unscalePID_i(Ki));
						SERIAL_PROTOCOL(" d:");
						SERIAL_PROTOCOL(unscalePID_d(Kd));
#ifdef PID_ADD_EXTRUSION_RATE
						SERIAL_PROTOCOL(" c:");
						//Kc does not have scaling applied above, or in resetting defaults
						SERIAL_PROTOCOL(Kc);
#endif
						SERIAL_PROTOCOLLN("");
					}
					break;
#endif //PIDTEMP
#ifdef PIDTEMPBED
					case 304: // M304
					{
						if(code_seen('P')) bedKp = code_value();
						if(code_seen('I')) bedKi = scalePID_i(code_value());
						if(code_seen('D')) bedKd = scalePID_d(code_value());

						updatePID();
						SERIAL_PROTOCOL(MSG_OK);
						SERIAL_PROTOCOL(" p:");
						SERIAL_PROTOCOL(bedKp);
						SERIAL_PROTOCOL(" i:");
						SERIAL_PROTOCOL(unscalePID_i(bedKi));
						SERIAL_PROTOCOL(" d:");
						SERIAL_PROTOCOL(unscalePID_d(bedKd));
						SERIAL_PROTOCOLLN("");
					}
					break;
#endif //PIDTEMP
					case 240: // M240  Triggers a camera by emulating a Canon RC-1 : http://www.doc-diy.net/photo/rc-1_hacked/
					{
#if defined(PHOTOGRAPH_PIN) && PHOTOGRAPH_PIN > -1
						const uint8_t NUM_PULSES=16;
						const float PULSE_LENGTH=0.01524;
						for(int i=0; i < NUM_PULSES; i++) {
							WRITE(PHOTOGRAPH_PIN, HIGH);
							delay_ms(PULSE_LENGTH);
							WRITE(PHOTOGRAPH_PIN, LOW);
							delay_ms(PULSE_LENGTH);
						}
						delay_ms(7);
						for(int i=0; i < NUM_PULSES; i++) {
							WRITE(PHOTOGRAPH_PIN, HIGH);
							delay_ms(PULSE_LENGTH);
							WRITE(PHOTOGRAPH_PIN, LOW);
							delay_ms(PULSE_LENGTH);
						}
#endif
					}
					break;
#ifdef PREVENT_DANGEROUS_EXTRUDE
					case 302: // allow cold extrudes, or set the minimum extrude temperature
					{
						float temp = .0;
						if (code_seen('S')) temp=code_value();
						set_extrude_min_temp(temp);
					}
					break;
#endif
					case 303: // M303 PID autotune
					{
						float temp = 150.0;
						int e=0;
						int c=5;
						if (code_seen('E')) e=code_value();
						if (e<0)
							temp=70;
						if (code_seen('S')) temp=code_value();
						if (code_seen('C')) c=code_value();
						PID_autotune(temp, e, c);
					}
					break;
					case 400: // M400 finish all moves
					{
						st_synchronize();
					}
					break;
					case 500: // M500 Store settings in EEPROM
					{
						Config_StoreSettings();
					}
					break;
					case 501: // M501 Read settings from EEPROM
					{
						Config_RetrieveSettings();
					}
					break;
					case 502: // M502 Revert to default settings
					{
						Config_ResetDefault();
					}
					break;
					case 503: // M503 print settings currently in memory
					{
						Config_PrintSettings();
					}
					break;
#ifdef ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED
					case 540:
					{
						if(code_seen('S')) abort_on_endstop_hit = code_value() > 0;
					}
					break;
#endif
#ifdef FILAMENTCHANGEENABLE
					case 600: //Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
					{
						float target[4];
						float lastpos[4];
						target[X_AXIS]=current_position[X_AXIS];
						target[Y_AXIS]=current_position[Y_AXIS];
						target[Z_AXIS]=current_position[Z_AXIS];
						target[E_AXIS]=current_position[E_AXIS];
						lastpos[X_AXIS]=current_position[X_AXIS];
						lastpos[Y_AXIS]=current_position[Y_AXIS];
						lastpos[Z_AXIS]=current_position[Z_AXIS];
						lastpos[E_AXIS]=current_position[E_AXIS];
						//retract by E
						if(code_seen('E'))
						{
							target[E_AXIS]+= code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_FIRSTRETRACT
							target[E_AXIS]+= FILAMENTCHANGE_FIRSTRETRACT ;
#endif
						}
						plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

						//lift Z
						if(code_seen('Z'))
						{
							target[Z_AXIS]+= code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_ZADD
							target[Z_AXIS]+= FILAMENTCHANGE_ZADD ;
#endif
						}
						plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

						//move xy
						if(code_seen('X'))
						{
							target[X_AXIS]+= code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_XPOS
							target[X_AXIS]= FILAMENTCHANGE_XPOS ;
#endif
						}
						if(code_seen('Y'))
						{
							target[Y_AXIS]= code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_YPOS
							target[Y_AXIS]= FILAMENTCHANGE_YPOS ;
#endif
						}

						plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

						if(code_seen('L'))
						{
							target[E_AXIS]+= code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_FINALRETRACT
							target[E_AXIS]+= FILAMENTCHANGE_FINALRETRACT ;
#endif
						}

						plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

						//finish moves
						st_synchronize();
						//disable extruder steppers so filament can be removed
						disable_e0();
						disable_e1();
						disable_e2();
						delay_ms(100);
						LCD_ALERTMESSAGEPGM(MSG_FILAMENTCHANGE);
						uint8_t cnt=0;

						//return to normal
						if(code_seen('L'))
						{
							target[E_AXIS]+= -code_value();
						}
						else
						{
#ifdef FILAMENTCHANGE_FINALRETRACT
							target[E_AXIS]+=(-1)*FILAMENTCHANGE_FINALRETRACT ;
#endif
						}
						current_position[E_AXIS]=target[E_AXIS]; //the long retract of L is compensated by manual filament feeding
						plan_set_e_position(current_position[E_AXIS]);
						plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //should do nothing
						plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move xy back
						plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move z back
						plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], lastpos[E_AXIS], feedrate/60, active_extruder); //final untretract
					}
					break;
#endif //FILAMENTCHANGEENABLE
#ifdef DUAL_X_CARRIAGE
					case 605: // Set dual x-carriage movement mode:
					//    M605 S0: Full control mode. The slicer has full control over x-carriage movement
					//    M605 S1: Auto-park mode. The inactive head will auto park/unpark without slicer involvement
					//    M605 S2 [Xnnn] [Rmmm]: Duplication mode. The second extruder will duplicate the first with nnn
					//                         millimeters x-offset and an optional differential hotend temperature of
					//                         mmm degrees. E.g., with "M605 S2 X100 R2" the second extruder will duplicate
					//                         the first with a spacing of 100mm in the x direction and 2 degrees hotter.
					//
					//    Note: the X axis should be homed after changing dual x-carriage mode.
					{
						st_synchronize();

						if (code_seen('S'))
							dual_x_carriage_mode = code_value();

						if (dual_x_carriage_mode == DXC_DUPLICATION_MODE)
						{
							if (code_seen('X'))
								duplicate_extruder_x_offset = max(code_value(),X2_MIN_POS - x_home_pos(0));

							if (code_seen('R'))
								duplicate_extruder_temp_offset = code_value();

							SERIAL_ECHO_START;
							SERIAL_ECHOPGM(MSG_HOTEND_OFFSET);
							SERIAL_ECHO(" ");
							SERIAL_ECHO(extruder_offset[X_AXIS][0]);
							SERIAL_ECHO(",");
							SERIAL_ECHO(extruder_offset[Y_AXIS][0]);
							SERIAL_ECHO(" ");
							SERIAL_ECHO(duplicate_extruder_x_offset);
							SERIAL_ECHO(",");
							SERIAL_ECHOLN(extruder_offset[Y_AXIS][1]);
						}
						else if (dual_x_carriage_mode != DXC_FULL_CONTROL_MODE && dual_x_carriage_mode != DXC_AUTO_PARK_MODE)
						{
							dual_x_carriage_mode = DEFAULT_DUAL_X_CARRIAGE_MODE;
						}

						active_extruder_parked = false;
						extruder_duplication_enabled = false;
						delayed_move_time = 0;
					}
					break;
#endif //DUAL_X_CARRIAGE

					case 907: // M907 Set digital trimpot motor current using axis codes.
					{
#if defined(DIGIPOTSS_PIN) && DIGIPOTSS_PIN > -1
						for(int i=0;i<NUM_AXIS;i++) if(code_seen(axis_codes[i])) digipot_current(i,code_value());
						if(code_seen('B')) digipot_current(4,code_value());
						if(code_seen('S')) for(int i=0;i<=4;i++) digipot_current(i,code_value());
#endif
					}
					break;
					case 908: // M908 Control digital trimpot directly.
					{
#if defined(DIGIPOTSS_PIN) && DIGIPOTSS_PIN > -1
						uint8_t channel,current;
						if(code_seen('P')) channel=code_value();
						if(code_seen('S')) current=code_value();
						digitalPotWrite(channel, current);
#endif
					}
					break;
					case 350: // M350 Set microstepping mode. Warning: Steps per unit remains unchanged. S code sets stepping mode for all drivers.
					{
#if defined(X_MS1_PIN) && X_MS1_PIN > -1
						if(code_seen('S')) for(int i=0;i<=4;i++) microstep_mode(i,code_value());
						for(int i=0;i<NUM_AXIS;i++) if(code_seen(axis_codes[i])) microstep_mode(i,(uint8_t)code_value());
						if(code_seen('B')) microstep_mode(4,code_value());
						microstep_readings();
#endif
					}
					break;
					case 351: // M351 Toggle MS1 MS2 pins directly, S# determines MS1 or MS2, X# sets the pin high/low.
					{
#if defined(X_MS1_PIN) && X_MS1_PIN > -1
						if(code_seen('S')) switch((int)code_value())
						{
							case 1:
								for(int i=0;i<NUM_AXIS;i++) if(code_seen(axis_codes[i])) microstep_ms(i,code_value(),-1);
								if(code_seen('B')) microstep_ms(4,code_value(),-1);
								break;
							case 2:
								for(int i=0;i<NUM_AXIS;i++) if(code_seen(axis_codes[i])) microstep_ms(i,-1,code_value());
								if(code_seen('B')) microstep_ms(4,-1,code_value());
								break;
						}
						microstep_readings();
#endif
					}
					break;
					case 999: // M999: Restart after being stopped
					Stopped = false;
					gcode_LastN = Stopped_gcode_LastN;
					FlushSerialRequestResend();
					break;
				}
		}

		else if(code_seen('T'))
		{
			tmp_extruder = code_value();
			if(tmp_extruder >= EXTRUDERS) {
				SERIAL_ECHO_START;
				SERIAL_ECHO("T");
				SERIAL_ECHO(tmp_extruder);
				SERIAL_ECHOLN(MSG_INVALID_EXTRUDER);
			}
			else {
				bool make_move = false;
				if(code_seen('F')) {
					make_move = true;
					next_feedrate = code_value();
					if(next_feedrate > 0.0) {
						feedrate = next_feedrate;
					}
				}
#if EXTRUDERS > 1
				if(tmp_extruder != active_extruder) {
					// Save current position to return to after applying extruder offset
					memcpy(destination, current_position, sizeof(destination));
#ifdef DUAL_X_CARRIAGE
					if (dual_x_carriage_mode == DXC_AUTO_PARK_MODE && Stopped == false &&
							(delayed_move_time != 0 || current_position[X_AXIS] != x_home_pos(active_extruder)))
					{
						// Park old head: 1) raise 2) move to park position 3) lower
						plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS] + TOOLCHANGE_PARK_ZLIFT,
								current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder);
						plan_buffer_line(x_home_pos(active_extruder), current_position[Y_AXIS], current_position[Z_AXIS] + TOOLCHANGE_PARK_ZLIFT,
								current_position[E_AXIS], max_feedrate[X_AXIS], active_extruder);
						plan_buffer_line(x_home_pos(active_extruder), current_position[Y_AXIS], current_position[Z_AXIS],
								current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder);
						st_synchronize();
					}

					// apply Y & Z extruder offset (x offset is already used in determining home pos)
					current_position[Y_AXIS] = current_position[Y_AXIS] -
						extruder_offset[Y_AXIS][active_extruder] +
						extruder_offset[Y_AXIS][tmp_extruder];
					current_position[Z_AXIS] = current_position[Z_AXIS] -
						extruder_offset[Z_AXIS][active_extruder] +
						extruder_offset[Z_AXIS][tmp_extruder];

					active_extruder = tmp_extruder;

					// This function resets the max/min values - the current position may be overwritten below.
					axis_is_at_home(X_AXIS);

					if (dual_x_carriage_mode == DXC_FULL_CONTROL_MODE)
					{
						current_position[X_AXIS] = inactive_extruder_x_pos;
						inactive_extruder_x_pos = destination[X_AXIS];
					}
					else if (dual_x_carriage_mode == DXC_DUPLICATION_MODE)
					{
						active_extruder_parked = (active_extruder == 0); // this triggers the second extruder to move into the duplication position
						if (active_extruder == 0 || active_extruder_parked)
							current_position[X_AXIS] = inactive_extruder_x_pos;
						else
							current_position[X_AXIS] = destination[X_AXIS] + duplicate_extruder_x_offset;
						inactive_extruder_x_pos = destination[X_AXIS];
						extruder_duplication_enabled = false;
					}
					else
					{
						// record raised toolhead position for use by unpark
						memcpy(raised_parked_position, current_position, sizeof(raised_parked_position));
						raised_parked_position[Z_AXIS] += TOOLCHANGE_UNPARK_ZLIFT;
						active_extruder_parked = true;
						delayed_move_time = 0;
					}
#else
					// Offset extruder (only by XY)
					int i;
					for(i = 0; i < 2; i++) {
						current_position[i] = current_position[i] -
							extruder_offset[i][active_extruder] +
							extruder_offset[i][tmp_extruder];
					}
					// Set the new active extruder and position
					active_extruder = tmp_extruder;
#endif //else DUAL_X_CARRIAGE
					plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
					// Move to the old position if 'F' was in the parameters
					if(make_move && Stopped == false) {
						prepare_move();
					}
				}
#endif
				SERIAL_ECHO_START;
				SERIAL_ECHO(MSG_ACTIVE_EXTRUDER);
				SERIAL_PROTOCOLLN((int)active_extruder);
			}
		}

		else
		{
			SERIAL_ECHO_START;
			SERIAL_ECHOPGM(MSG_UNKNOWN_COMMAND);
			SERIAL_ECHO(cmdbuffer[bufindr]);
			SERIAL_ECHOLNPGM("\"");
		}

		ClearToSend();
	}

	void FlushSerialRequestResend()
	{
		//char cmdbuffer[bufindr][100]="Resend:";
		wait_ms(200); //dont know
		MYSERIAL.flush();
		SERIAL_PROTOCOLPGM(MSG_RESEND);
		SERIAL_PROTOCOLLN(gcode_LastN + 1);
		ClearToSend();
	}

	void ClearToSend()
	{
		previous_millis_cmd = millis();
#ifdef SDSUPPORT
		if(fromsd[bufindr])
			return;
#endif //SDSUPPORT
		SERIAL_PROTOCOLLNPGM(MSG_OK);
		wait_ms(10); //ACHTUNG
	}

	void get_coordinates()
	{
		bool seen[4]={false,false,false,false};
		for(int8_t i=0; i < NUM_AXIS; i++) {
			if(code_seen(axis_codes[i]))
			{
				destination[i] = (float)code_value() + (axis_relative_modes[i] || relative_mode)*current_position[i];
				seen[i]=true;
			}
			else destination[i] = current_position[i]; //Are these else lines really needed?
		}
		if(code_seen('F')) {
			next_feedrate = code_value();
			if(next_feedrate > 0.0) feedrate = next_feedrate;
		}
#ifdef FWRETRACT
		if(autoretract_enabled)
			if( !(seen[X_AXIS] || seen[Y_AXIS] || seen[Z_AXIS]) && seen[E_AXIS])
			{
				float echange=destination[E_AXIS]-current_position[E_AXIS];
				if(echange<-MIN_RETRACT) //retract
				{
					if(!retracted)
					{

						destination[Z_AXIS]+=retract_zlift; //not sure why chaninging current_position negatively does not work.
						//if slicer retracted by echange=-1mm and you want to retract 3mm, corrrectede=-2mm additionally
						float correctede=-echange-retract_length;
						//to generate the additional steps, not the destination is changed, but inversely the current position
						current_position[E_AXIS]+=-correctede;
						feedrate=retract_feedrate;
						retracted=true;
					}

				}
				else
					if(echange>MIN_RETRACT) //retract_recover
					{
						if(retracted)
						{
							//current_position[Z_AXIS]+=-retract_zlift;
							//if slicer retracted_recovered by echange=+1mm and you want to retract_recover 3mm, corrrectede=2mm additionally
							float correctede=-echange+1*retract_length+retract_recover_length; //total unretract=retract_length+retract_recover_length[surplus]
							current_position[E_AXIS]+=correctede; //to generate the additional steps, not the destination is changed, but inversely the current position
							feedrate=retract_recover_feedrate;
							retracted=false;
						}
					}

			}
#endif //FWRETRACT
	}

	void get_arc_coordinates()
	{
#ifdef SF_ARC_FIX
		bool relative_mode_backup = relative_mode;
		relative_mode = true;
#endif
		get_coordinates();
#ifdef SF_ARC_FIX
		relative_mode=relative_mode_backup;
#endif

		if(code_seen('I')) {
			offset[0] = code_value();
		}
		else {
			offset[0] = 0.0;
		}
		if(code_seen('J')) {
			offset[1] = code_value();
		}
		else {
			offset[1] = 0.0;
		}
	}

	void clamp_to_software_endstops(float target[3])
	{
		if (min_software_endstops) {
			if (target[X_AXIS] < min_pos[X_AXIS]) target[X_AXIS] = min_pos[X_AXIS];
			if (target[Y_AXIS] < min_pos[Y_AXIS]) target[Y_AXIS] = min_pos[Y_AXIS];
			if (target[Z_AXIS] < min_pos[Z_AXIS]) target[Z_AXIS] = min_pos[Z_AXIS];
		}

		if (max_software_endstops) {
			if (target[X_AXIS] > max_pos[X_AXIS]) target[X_AXIS] = max_pos[X_AXIS];
			if (target[Y_AXIS] > max_pos[Y_AXIS]) target[Y_AXIS] = max_pos[Y_AXIS];
			if (target[Z_AXIS] > max_pos[Z_AXIS]) target[Z_AXIS] = max_pos[Z_AXIS];
		}
	}


	void prepare_move()
	{
		clamp_to_software_endstops(destination);

		previous_millis_cmd = millis();

#ifdef DUAL_X_CARRIAGE
		if (active_extruder_parked)
		{
			if (dual_x_carriage_mode == DXC_DUPLICATION_MODE && active_extruder == 0)
			{
				// move duplicate extruder into correct duplication position.
				plan_set_position(inactive_extruder_x_pos, current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
				plan_buffer_line(current_position[X_AXIS] + duplicate_extruder_x_offset, current_position[Y_AXIS], current_position[Z_AXIS],
						current_position[E_AXIS], max_feedrate[X_AXIS], 1);
				plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
				st_synchronize();
				extruder_duplication_enabled = true;
				active_extruder_parked = false;
			}
			else if (dual_x_carriage_mode == DXC_AUTO_PARK_MODE) // handle unparking of head
			{
				if (current_position[E_AXIS] == destination[E_AXIS])
				{
					// this is a travel move - skit it but keep track of current position (so that it can later
					// be used as start of first non-travel move)
					if (delayed_move_time != 0xFFFFFFFFUL)
					{
						memcpy(current_position, destination, sizeof(current_position));
						if (destination[Z_AXIS] > raised_parked_position[Z_AXIS])
							raised_parked_position[Z_AXIS] = destination[Z_AXIS];
						delayed_move_time = millis();
						return;
					}
				}
				delayed_move_time = 0;
				// unpark extruder: 1) raise, 2) move into starting XY position, 3) lower
				plan_buffer_line(raised_parked_position[X_AXIS], raised_parked_position[Y_AXIS], raised_parked_position[Z_AXIS],    current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder);
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], raised_parked_position[Z_AXIS],
						current_position[E_AXIS], min(max_feedrate[X_AXIS],max_feedrate[Y_AXIS]), active_extruder);
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS],
						current_position[E_AXIS], max_feedrate[Z_AXIS], active_extruder);
				active_extruder_parked = false;
			}
		}
#endif //DUAL_X_CARRIAGE

		// Do not use feedmultiply for E or Z only moves
		if( (current_position[X_AXIS] == destination [X_AXIS]) && (current_position[Y_AXIS] == destination [Y_AXIS])) {
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
		}
		else {
			plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate*feedmultiply/60/100.0, active_extruder);
		}
		for(int8_t i=0; i < NUM_AXIS; i++) {
			current_position[i] = destination[i];
		}
	}

	void prepare_arc_move(char isclockwise) {
		float r = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc

		// Trace the arc
		mc_arc(current_position, destination, offset, X_AXIS, Y_AXIS, Z_AXIS, feedrate*feedmultiply/60/100.0, r, isclockwise, active_extruder);

		// As far as the parser is concerned, the position is now == target. In reality the
		// motion control system might still be processing the action and the real tool position
		// in any intermediate location.
		for(int8_t i=0; i < NUM_AXIS; i++) {
			current_position[i] = destination[i];
		}
		previous_millis_cmd = millis();
	}

#if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1

#if defined(FAN_PIN)
#if CONTROLLERFAN_PIN == FAN_PIN
#error "You cannot set CONTROLLERFAN_PIN equal to FAN_PIN"
#endif
#endif

	unsigned long lastMotor = 0; //Save the time for when a motor was turned on last
	unsigned long lastMotorCheck = 0;

	void controllerFan()
	{
		if ((millis() - lastMotorCheck) >= 2500) //Not a time critical function, so we only check every 2500ms
		{
			lastMotorCheck = millis();

			if(!READ(X_ENABLE_PIN) || !READ(Y_ENABLE_PIN) || !READ(Z_ENABLE_PIN)
#if EXTRUDERS > 2
					|| !READ(E2_ENABLE_PIN)
#endif
#if EXTRUDER > 1
#if defined(X2_ENABLE_PIN) && X2_ENABLE_PIN > -1
					|| !READ(X2_ENABLE_PIN)
#endif
					|| !READ(E1_ENABLE_PIN)
#endif
					|| !READ(E0_ENABLE_PIN)) //If any of the drivers are enabled...
			{
				lastMotor = millis(); //... set time to NOW so the fan will turn on
			}

			if ((millis() - lastMotor) >= (CONTROLLERFAN_SECS*1000UL) || lastMotor == 0) //If the last time any driver was enabled, is longer since than CONTROLLERSEC...
			{
				digitalWrite(CONTROLLERFAN_PIN, 0);
				analogWrite(CONTROLLERFAN_PIN, 0);
			}
			else
			{
				// allows digital or PWM fan output to be used (see M42 handling)
				digitalWrite(CONTROLLERFAN_PIN, CONTROLLERFAN_SPEED);
				analogWrite(CONTROLLERFAN_PIN, CONTROLLERFAN_SPEED);
			}
		}
	}
#endif

	void manage_inactivity()
	{
		if( (millis() - previous_millis_cmd) >  max_inactive_time )
			if(max_inactive_time)
				kill();
		if(stepper_inactive_time)  {
			if( (millis() - previous_millis_cmd) >  stepper_inactive_time )
			{
				if(blocks_queued() == false) {
					disable_x();
					disable_y();
					disable_z();
					disable_e0();
					disable_e1();
					disable_e2();
				}
			}
		}
#if defined(KILL_PIN) && KILL_PIN > -1
		if( 0 == READ(KILL_PIN) )
			kill();
#endif
#if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1
		controllerFan(); //Check if fan should be turned on to cool stepper drivers down
#endif
#ifdef EXTRUDER_RUNOUT_PREVENT
		if( (millis() - previous_millis_cmd) >  EXTRUDER_RUNOUT_SECONDS*1000 )
			if(degHotend(active_extruder)>EXTRUDER_RUNOUT_MINTEMP)
			{
				bool oldstatus=READ(E0_ENABLE_PIN);
				enable_e0();
				float oldepos=current_position[E_AXIS];
				float oldedes=destination[E_AXIS];
				plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS],
						current_position[E_AXIS]+EXTRUDER_RUNOUT_EXTRUDE*EXTRUDER_RUNOUT_ESTEPS/axis_steps_per_unit[E_AXIS],
						EXTRUDER_RUNOUT_SPEED/60.*EXTRUDER_RUNOUT_ESTEPS/axis_steps_per_unit[E_AXIS], active_extruder);
				current_position[E_AXIS]=oldepos;
				destination[E_AXIS]=oldedes;
				plan_set_e_position(oldepos);
				previous_millis_cmd=millis();
				st_synchronize();
				WRITE(E0_ENABLE_PIN,oldstatus);
			}
#endif
#if defined(DUAL_X_CARRIAGE)
		// handle delayed move timeout
		if (delayed_move_time != 0 && (millis() - delayed_move_time) > 1000 && Stopped == false)
		{
			// travel moves have been received so enact them
			delayed_move_time = 0xFFFFFFFFUL; // force moves to be done
			memcpy(destination,current_position,sizeof(destination));
			prepare_move();
		}
#endif
		check_axes_activity();
	}

	void kill()
	{
		cli(); // Stop interrupts
		disable_heater();

		disable_x();
		disable_y();
		disable_z();
		disable_e0();
		disable_e1();
		disable_e2();

#if defined(PS_ON_PIN) && PS_ON_PIN > -1
		pinMode(PS_ON_PIN,INPUT);
#endif
		SERIAL_ERROR_START;
		SERIAL_ERRORLNPGM(MSG_ERR_KILLED);
		LCD_ALERTMESSAGEPGM(MSG_KILLED);
		suicide();
		while(1) { /* Intentionally left empty */ } // Wait for reset
	}

	void Stop()
	{
		disable_heater();
		if(Stopped == false) {
			Stopped = true;
			Stopped_gcode_LastN = gcode_LastN; // Save last g_code for restart
			SERIAL_ERROR_START;
			SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);
			LCD_MESSAGEPGM(MSG_STOPPED);
		}
	}

	bool IsStopped() { return Stopped; };

#ifdef FAST_PWM_FAN
	void setPwmFrequency(uint8_t pin, int val)
	{
		switch(digitalPinToTimer(pin))
		{

#if defined(TCCR0A)
			case TIMER0A:
			case TIMER0B:
				//         TCCR0B &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02));
				//         TCCR0B |= val;
				break;
#endif

#if defined(TCCR1A)
			case TIMER1A:
			case TIMER1B:
				//         TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
				//         TCCR1B |= val;
				break;
#endif

#if defined(TCCR2)
			case TIMER2:
			case TIMER2:
				TCCR2 &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
				TCCR2 |= val;
				break;
#endif

#if defined(TCCR2A)
			case TIMER2A:
			case TIMER2B:
				TCCR2B &= ~(_BV(CS20) | _BV(CS21) | _BV(CS22));
				TCCR2B |= val;
				break;
#endif

#if defined(TCCR3A)
			case TIMER3A:
			case TIMER3B:
			case TIMER3C:
				TCCR3B &= ~(_BV(CS30) | _BV(CS31) | _BV(CS32));
				TCCR3B |= val;
				break;
#endif

#if defined(TCCR4A)
			case TIMER4A:
			case TIMER4B:
			case TIMER4C:
				TCCR4B &= ~(_BV(CS40) | _BV(CS41) | _BV(CS42));
				TCCR4B |= val;
				break;
#endif

#if defined(TCCR5A)
			case TIMER5A:
			case TIMER5B:
			case TIMER5C:
				TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52));
				TCCR5B |= val;
				break;
#endif

		}
	}
#endif //FAST_PWM_FAN

	bool setTargetedHotend(int code){
		tmp_extruder = active_extruder;
		if(code_seen('T')) {
			tmp_extruder = code_value();
			if(tmp_extruder >= EXTRUDERS) {
				SERIAL_ECHO_START;
				switch(code){
					case 104:
						SERIAL_ECHO(MSG_M104_INVALID_EXTRUDER);
						break;
					case 105:
						SERIAL_ECHO(MSG_M105_INVALID_EXTRUDER);
						break;
					case 109:
						SERIAL_ECHO(MSG_M109_INVALID_EXTRUDER);
						break;
					case 218:
						SERIAL_ECHO(MSG_M218_INVALID_EXTRUDER);
						break;
				}
				SERIAL_ECHOLN(tmp_extruder);
				return true;
			}
		}
		return false;
	}

