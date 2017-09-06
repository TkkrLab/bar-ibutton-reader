/* 
 *  USB iButton reader for the barsystem
 *  Renze Nicolai 2017
 */

#include <OneWire.h>
#include "sha256.h"

/*** Configuration ***/
#define FW_VERSION 3

/*** Pins ***/
#define ONEWIRE_PIN 8
#define ONEWIRE_LED 9
#define INTERNAL_LED 13

/*** Variables ***/
String sha256string = "";

/*** Objects ***/
OneWire ds(ONEWIRE_PIN);

/*** Main program ***/
void setup() {
  Serial.begin(115200);
  pinMode(ONEWIRE_LED, OUTPUT);
  pinMode(INTERNAL_LED, OUTPUT);
  Serial.println("debug{Tkkrlab iButton reader}");
  Serial.println("debug{Renze Nicolai 2017}");
  Serial.println("debug{Firmware version: "+String(FW_VERSION)+"}");
}

unsigned long heartbeat_time = 0;
uint8_t heartbeat = 0;
bool newdevice = true;

void loop() {
  if (heartbeat_time<millis()) {
    heartbeat_time=millis()+100;
    heartbeat++;
  }
  
  //OneWire
  bool present = ds.reset();
  ds.skip();
  if (present) { //Device present on the bus
    uint8_t addr_in[8] = {0};
    uint8_t addr[8] = {0};
    if (ds.search(addr_in)) {
      if (newdevice) {
        digitalWrite(ONEWIRE_LED, true);
        digitalWrite(INTERNAL_LED, true);
        if (ds.crc8( addr_in, 7) == addr_in[7]) {  

          /*for (uint8_t i = 0; i<8; i++) {
            Serial.print(addr_in[i], HEX);
            if (i<7) Serial.print(":");
          }
          Serial.println();*/
          //getOneWireInfo(addr_in[0]);
          for (uint8_t i = 0; i<8; i++) {
            addr[i] = addr_in[7-i];
          }
          
          //Serial.print("x{");

          Sha256.init();         
          Sha256.print("42SUPERSECRETSALT1337");
          for (uint8_t i = 0; i<7; i++) { Sha256.write(addr[i]); }
          sha256string = "";
          uint8_t* hash = Sha256.result();
          for (uint8_t i = 0; i<32; i++) {
            sha256string = sha256string+("0123456789abcdef"[hash[i]>>4])+("0123456789abcdef"[hash[i]&0xf]);
          }
          //Serial.println(idstring);
          Serial.println("sha256{"+sha256string+"}");

          String idstring = "";
          for (uint8_t i = 0; i<8; i++) {
            idstring = idstring + ("0123456789abcdef"[addr_in[i]>>4]) + ("0123456789abcdef"[addr_in[i]&0xf]);
          }
          Serial.println("id{"+idstring+"}");

          
          digitalWrite(ONEWIRE_LED, HIGH);
          digitalWrite(INTERNAL_LED, HIGH);
          delay(2000);
          digitalWrite(ONEWIRE_LED, LOW);
          digitalWrite(INTERNAL_LED, LOW);
        } else {
          Serial.println("error{Invalid CRC}");
        }
      }
      newdevice = false;
    } else {
      ds.reset_search();
    }
  } else {
    newdevice = true;
    bool heartbeat_led = (heartbeat&0b00001000)>>3;
    //digitalWrite(ONEWIRE_LED, heartbeat_led);
    digitalWrite(INTERNAL_LED, heartbeat_led);
  }
}
