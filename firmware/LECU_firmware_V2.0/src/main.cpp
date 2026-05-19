#include <Arduino.h>
#include <Servo.h>
#include <control.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <DAQ.h>
#include <Adafruit_SPIFlash.h>
#include <SdFat.h>
#define FLASH_CS   17
#define FLASH_SCK  18
#define FLASH_MISO 16
#define FLASH_MOSI 19
#define flashSPI SPI


// Pin Assignments
int indicatorLEDPin = 22;
int fuelMainValvePWMPin   = 26;
int oxMainValvePWMPin     = 27;
int LECUServoPwrSwitchPin = 28;
int fanCtrlPin = 14;
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

// ------------------- Flash IC ------------------------
Adafruit_FlashTransport_SPI flashTransport(
    FLASH_CS,
    flashSPI
);
Adafruit_SPIFlash flash(&flashTransport);

typedef struct __attribute__((packed)) {

    uint32_t timestamp_ms;
    uint32_t AI0;
    uint32_t AI1;
    uint32_t AI2;
    uint32_t AI3;
    uint32_t AI4;
    uint32_t AI5;
    uint32_t AI6;
    uint32_t TC1;
    uint32_t TC2;
    uint32_t TC3;

} SensorLogRecord;

uint32_t flashWriteAddress = 0;


// -------------------- Serial RX --------------------
const byte NUM_CHARS = 4;
char receivedChars[NUM_CHARS];
bool newData = false;

// -------------------- Timer --------------------
RPI_PICO_Timer ITimer0(0);
const int SENSOR_DATA_TX_INTERVAL = 200;   // in ms
volatile bool sendDataFlag = false;
volatile bool logDataFlag = false;
volatile uint32_t tick = 0;

// -------------------- Function Declarations --------------------
void recv_with_start_end_markers();
void process_ctrl_packet();
void send_sensor_data();
void log_sensor_data_to_flash();
bool send_sensor_data_ISR(struct repeating_timer *t);


void setup() {
  Serial1.begin(115200);
  //Serial.begin(115200);
  pinMode(LECUServoPwrSwitchPin, OUTPUT);
  pinMode(indicatorLEDPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(fanCtrlPin, OUTPUT);
  digitalWrite(fanCtrlPin, HIGH);

  fuelValve.attach(fuelMainValvePWMPin, 500, 2500);
  oxValve.attach(oxMainValvePWMPin, 500, 2500);

  init_DAQ();
  delay(1500);

  flashSPI.setRX(FLASH_MISO);
  flashSPI.setTX(FLASH_MOSI);
  flashSPI.setSCK(FLASH_SCK);
  flashSPI.begin();

  SPI.begin();

  if (!flash.begin()) {
    digitalWrite(LED_BUILTIN, HIGH);
    while (1) {
      delay(100);
    }
  }
  

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

  if (logDataFlag) {
    logDataFlag = false;
    log_sensor_data_to_flash();
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

void log_sensor_data_to_flash() {

    SensorLogRecord record;

    record.timestamp_ms = tick;
    record.AI0 = AI0Reading;
    record.AI1 = AI1Reading;
    record.AI2 = AI2Reading;
    record.AI3 = AI3Reading;
    record.AI4 = AI4Reading;
    record.AI5 = AI5Reading;
    record.AI6 = AI6Reading;
    record.TC1 = TC1Reading;
    record.TC2 = TC2Reading;
    record.TC3 = TC3Reading;

    // Erase 4KB sector when entering new sector
    if ((flashWriteAddress % 4096) == 0) {
      flash.eraseSector(flashWriteAddress);
    }

    flash.writeBuffer(
        flashWriteAddress,
        (uint8_t*)&record,
        sizeof(record)
    );

    flashWriteAddress += sizeof(record);
}

bool send_sensor_data_ISR(struct repeating_timer *t) {
  tick++;
  sendDataFlag = true;  // Set flag only
  if (tick % 5 == 0) {
    logDataFlag = true;
  }
  return true;
}