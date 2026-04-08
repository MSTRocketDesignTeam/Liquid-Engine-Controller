#include <Arduino.h>
#include <Servo.h>
#include <control.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <DAQ.h>

// Pin Assignments
int indicatorLEDPin = 22;
int fuelMainValvePWMPin   = 26;
int oxMainValvePWMPin     = 27;
int LECUServoPwrSwitchPin = 28;
Servo fuelValve;
Servo oxValve;

// -------------------- Control States --------------------
// Loop-owned states
char LECUServoPwrSwitchState;
char N2OMainValvePurgeState;
char mainValvesState;
char throttlingAlgorithmState;

// -------------------- Serial RX --------------------
const byte NUM_CHARS = 4;
char receivedChars[NUM_CHARS];
bool newData = false;

// -------------------- Timer --------------------
RPI_PICO_Timer ITimer0(0);
const int SENSOR_DATA_TX_INTERVAL = 1000;   // in ms
volatile bool sendDataFlag = false;

// -------------------- Function Declarations --------------------
void recv_with_start_end_markers();
void process_ctrl_packet();
bool send_sensor_data_ISR(struct repeating_timer *t);


void setup() {
  Serial1.begin(115200);
  pinMode(LECUServoPwrSwitchPin, OUTPUT);
  pinMode(indicatorLEDPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  fuelValve.attach(fuelMainValvePWMPin, 500, 2500);
  oxValve.attach(oxMainValvePWMPin, 500, 2500);
}

void loop() {
  recv_with_start_end_markers();
  if (newData) {
    newData = false;
  }
  process_ctrl_packet();
  

}


void recv_with_start_end_markers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData == false) {
    rc = Serial1.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= NUM_CHARS) {
          ndx = NUM_CHARS - 1;
        }
      } else {
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void process_ctrl_packet() {

  // ---- Critical section: copy shared RX buffer ----
  noInterrupts();
  LECUServoPwrSwitchState = receivedChars[0];
  N2OMainValvePurgeState        = receivedChars[1];
  mainValvesState        = receivedChars[2];
  throttlingAlgorithmState  = receivedChars[3];
  interrupts();

  // ---- Update hardware (interrupts ENABLED) ----
  update_onboard_states(
    LECUServoPwrSwitchState,
    N2OMainValvePurgeState,
    mainValvesState,
    throttlingAlgorithmState,
    LECUServoPwrSwitchPin,
    oxValve,
    fuelValve,
    indicatorLEDPin
  );
}

bool send_sensor_data_ISR(struct repeating_timer *t) {
  sendDataFlag = true;  // Set flag only
  return true;
}