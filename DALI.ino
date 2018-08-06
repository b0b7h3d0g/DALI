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

  dali.setupTransmit(DALI_TX);
  dali.setupAnalogReceive(DALI_RX_A);
  dali.busTest();
  dali.msgMode = true;

  Serial.println("Analog Level: " + dali.analogLevel);
  help(); //Show help
}


void help() {
  Serial.println("Enter 16 bit command or another command from list:");
  Serial.println("help -  command list");
  Serial.println("on -  broadcast on 100%");
  Serial.println("off -  broadcast off 0%");
  Serial.println("scan -  device short address scan");
  Serial.println("initialise -  start process of initialisation");
  Serial.println();
}

void sinus () {
  uint8_t lf_1_add = 0;
  uint8_t lf_2_add = 1;
  uint8_t lf_3_add = 2;
  uint8_t lf_1;
  uint8_t lf_2;
  uint8_t lf_3;
  int i;
  int j = 0;

  while (Serial.available() == 0) {
    Serial.println("execute sinus() function");
    
    for (i = 0; i < 360; i = i + 1) {

      if (Serial.available() != 0) {
        dali.transmit(BROADCAST_C, ON_C);
        break;
      }

      lf_1 = (int) abs(254 * sin(i * 3.14 / 180));
      lf_2 = (int) abs(254 * sin(i * 3.14 / 180 + 2 * 3.14 / 3));
      lf_3 = (int) abs(254 * sin(i * 3.14 / 180 + 1 * 3.14 / 3));

      dali.transmit(lf_1_add << 1, lf_1);
      delay(5);
      dali.transmit(lf_2_add << 1, lf_2);
      delay(5);
      dali.transmit(lf_3_add << 1, lf_3);
      delay(25);
    }
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
    
    if (inChar == '\n') {
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

    if (inputString == "sinus") {
      sinus();
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
 * WeMos D1 mini Pin Number - Arduino IDE Pin Number
 * D0  16
 * D1  5
 * D2  4
 * D3  0
 * D4  2
 * D5  14
 * D6  12
 * D7  13
 * D8  15
 * TX  1
 * RX  3
 * 
 * WeMOs D1 mini Pin Function - ESP-8266 Pin
 * TX  TXD TXD
 * RX  RXD RXD
 * A0  Analog input, max 3.3V input  A0
 * D0  IO  GPIO16
 * D1  IO, SCL GPIO5
 * D2  IO, SDA GPIO4
 * D3  IO,10k Pull-up  GPIO0
 * D4  IO, 10k pull-up, BUILTIN_LED  GPIO2
 * D5  IO, SCK GPIO14
 * D6  IO, MISO  GPIO12
 * D7  IO, MOSI  GPIO13
 * D8  IO,10k pull-down, SS  GPIO15
 * G Ground  GND
 * 5V  5V  –
 * 3V3 3.3V  3.3V
 * RST Reset RST
  */
