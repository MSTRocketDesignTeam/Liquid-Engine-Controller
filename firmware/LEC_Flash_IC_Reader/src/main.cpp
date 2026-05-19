#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>

// ---------------- FLASH CONFIG ----------------

#define FLASH_CS 17
#define FLASH_SCK  18
#define FLASH_MISO 16
#define FLASH_MOSI 19

SPISettings flashICsettings(10000000, MSBFIRST, SPI_MODE0);
Adafruit_FlashTransport_SPI flashTransport(FLASH_CS, SPI);
Adafruit_SPIFlash flash(&flashTransport);

// ---------------- GPIO CONFIG ----------------

#define START_PIN 2

// ---------------- LOG RECORD STRUCT ----------------

// MUST MATCH EXACTLY with the logging struct
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

// ---------------- FLASH CONFIG ----------------

// Adjust if needed
const uint32_t FLASH_START_ADDR = 0;

// W25Q128 = 16 MB
const uint32_t FLASH_SIZE_BYTES = 16 * 1024 * 1024;

const uint32_t RECORD_SIZE = sizeof(SensorLogRecord);

// ---------------- STATE ----------------

bool startedDump = false;

// ---------------- FUNCTION DECLARATIONS ----------------

void print_record(const SensorLogRecord &record);

// ------------------------------------------------------
// SETUP
// ------------------------------------------------------

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(START_PIN, INPUT);

    Serial.begin(115200);

    delay(2000);

    SPI.begin();

    if (!flash.begin()) {

        Serial.println("FLASH INIT FAILED");

        while (1) {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(100);
        }
    }

    Serial.println("FLASH READY");
    Serial.println("Waiting for GPIO2 HIGH to begin dump...");
}

// ------------------------------------------------------
// LOOP
// ------------------------------------------------------

void loop() {

    if (!startedDump) {

        if (digitalRead(START_PIN) == HIGH) {

            startedDump = true;

            Serial.println("STARTING FLASH DUMP...");
            Serial.println();
        } else {

            delay(10);
            return;
        }
    }

    SensorLogRecord record;

    uint32_t addr = FLASH_START_ADDR;

    while ((addr + RECORD_SIZE) <= FLASH_SIZE_BYTES) {

        flash.readBuffer(
            addr,
            (uint8_t*)&record,
            RECORD_SIZE
        );

        // Detect unwritten flash
        // Flash defaults to all 0xFF after erase
        if (record.timestamp_ms == 0xFFFFFFFF) {

            Serial.println();
            Serial.println("END OF LOG");
            break;
        }

        print_record(record);

        addr += RECORD_SIZE;

        delay(1);
    }

    Serial.println();
    Serial.println("FLASH DUMP COMPLETE");

    while (1) {
        delay(1000);
    }
}

// ------------------------------------------------------
// PRINT RECORD
// ------------------------------------------------------

void print_record(const SensorLogRecord &r) {

    Serial.print(r.timestamp_ms);

    Serial.print(',');

    Serial.print(r.AI0);
    Serial.print(',');

    Serial.print(r.AI1);
    Serial.print(',');

    Serial.print(r.AI2);
    Serial.print(',');

    Serial.print(r.AI3);
    Serial.print(',');

    Serial.print(r.AI4);
    Serial.print(',');

    Serial.print(r.AI5);
    Serial.print(',');

    Serial.print(r.AI6);
    Serial.print(',');

    Serial.print(r.TC1);
    Serial.print(',');

    Serial.print(r.TC2);
    Serial.print(',');

    Serial.println(r.TC3);
}