#include "Dali.h"

const int DALI_TX = D0;
const int DALI_RX_A = A0;

#define BROADCAST_DP 0b11111110
#define BROADCAST_C 0b11111111
#define ON_DP 0b11111110
#define OFF_DP 0b00000000
#define ON_C 0b00000101
#define OFF_C 0b00000000
#define QUERY_STATUS 0b10010000
#define RESET 0b00100000

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  // initialize serial:
  Serial.begin(115200);

  dali.msgMode = true;

  dali.setupTransmit(DALI_TX);
  dali.setupAnalogReceive(DALI_RX_A);
  dali.busTest();
 
  help(); //Show help

  // TODO: restore state here
  // dali.transmit(BROADCAST_C, OFF_C);
  // dali.transmit(BROADCAST_C, ON_C);
  cycle();
}

void help() {
  Serial.println("==================================================");
  Serial.println("Enter 16 bit command or another command from list:");
  Serial.println("help -  command list");
  Serial.println("on -  broadcast on 100%");
  Serial.println("off -  broadcast off 0%");
  Serial.println("scan -  device short address scan");
  Serial.println("initialise -  start process of initialisation");
  Serial.println("test -  start receiving test");
  Serial.println("analog - read analog port value");
  Serial.println("cycle - fade in/out using ARC");
  Serial.println("");
}

void cycle() {
  Serial.println("initializing cycle()");

  dali.transmit(BROADCAST_C, ON_C);
  delay(20000); // wait for HPS

  while (Serial.available() == 0) {
    Serial.println("execute cycle fade-out");

    for (int i = 254; i > 192; i = i - 1) {
      dali.transmit(BROADCAST_DP, i);
      delay(2000);
    }

    delay(10000); // wait for HPS

    Serial.println("execute cycle fade-in");
    
    for (int i = 192; i < 255; i = i + 1) {
      dali.transmit(BROADCAST_DP, i);
      delay(2000);
    }

    delay(10000); // wait for HPS
  }
}

void testReceive () {
  dali.transmit(0, 254);
  delay(200);
  dali.transmit(1, 0x90);
  Serial.println(dali.receive());
}

void loop() {
  int cmd1;
  int cmd2;

  while (Serial.available()) {
    // get the new byte
    char inChar = (char)Serial.read();
    
    if (inChar == '\n' or inChar == '\r') {
      // if the incoming character is a newline, set a flag so the main loop can do something about it
      stringComplete = true;
    }
    else {
      // add it to the inputString
      inputString += inChar;
    }
  }

  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println("cmd: " + inputString);

    if (inputString == "cycle") {
      cycle();
    };

    if (inputString == "scan") {
      dali.scanShortAdd();
    }; // scan short addresses

    if (inputString == "on") {
      dali.transmit(BROADCAST_C, ON_C);
    }; // broadcast, 100%

    if (inputString == "off") {
      dali.transmit(BROADCAST_C, OFF_C);
    }; // broadcast, 0%

    if (inputString == "initialise" or inputString == "ini") {
      dali.initialisation();
    }; // initialisation

    if (inputString == "help") {
      help();
    }; //help

    if (inputString == "test") {
      testReceive();
    }; //graph

    if (inputString == "analog") {
      Serial.println(dali.analogLevel);
      Serial.println(analogRead(DALI_RX_A));
    };

    if (dali.cmdCheck(inputString, cmd1, cmd2)) {
      dali.transmit(cmd1, cmd2);  // command in binary format: (address byte, command byte)
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

  delay(100);
};

/* 
 * WeMOs D1 mini Pin Function - ESP-8266 Pin
 * =========================================
 * TX  TXD                            GPIO1
 * RX  RXD                            GPIO3
 * A0  Analog input, max 3.3V input   A0
 * D0  IO                             GPIO16
 * D1  IO, SCL                        GPIO5
 * D2  IO, SDA                        GPIO4
 * D3  IO,10k Pull-up                 GPIO0
 * D4  IO, 10k pull-up, BUILTIN_LED   GPIO2
 * D5  IO, SCK                        GPIO14
 * D6  IO, MISO                       GPIO12
 * D7  IO, MOSI                       GPIO13
 * D8  IO,10k pull-down, SS           GPIO15
 * G   Ground
 * 5V  5V
 * 3V3 3.3V
 * RST Reset
 * 
 * DALI ARC (Examples)
 * ===============================
 * 254 : 11111110 11111110 (158 W) (ON)
 * 252 : 11111110 11111100 (150 W)
 * 250 : 11111110 11111010 (145 W)
 * 248 : 11111110 11111000 (140 W)
 * 246 : 11111110 11110110 (135 W)
 * 244 : 11111110 11110100 (130 W)
 * 242 : 11111110 11110010 (125 W)
 * 240 : 11111110 11110000 (120 W)
 * 238 : 11111110 11101110 (115 W)
 * 236 : 11111110 11101100 (110 W)
 * 234 : 11111110 11101010 (107 W)
 * 232 : 11111110 11101000 (103 W)
 * 230 : 11111110 11100110 (100 W)
 * 228 : 11111110 11100100 (97 W)
 * 226 : 11111110 11100010 (93 W)
 * 224 : 11111110 11100000 (90 W)
 * 222 : 11111110 11011110 (87 W)
 * 220 : 11111110 11011100 (84 W)
 * 218 : 11111110 11011010 (82 W)
 * ...
 * 196 : 11111110 11000100 (60 W)
 * 192 : 11111110 11000000 (58 W)
 * 0   : 11111110 00000000 (58 W) (OFF)
 */