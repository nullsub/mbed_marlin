#include "mbed.h"
#include "marlin/pins.h"
#include "math.h"
#include "marlin/Marlin.h"

DigitalOut p_heater0_led(LED1);//x
DigitalOut p_heat_bed_led(LED2);//y
//DigitalOut led3(LED3);//z
DigitalOut p_led(LED_PIN);//e

DigitalOut p_fan(FAN_PIN);

//DigitalOut p_x_enable(X_ENABLE_PIN);
DigitalOut p_x_dir(X_DIR_PIN);
DigitalOut p_x_step(X_STEP_PIN);
//DigitalIn p_x_min(X_MIN_PIN);
//DigitalIn p_x_max(X_MAX_PIN);

//DigitalOut p_y_enable(Y_ENABLE_PIN);
DigitalOut p_y_dir(Y_DIR_PIN);
DigitalOut p_y_step(Y_STEP_PIN);
//DigitalIn p_y_min(Y_MIN_PIN);
//DigitalIn p_y_max(Y_MAX_PIN);

//DigitalOut p_z_enable(Z_ENABLE_PIN);
DigitalOut p_z_dir(Z_DIR_PIN);
DigitalOut p_z_step(Z_STEP_PIN);
//DigitalIn p_z_min(Z_MIN_PIN);
//DigitalIn p_z_max(Z_MAX_PIN);

//DigitalOut p_e_enable(E_ENABLE_PIN);
DigitalOut p_e_dir(E_DIR_PIN);
DigitalOut p_e_step(E_STEP_PIN);

DigitalOut p_heater0(HEATER_0_PIN);
DigitalOut p_heater_bed(HEATER_BED_PIN);//heated-build-platform

AnalogIn p_temp0(TEMP_0_PIN);
AnalogIn p_temp_bed(TEMP_BED_PIN);//heated-build-platform thermistor

SerialBuffered serial_buffered( 4096, USBTX, USBRX);

Timer timer;

unsigned int millis()
{
	static long long current_ms = 0;
	if(timer.read_ms() > 10000*60) { /*reset every 10 minutes*/
		current_ms += timer.read_ms();
		timer.reset();
	}
	return current_ms + timer.read_ms();
}

void delay_ms(int ms)
{
	wait_ms(ms);
}

void cli(){}
void sei(){}

int main() {
	mbed_interface_disconnect(); //disable debug interface. and acces to MAC and local FS. ADC will work better!
	timer.start();
	serial_buffered.baud(BAUDRATE);
	setup();
	while (1) {
		loop();
	}
}
