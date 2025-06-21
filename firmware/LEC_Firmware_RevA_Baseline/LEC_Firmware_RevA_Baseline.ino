#include <SoftwareSerial.h>          
#include <Servo.h>

//::::::::::::::: LEC(End Device) ::::::::::::::::::::::::::

SerialPIO XBee(0, 1); //Tx = 0, Rx = 1

Servo fuelServo; //Initialize fuel servo object
Servo oxServo; //Initialize ox servo object

const int FUEL_PWM_PIN = 22; //Attach fuel servo to GPIO 7 (pin 10)
const int OX_PWM_PIN = 21; //Attach ox servo to GPIO 10 (pin 14)

const int OX_OPEN_ANGLE = 715; //Define the pulse width that means "open"
const int OX_CLOSE_ANGLE = 1700; //Define the pulse width that means "close"
const int FUEL_OPEN_ANGLE = 550;
const int FUEL_CLOSE_ANGLE = 1500;

//Radio Message Parsing Variables
bool started = false;  
bool ended = false;    
char incomingByte;     
char msg[5];           
byte indx = 0;         
float txMsg[9];
const float x = 1; 

void setup() {
  //Serial1.setTX(0);
  //Serial1.setRX(1);
  XBee.begin(9600);                  
  randomSeed(analogRead(0));         
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(16, INPUT);
  fuelServo.attach(FUEL_PWM_PIN, 500, 2500);
  oxServo.attach(OX_PWM_PIN, 500, 2500);
}

void XBeeRecv() {
  while (XBee.available() > 0) {  
    incomingByte = XBee.read();   

    if (incomingByte == '{') {  
      started = true;
      indx = 0;
      msg[indx] = '\0';                
    } else if (incomingByte == '}') {  
      ended = true;
      break;                           
    } else if (started && indx < 4) {  
      msg[indx] = incomingByte;
      indx++;
      msg[indx] = '\0';  
    }
  }
}

void Parse() {
  if (started && ended) {   
    if (msg[3] == 'A') {
      if (msg[0] == 'T') {
      digitalWrite(LED_BUILTIN, HIGH);
      fuelServo.writeMicroseconds(FUEL_OPEN_ANGLE);
      oxServo.writeMicroseconds(OX_OPEN_ANGLE);
      } else if (msg[0] == 'F') {
        digitalWrite(LED_BUILTIN, LOW);
        fuelServo.writeMicroseconds(FUEL_CLOSE_ANGLE);
        oxServo.writeMicroseconds(OX_CLOSE_ANGLE);
      }
    } else if (msg[3] == 'M') {
      if (msg[1] == 'T') {
        digitalWrite(LED_BUILTIN, HIGH);
      } else if (msg[1] == 'F') {
        digitalWrite(LED_BUILTIN, LOW);
      }
      if (msg[2] == 'T') {
        fuelServo.writeMicroseconds(FUEL_OPEN_ANGLE);
        oxServo.writeMicroseconds(OX_OPEN_ANGLE);
      } else if (msg[2] == 'F') {
        fuelServo.writeMicroseconds(FUEL_CLOSE_ANGLE);
        oxServo.writeMicroseconds(OX_CLOSE_ANGLE);
      }
    }
    started = false;
    ended = false;
  }
}


void loop() {  
  XBeeRecv();
  Parse();  
}
