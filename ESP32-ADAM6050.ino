/*
    ADAM-6050 emulator or ESP32 makes it possible to use ESP32 as a I/O module with Synology Surveilance Station
    Copyright (C) 2022  Janis Rocans

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
  This code should be able for ESP32 to act like a ADAM-6050 I/O module with Synology surveilance station, but using WiFi. I've tested it only with Az-Delivery ESP32-WROOM-32.
  Documentation used for reference: http://advdownload.advantech.com/productfile/Downloadfile4/1-1M99LTH/ADAM-6000_User_Manaul_Ed_9.pdf
  Change WiFi credentials before use.
  GPIO pins 36, 39, 34 and 35 do not have pull-up resistors on my ESP32 board, so external pull-up resistors should be used.
  I organised pins in order that was easier for me to solder jumper wires, so output pins might seem out of order.

  WARNING!!!
  This code does not implement password validation and session management and should not be used in external networks.
*/

#include <WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_WIFI_PASS"
#define UDP_PORT 1025
#define PORT_01 36
#define PORT_02 39
#define PORT_03 34
#define PORT_04 35
#define PORT_05 32
#define PORT_06 33
#define PORT_07 25
#define PORT_08 26
#define PORT_09 27
#define PORT_10 13
#define PORT_11 23
#define PORT_12 22
#define PORT_13 21
#define PORT_14 19
#define PORT_15 18
#define PORT_16 4
#define PORT_17 17
#define PORT_18 16

#define EEPROM_SIZE 12

WiFiUDP UDP;
char packet[255];
char reply[] = "Packet received!";

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

unsigned long previousMillis = 0;

// Not sure, how good or reliable this function is.
void checkWiFi() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= 10000)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    UDP.stop();
    WiFi.disconnect();
    WiFi.reconnect();
    UDP.begin(UDP_PORT);
    previousMillis = currentMillis;
  }
}

void setup() {
  pinMode(PORT_01, INPUT);
  pinMode(PORT_02, INPUT);
  pinMode(PORT_03, INPUT);
  pinMode(PORT_04, INPUT);
  pinMode(PORT_05, INPUT_PULLUP);
  pinMode(PORT_06, INPUT_PULLUP);
  pinMode(PORT_07, INPUT_PULLUP);
  pinMode(PORT_08, INPUT_PULLUP);
  pinMode(PORT_09, INPUT_PULLUP);
  pinMode(PORT_10, INPUT_PULLUP);
  pinMode(PORT_11, INPUT_PULLUP);
  pinMode(PORT_12, INPUT_PULLUP);
  pinMode(PORT_13, OUTPUT);
  pinMode(PORT_14, OUTPUT);
  pinMode(PORT_15, OUTPUT);
  pinMode(PORT_16, OUTPUT);
  pinMode(PORT_17, OUTPUT);
  pinMode(PORT_18, OUTPUT);

  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  initAllOutputs();
  initWiFi();
  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);
}

int getInputStates() {
  int states = 0;
  bitWrite(states, 0, !digitalRead(PORT_01));
  bitWrite(states, 1, !digitalRead(PORT_02));
  bitWrite(states, 2, !digitalRead(PORT_03));
  bitWrite(states, 3, !digitalRead(PORT_04));
  bitWrite(states, 4, !digitalRead(PORT_05));
  bitWrite(states, 5, !digitalRead(PORT_06));
  bitWrite(states, 6, !digitalRead(PORT_07));
  bitWrite(states, 7, !digitalRead(PORT_08));
  bitWrite(states, 8, !digitalRead(PORT_09));
  bitWrite(states, 9, !digitalRead(PORT_10));
  bitWrite(states, 10, !digitalRead(PORT_11));
  bitWrite(states, 11, !digitalRead(PORT_12));
  return states;
}

void loop() {
  checkWiFi();
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    // Serial.print("Received packet! Size: ");
    // Serial.println(packetSize);
    // Serial.print("Client: ");
    // Serial.print(UDP.remoteIP());
    // Serial.print(":");
    // Serial.println(UDP.remotePort());
    int len = UDP.read(packet, 255);

    if (len > 0)
    {
      packet[len] = '\0';
    }
    // Serial.print("Packet received: ");
    // Serial.println(packet);

    // TODO - DUMMY LOGIN. No idea, how it is supposed to work and how sessions should be managed. Duplicated functionality from other projects. No information in documentation.
    if (startsWith(packet, "$01PW", 5)) {
      UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
      UDP.print(">01\r");
      UDP.endPacket();
      return;
    }

    // TODO - Request IO status. Documentation is not clear regarding this command.
    if (startsWith(packet, "$01C", 4)) {
      UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
      UDP.print("!01000000000000000000000000000000000000\r");
      UDP.endPacket();
      return;
    }

    // Request IO input status
    if (startsWith(packet, "$016", 4)) {
      UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
      int inputs = getInputStates();
      char hex [11];
      sprintf(hex, "!0100%04X\r", inputs);
      UDP.print(hex);
      UDP.endPacket();
      return;
    }

    // Request set IO output
    if (startsWith(packet, "#01", 3)) {
      byte channel = 16;
      byte status = packet[6] == '0' ? 0 : 1;
      if (packet[3] == '1') {
        if (packet[4] < 'A') {
          channel = (byte)packet[4] - 48;
        } else {
          channel = (byte)packet[4] - 55;
        }
        if (channel < 16) {
          setOutput(channel, status);
          UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
          UDP.print(">\r");
          UDP.endPacket();
          return;
        }
      }
      if (packet[3] == '0' && packet[4] == '0') { // Not used by Synology surveilance station, so can not be verified
        setAllOutputs(status);
        UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
        UDP.print(">\r");
        UDP.endPacket();
        return;
      }
    }
  }
}

void setOutput(byte channel, byte status) {
  switch (channel) {
    case 0: {
        digitalWrite(PORT_13, status);
        break;
      }
    case 1: {
        digitalWrite(PORT_14, status);
        break;
      }
    case 2: {
        digitalWrite(PORT_15, status);
        break;
      }
    case 3: {
        digitalWrite(PORT_16, status);
        break;
      }
    case 4: {
        digitalWrite(PORT_17, status);
        break;
      }
    case 5: {
        digitalWrite(PORT_18, status);
        break;
      }
    default: {
      return;
      }
  }
  EEPROM.write(channel, status);
  EEPROM.commit();
}

void setAllOutputs(byte status) {
  setOutput(0, status);
  setOutput(1, status);
  setOutput(2, status);
  setOutput(3, status);
  setOutput(4, status);
  setOutput(5, status);
}

void initAllOutputs() {
  setOutput(0, EEPROM.read(0));
  setOutput(1, EEPROM.read(1));
  setOutput(2, EEPROM.read(2));
  setOutput(3, EEPROM.read(3));
  setOutput(4, EEPROM.read(4));
  setOutput(5, EEPROM.read(5));
}

boolean startsWith(char* target, String expected, int length) {
  for (int i = 0; i < length; i++) {
    if (target[i] != expected[i]) {
      return false;
    }
  }
  return true;
}
