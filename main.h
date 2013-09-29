#ifndef MAIN_H_
#define MAIN_H_

#include "mbed.h"
#include "SerialBuffered.h"

extern SerialBuffered serial_buffered;

extern DigitalOut p_heater0_led;
extern DigitalOut p_heat_bed_led;
//DigitalOut led3(LED3);//z
extern DigitalOut p_led;
extern DigitalOut p_fan;

//DigitalOut p_x_enable(X_ENABLE_PIN);
extern DigitalOut p_x_dir;
extern DigitalOut p_x_step;
//DigitalIn p_x_min(X_MIN_PIN);
//DigitalIn p_x_max(X_MAX_PIN);

//DigitalOut p_y_enable(Y_ENABLE_PIN);
extern DigitalOut p_y_dir;
extern DigitalOut p_y_step;
//DigitalIn p_y_min(Y_MIN_PIN);
//DigitalIn p_y_max(Y_MAX_PIN);

//DigitalOut p_z_enable(Z_ENABLE_PIN);
extern DigitalOut p_z_dir;
extern DigitalOut p_z_step;
//DigitalIn p_z_min(Z_MIN_PIN);
//DigitalIn p_z_max(Z_MAX_PIN);

//DigitalOut p_e_enable(E_ENABLE_PIN);
extern DigitalOut p_e_dir;
extern DigitalOut p_e_step;

extern DigitalOut p_heater0;
extern DigitalOut p_heater_bed;;

extern AnalogIn p_temp0;
extern AnalogIn p_temp_bed;

int micros();

unsigned int millis();

void cli();
void sei();
void delay_ms(int ms);

#endif
