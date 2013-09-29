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
/*
char print_buffer[100];
void print_int(int var) {
    sprintf(print_buffer,"%d",var);
    print_string(print_buffer);
}

void print_long(long var) {
    sprintf(print_buffer,"%ld", var);
    print_string(print_buffer);
}

void print_float(float var) {
    sprintf(print_buffer,"%f",var);
    print_string(print_buffer);
}

*/
int micros() {
    static long long current_us = 0;
    current_us += timer.read_us();
    timer.reset();
    return current_us;
}

unsigned int millis() {
    return (unsigned int)(micros()/1000);
}

void delay_ms(int ms)
{
	wait_ms(ms);
}

void cli(){}
void sei(){}

int main() {
    timer.start();
	serial_buffered.baud(BAUDRATE);
    setup();
    while (1) {
        loop();
    }
}

/*
void log_message(char*   message) {
    print_string("DEBUG");
    print_string(message);
}

void log_bool(char* message, int value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %i", value);
}

void log_int(char* message, int value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %i", value);
}

void log_long(char* message, long value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %l", value);
}

void log_float(char* message, float value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %f", value);
}

void log_uint(char* message, unsigned int value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %i", value);
}

void log_ulong(char* message, unsigned long value) {
    print_string("DEBUG");
    print_string(message);
    print_string(": %l", value);
}

void log_int_array(char* message, int value[], int array_lenght) {
    print_string("DEBUG");
    print_string(message);
    print_string(": {");
    for (int i=0; i < array_lenght; i++) {
        print_string("%i",value[i]);
        if (i != array_lenght-1) print_string(", ");
    }
    print_string("}\r\n");
}

void log_long_array(char* message, long value[], int array_lenght) {
    print_string("DEBUG");
    print_string(message);
    print_string(": {");
    for (int i=0; i < array_lenght; i++) {
        print_string("%l",value[i]);
        if (i != array_lenght-1) print_string(", ");
    }
    print_string("}\r\n");
}

void log_float_array(char* message, float value[], int array_lenght) {
    print_string("DEBUG");
    print_string(message);
    print_string(": {");
    for (int i=0; i < array_lenght; i++) {
        print_string("%f",value[i]);
        if (i != array_lenght-1) print_string(", ");
    }
    print_string("}\r\n");
}

void log_uint_array(char* message, unsigned int value[], int array_lenght) {
    print_string("DEBUG");
    print_string(message);
    print_string(": {");
    for (int i=0; i < array_lenght; i++) {
        print_string("%i", value[i]);
        if (i != array_lenght-1) print_string(", ");
    }
    print_string("}\r\n");
}

void log_ulong_array(char* message, unsigned long value[], int array_lenght) {
    print_string("DEBUG");
    print_string(message);
    print_string(": {");
    for (int i=0; i < array_lenght; i++) {
        print_string("%l",value[i]);
        if (i != array_lenght-1) print_string(", ");
    }
    print_string("}\r\n");
}
*/
