/*
 * MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * The library file MFRC522.h has a wealth of useful info. Please read it.
 * The functions are documented in MFRC522.cpp.
 *
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by Søren Thing Andersen (access.thing.dk), fall of 2013 (Translation to English, refactored, comments, anti collision, cascade levels.)
 * Released into the public domain.
 *
 * Sample program showing how to read data from a PICC using a MFRC522 reader on the Arduino SPI interface.
 *----------------------------------------------------------------------------- empty_skull 
 * Aggiunti pin per arduino Mega
 * add pin configuration for arduino mega
 * http://mac86project.altervista.org/
 ----------------------------------------------------------------------------- Nicola Coppola
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               51                MOSI
 * SPI MISO   12               50                MISO
 * SPI SCK    13               52                SCK
 *
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on ebay.com. 
 */
#include <SPI.h>
#include <MFRC522.h>
#include<string.h> 
#include <stdio.h>

#define SS_PIN 53
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

/*ETHERNET*/
#include <ArduinoJson.h>
#include <Ethernet.h>

EthernetClient clientGET;

const size_t capacity =  JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(6) + 500;
DynamicJsonDocument docG(capacity);

void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
      
    /*ETHERNET*/
    while (!Serial) continue;

    // Initialize Ethernet library
    byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    if (!Ethernet.begin(mac)) {
      Serial.println(F("Failed to configure Ethernet"));
      return;
    }
    
    delay(1000);
  
    Serial.println(F("Connecting..."));

    // Connect to HTTP server
    clientGET.setTimeout(10000);
    if (!clientGET.connect("accesswave.herokuapp.com", 80)) {
      Serial.println(F("Connection failed"));
      return;
    }
  
    Serial.println(F("Connected!"));
    SPI.begin();            // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    Serial.println("Scan PICC to see UID and type...");
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Send HTTP request
    clientGET.println("GET /device HTTP/1.0");
    clientGET.println("Host: accesswave.herokuapp.com");
    clientGET.println("Connection: close");
    if (clientGET.println() == 0) {
      Serial.println(F("Failed to send request"));
      return;
    }
  
    // Check HTTP status
    char statusG[32] = {0};
    clientGET.readBytesUntil('\r', statusG, sizeof(statusG));
    if (strcmp(statusG, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(statusG);
      return;
    }
  
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!clientGET.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }
    
}
int hexadecimalToDecimal(char hexVal[]) 
{    
    int len = strlen(hexVal); 
      
    // Initializing base value to 1, i.e 16^0 
    int base = 1; 
      
    int dec_val = 0; 
      
    // Extracting characters as digits from last character 
    for (int i=len-1; i>=0; i--) 
    {    
        // if character lies in '0'-'9', converting  
        // it to integral 0-9 by subtracting 48 from 
        // ASCII value. 
        if (hexVal[i]>='0' && hexVal[i]<='9') 
        { 
            dec_val += (hexVal[i] - 48)*base; 
                  
            // incrementing base by power 
            base = base * 16; 
        } 
  
        // if character lies in 'A'-'F' , converting  
        // it to integral 10 - 15 by subtracting 55  
        // from ASCII value 
        else if (hexVal[i]>='A' && hexVal[i]<='F') 
        { 
            dec_val += (hexVal[i] - 55)*base; 
          
            // incrementing base by power 
            base = base*16; 
        } 
    } 
      
    return dec_val; 
} 

void loop() {
  
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
        return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {
        return;     
    } 
  
    // Extract values
    DeserializationError error = deserializeJson(docG, clientGET);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    int code = 0;
    code = docG[0]["code"].as<int>();
    char* firstBlock = "";
    firstBlock = docG[0]["firstBlock"].as<char*>();
    char* secondBlock = "";
    secondBlock = docG[0]["secondBlock"].as<char*>();
    char* thirdBlock = "";
    thirdBlock = docG[0]["thirdBlock"].as<char*>();
    char* fourthBlock = "";
    fourthBlock = docG[0]["fourthBlock"].as<char*>();
    char* userName = "";
    userName = docG[0]["userName"].as<char*>();
    
    //docG[0]["firstBlock"].as<char*>();
    Serial.println(F("Response:"));  
    Serial.println(docG[0]["code"].as<int>());
    Serial.println(docG[0]["firstBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[0]);
    Serial.println(docG[0]["secondBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[1]);
    Serial.println(docG[0]["thirdBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[2]);
    Serial.println(docG[0]["fourthBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[3]); 
    Serial.println(docG[0]["userName"].as<char*>());
      
    if (mfrc522.uid.uidByte[0] == hexadecimalToDecimal(firstBlock)
        && mfrc522.uid.uidByte[1] == hexadecimalToDecimal(secondBlock)
          && mfrc522.uid.uidByte[2] == hexadecimalToDecimal(thirdBlock)
            && mfrc522.uid.uidByte[3] == hexadecimalToDecimal(fourthBlock)) {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)     
                  
        EthernetClient clientPOST;
        DynamicJsonDocument docP(capacity);  
        
        // Connect to HTTP server
        clientPOST.setTimeout(10000);
        if (!clientPOST.connect("accesswave.herokuapp.com", 80)) {
          Serial.println(F("Connection failed"));
          return;
        }
      
        Serial.println(F("Connected!"));
        // Send HTTP request
        clientPOST.print("GET /access/auth?code=");
        clientPOST.print(code);
        clientPOST.println(" HTTP/1.0");        
        clientPOST.println("Host: accesswave.herokuapp.com");
        clientPOST.println("Connection: close");
        
        if (clientPOST.println() == 0) {
          Serial.println(F("Failed to send request"));
          return;
        }
      
        // Check HTTP status
        char statusP[32] = {0};
        clientPOST.readBytesUntil('\r', statusP, sizeof(statusP));
        if (strcmp(statusP, "HTTP/1.1 200 OK") != 0) {
          Serial.print(F("Unexpected response: "));
          Serial.println(statusP);
          return;
        }
      
        // Skip HTTP headers
        char endOfHeadersP[] = "\r\n\r\n";
        if (!clientPOST.find(endOfHeadersP)) {
          Serial.println(F("Invalid response"));
          return;
        }
        
        // Extract values
        DeserializationError errorP = deserializeJson(docP, clientPOST);
        if (errorP) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(errorP.c_str());
          return;
        }
                
        Serial.println(F("Response:"));  
        Serial.println(docP["code"].as<long>());
        Serial.println(docP["entry"].as<char*>());
        
        clientPOST.stop();
    }else{              
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    }
    /*
    */

    
    // Dump debug info about the card. PICC_HaltA() is automatically called.
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    
    // Disconnect
    clientGET.stop();
    delay(1000);

}
