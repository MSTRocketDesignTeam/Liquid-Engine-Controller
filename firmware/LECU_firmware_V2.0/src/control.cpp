#include <Arduino.h>
#include <Servo.h>
#include <control.h>

// Purpose: update onboard control hardware states (valve positions, indicator LED states, etc.) based on ctrlString index values. 
// Pre: 
// Post: 
void update_onboard_states(char LECUServoPwrSwitchState, char N2OMainPurgeState, char mainValvesState, char throttlingAlgorithmState,
                            int LECUServoPwrSwitchPin, Servo &oxValve, Servo &fuelValve, int indicatorLEDPin) {
    if (LECUServoPwrSwitchState == 'L') {
        digitalWrite(LECUServoPwrSwitchPin, LOW);
    } else if (LECUServoPwrSwitchState == 'H') {
        digitalWrite(LECUServoPwrSwitchPin, HIGH);
    }

    if (mainValvesState == 'O') {
        // Main valves override everything
        oxValve.writeMicroseconds(SERVO_OPEN_ANGLE_US);
        fuelValve.writeMicroseconds(SERVO_OPEN_ANGLE_US);
    } else if (mainValvesState == 'C') {
        // Fuel always follows main valves
        fuelValve.writeMicroseconds(SERVO_CLOSE_ANGLE_US);

        // Ox valve can still be controlled by purge
        if (N2OMainPurgeState == 'O') {
            oxValve.writeMicroseconds(SERVO_OPEN_ANGLE_US);
        } else if (N2OMainPurgeState == 'C') {
            oxValve.writeMicroseconds(SERVO_CLOSE_ANGLE_US);
        }
    }
    
    
    

}