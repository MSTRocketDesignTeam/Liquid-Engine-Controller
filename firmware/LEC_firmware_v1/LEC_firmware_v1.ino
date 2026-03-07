#include <Serial.h>
#include <Servo.h>

#define XBee Serial1

// Open Frame: 7E 00 1C 10 01 00 13 A2 00 42 53 4C 53 FF FE 00 00 4F 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 70
// CLose Frame: 7E 00 1C 10 01 00 13 A2 00 42 53 4C 53 FF FE 00 00 43 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 2D 7C

//::::::::::::::: LEC(End Device) ::::::::::::::::::::::::::

Servo fuelServo; //Initialize fuel servo object
Servo oxServo; //Initialize ox servo object

const int FUEL_PWM_PIN = 22; //Attach fuel servo to GPIO 7 (pin 10)
const int OX_PWM_PIN = 21; //Attach ox servo to GPIO 10 (pin 14)

const int OX_OPEN_ANGLE = 1000; //Define the pulse width that means "open"
const int OX_CLOSE_ANGLE = 2500; //Define the pulse width that means "close"
const int FUEL_OPEN_ANGLE = 1000;
const int FUEL_CLOSE_ANGLE = 2500;

// Radio Message Parsing Variables
bool parsed = true;
char incomingByte;
char msg[32];
int idx = 0;

void setup() {
  // Initialize Serial0 (USB), and Serial1 (XBEE UART)
  Serial.begin(9600);
  XBee.begin(115200);

  // Set servo power pin
  pinMode(2, OPUTPUT);
  digitalWrite(2, HIGH);

  // Peripherial init
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  fuelServo.attach(FUEL_PWM_PIN, FUEL_CLOSE_ANGLE, FUEL_OPEN_ANGLE);
  oxServo.attach(OX_PWM_PIN, OX_OPEN_ANGLE, OX_CLOSE_ANGLE);
}

void XBeeRecv() {
  while (XBee.available() > 0) {
    incomingByte = XBee.read();
    msg[idx] = incomingByte;

    idx++;
    parsed = false;

    // Debug print statement
    Serial.print(incomingByte);
    Serial.print(", ");
    Serial.print(idx);
    Serial.print(", ");
    Serial.print(parsed);
    Serial.println();
  }
}

void Parse(){
  if (msg[15] == 'O'){
    digitalWrite(LED_BUILTIN, HIGH);
    fuelServo.writeMicroseconds(FUEL_OPEN_ANGLE);
    oxServo.writeMicroseconds(OX_OPEN_ANGLE);
    Serial.println("Opening valve");
  }
  else if (msg[15] == 'C'){
    digitalWrite(LED_BUILTIN, LOW);
    fuelServo.writeMicroseconds(FUEL_CLOSE_ANGLE);
    oxServo.writeMicroseconds(OX_CLOSE_ANGLE);
    Serial.println("Closing valve");
  }
  
  parsed = true;
  idx = 0;
  Serial.println("Parsed");
}

void loop() {  
  XBeeRecv();
  
  // Only parse new unparsed data
  if (!parsed)
  {
    Parse();
  }
}