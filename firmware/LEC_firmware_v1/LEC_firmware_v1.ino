#include <Serial.h>
#include <Servo.h>

#define XBee Serial1

//::::::::::::::: LEC(End Device) ::::::::::::::::::::::::::

Servo fuelServo; //Initialize fuel servo object
Servo oxServo; //Initialize ox servo object

const int FUEL_PWM_PIN = 26; //Attach fuel servo to GPIO 22 (pin 10)
const int OX_PWM_PIN = 27; //Attach ox servo to GPIO 21 (pin 14)

const int OX_OPEN_ANGLE = 1000; //Define the pulse width that means "open"
const int OX_CLOSE_ANGLE = 2500; //Define the pulse width that means "close"
const int FUEL_OPEN_ANGLE = 1000;
const int FUEL_CLOSE_ANGLE = 2500;

const byte txHeader[] = {0x7E, 0x00, 0x0E, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x42, 0x54, 0x0B, 0xE6, 0xFF, 0xFE, 0x00, 0x00};
const byte msgTermChar[] = {0x00, 0x00, 0x00, 0x3E};

// Radio Message Parsing Variable  for (int i = 0; i < 4; i++){
    XBee.print(msgTermChar[i]);
  }s
bool parsed = true;
char incomingByte;
char msg[24];
int idx = 0;

// Throttling algo placeholder
bool tAlgoEnable = false;

void setup() {
  // Initialize Serial0 (USB), and Serial1 (XBEE UART)
  XBee.begin(115200);

  // Set servo power pin
  pinMode(28, OUTPUT); 

  // Peripherial init
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  fuelServo.attach(FUEL_PWM_PIN, FUEL_CLOSE_ANGLE, FUEL_OPEN_ANGLE);
  oxServo.attach(OX_PWM_PIN, OX_OPEN_ANGLE, OX_CLOSE_ANGLE);
}

void XBeeRx() {
  while (XBee.available() > 0) {
    incomingByte = XBee.read();
    msg[idx] = incomingByte;

    idx++;
    parsed = false;
  }
}

void XBeeTxNoise(){
  for (int i = 0; i < 17; i++){
    XBee.print(txHeader[i]);
  }

  for (int i = 0; i < 4; i++){
    XBee.print(msgTermChar[i]);
  }

  for (int j = 0; j < 52; j++){
    XBee.print((byte)random(256));
  }

  for (int i = 0; i < 4; i++){
    XBee.print(msgTermChar[i]);
  }

  XBee.print(0); // Dummy checksum
}

void Parse(){
  // Check that it's an RX packet 
  if (msg[3] == 0x90){
    // For all cmds, default to "safe state" if any command other than the high state is received
    if (msg[16] == 'H'){
      digitalWrite(28, HIGH);
    } else {
      digitalWrite(28, LOW);
    }

    if (msg[17] == 'O'){
      oxServo.writeMicroseconds(OX_OPEN_ANGLE);
    } else {
      oxServo.writeMicroseconds(OX_CLOSE_ANGLE);
    }

    if (msg[18] == 'O'){
      digitalWrite(LED_BUILTIN, HIGH);
      fuelServo.writeMicroseconds(FUEL_OPEN_ANGLE);
      oxServo.writeMicroseconds(OX_OPEN_ANGLE);
    }
    else{
      digitalWrite(LED_BUILTIN, LOW);
      fuelServo.writeMicroseconds(FUEL_CLOSE_ANGLE);
      oxServo.writeMicroseconds(OX_CLOSE_ANGLE);
    }
    
    tAlgoEnable = (msg[19] == 'T');
  }

  parsed = true;
  idx = 0;
}

void loop() {
  XBeeRx();
  
  // Only parse new unparsed data
  if (!parsed)
  {
    Parse();
  }

  XBeeTxNoise();

  delay(500)
}