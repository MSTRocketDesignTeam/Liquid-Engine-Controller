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

static unsigned long lastLEDMillis = 0;
static unsigned long currentLEDMillis = 0;
static int LEDState = LOW;

// ------------------- Sensor Data -----------------------
volatile uint32_t AI0Reading;
volatile uint32_t AI1Reading;
volatile uint32_t AI2Reading;
volatile uint32_t AI3Reading;
volatile uint32_t AI4Reading;
volatile uint32_t AI5Reading;
volatile uint32_t AI6Reading;
volatile uint32_t TC1Reading;
volatile uint32_t TC2Reading;
volatile uint32_t TC3Reading;

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
void send_sensor_data();
bool send_sensor_data_ISR(struct repeating_timer *t);


void setup() {
  Serial1.begin(115200);
  //Serial.begin(115200);
  pinMode(LECUServoPwrSwitchPin, OUTPUT);
  pinMode(indicatorLEDPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  fuelValve.attach(fuelMainValvePWMPin, 500, 2500);
  oxValve.attach(oxMainValvePWMPin, 500, 2500);

  init_DAQ();
  delay(1500);

  // Timer ISR every 10 ms
  ITimer0.attachInterruptInterval(SENSOR_DATA_TX_INTERVAL*1000, send_sensor_data_ISR);
}

void loop() {
  recv_with_start_end_markers();
  if (newData) {
    process_ctrl_packet();
    newData = false;
  }

  if (sendDataFlag) {
    sendDataFlag = false;
    read_DAQ_module(AI0Reading, AI1Reading, AI2Reading, AI3Reading, AI4Reading, AI5Reading, AI6Reading, TC1Reading, TC2Reading, TC3Reading);
    send_sensor_data();
  }

  currentLEDMillis = millis();

  if ( ((LECUServoPwrSwitchState == 'L') || (LECUServoPwrSwitchState == 'H')) ) {
    if ((currentLEDMillis - lastLEDMillis) >= 1000) {
      lastLEDMillis = currentLEDMillis;
      LEDState = (LEDState == LOW) ? HIGH : LOW;
      digitalWrite(indicatorLEDPin, LEDState);
    }
  } else {
    digitalWrite(indicatorLEDPin, LOW);
  }
  
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

void send_sensor_data() {

  uint8_t packet[42];

  packet[0] = '<';

  uint32_t values[10] = {
    AI0Reading,
    AI1Reading,
    AI2Reading,
    AI3Reading,
    AI4Reading,
    AI5Reading,
    AI6Reading,
    TC1Reading,
    TC2Reading,
    TC3Reading
  };

  memcpy(&packet[1], values, sizeof(values));

  packet[41] = '>';

  Serial1.write(packet, sizeof(packet));
}

bool send_sensor_data_ISR(struct repeating_timer *t) {
  sendDataFlag = true;  // Set flag only
  return true;
}