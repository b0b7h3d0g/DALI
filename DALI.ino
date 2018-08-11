#include "Dali.h"

const int DALI_TX = D0;   // DALI Digital Out
const int DALI_RX_A = A0; // DALI Analog In
const int CYCLE_PIN = D3; // Push-button between D8 and GDN

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
  // initialize serial:
  Serial.begin(115200);
  pinMode(CYCLE_PIN, INPUT);

  dali.msgMode = true;

  dali.setupTransmit(DALI_TX);
  dali.setupAnalogReceive(DALI_RX_A);
  dali.busTest();

  help(); //Show help

  // TODO: restore state here
  // dali.transmit(BROADCAST_C, OFF_C);
  // dali.transmit(BROADCAST_C, ON_C);
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
  Serial.println("status - read analog port value");
  Serial.println("cycle - fade in/out using ARC");
  Serial.println("");
}

void cycle() {
  do {
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
  while(digitalRead(CYCLE_PIN) == 1 || Serial.available() == 0);
}


void showStatus() {
   int pinState = digitalRead(CYCLE_PIN);
   int analogValue = analogRead(DALI_RX_A);

   Serial.printf("Chip ID = %08X\n", ESP.getChipId());
   Serial.printf("DALI Level: %i\n", dali.analogLevel);
   Serial.printf("Analog Read: %f\n", analogValue * (3.2 / 1023.0)); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V)
   Serial.printf("Cycle Read: %s\n", (pinState)? "HIGH": "LOW");
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

  // trigger cycle on pin high
  if (digitalRead(CYCLE_PIN) == 1) {
    stringComplete = true;
    inputString = "cycle";
  }

  // execute the command
  if (stringComplete) {
    Serial.println("cmd: " + inputString);

    if (inputString == "cycle") {
      cycle();
    }; // fade-in-out

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

    if (inputString == "status") {
      showStatus();
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