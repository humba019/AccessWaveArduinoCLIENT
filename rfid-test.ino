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

#define SS_PIN 53
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

/*ETHERNET*/
#include <ArduinoJson.h>
#include <Ethernet.h>

EthernetClient client;

const size_t capacity =  JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(6) + 500;
DynamicJsonDocument doc(capacity);

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
    client.setTimeout(10000);
    if (!client.connect("accesswave.herokuapp.com", 80)) {
      Serial.println(F("Connection failed"));
      return;
    }
  
    Serial.println(F("Connected!"));
    SPI.begin();            // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    Serial.println("Scan PICC to see UID and type...");
    pinMode(LED_BUILTIN, OUTPUT);
    
    // Send HTTP request
    client.println(F("GET /device HTTP/1.0"));
    client.println(F("Host: accesswave.herokuapp.com"));
    client.println(F("Connection: close"));
    if (client.println() == 0) {
      Serial.println(F("Failed to send request"));
      return;
    }
  
    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }
  
    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
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
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    char* firstBlock = "";
    firstBlock = doc[0]["firstBlock"].as<char*>();
    char* secondBlock = "";
    secondBlock = doc[0]["secondBlock"].as<char*>();
    char* thirdBlock = "";
    thirdBlock = doc[0]["thirdBlock"].as<char*>();
    char* fourthBlock = "";
    fourthBlock = doc[0]["fourthBlock"].as<char*>();
    
    //doc[0]["firstBlock"].as<char*>();
    Serial.println(F("Response:"));  
    Serial.println(doc[0]["code"].as<long>());
    Serial.println(doc[0]["firstBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[0]);
    Serial.println(doc[0]["secondBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[1]);
    Serial.println(doc[0]["thirdBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[2]);
    Serial.println(doc[0]["fourthBlock"].as<char*>());
    Serial.println(mfrc522.uid.uidByte[3]); 
      
    if (mfrc522.uid.uidByte[0] == hexadecimalToDecimal(firstBlock)
        && mfrc522.uid.uidByte[1] == hexadecimalToDecimal(secondBlock)
          && mfrc522.uid.uidByte[2] == hexadecimalToDecimal(thirdBlock)
            && mfrc522.uid.uidByte[3] == hexadecimalToDecimal(fourthBlock)) {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)                 
    }else{              
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    }
    /*
    */

    
    // Dump debug info about the card. PICC_HaltA() is automatically called.
    mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
    
    // Disconnect
    client.stop();
    delay(1000);

}
