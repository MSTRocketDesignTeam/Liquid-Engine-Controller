#ifndef CONTROL_
#define CONTROL_

#include <Arduino.h>
#include <Servo.h>

// Defining control angles of the servos
const int SERVO_OPEN_ANGLE_US = 1140;
const int SERVO_CLOSE_ANGLE_US = 2500;
const int SERVO_PARTIAL_ANGLE_US = 2100;

void update_onboard_states(char LECUServoPwrSwitchState, char N2OMainPurgeState, char mainValvesState, char throttlingAlgorithmState,
                            int LECUServoPwrSwitchPin, Servo &oxValve, Servo &fuelValve, int indicatorLEDPin);

#endif
