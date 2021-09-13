// Embedded Systems Workshop - Lab 2 and 3
// Tushar Choudhary

#include "DHT.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include "BluetoothSerial.h"

// Defining the max safe temperature
int safe_t=15;

// Pin numbers for LEDs and buzzer
const byte alert = 23;
const byte not_alert = 22;

// WiFi and Bluetooh details
#define WIFI_NAME "OnePlus"
#define PASSWORD "56781234"
#define TIMEOUT  5000                              // Timeout for server response.
BluetoothSerial SerialBT;
WiFiClient client;

// ThingSpeak details
#define NUM_FIELDS 3                               // To update more fields, increase this number and add a field label below.
#define THING_SPEAK_ADDRESS "api.thingspeak.com"
String writeAPIKey="YHZDPHX3B3S1OZB6";             // Change this to the write API key for your channel.

// For connecting to WiFi
int connectWifi()
{   
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin( WIFI_NAME , PASSWORD );
        Serial.println( "Connecting to Wi-Fi" );
        delay( 2500 );
    }
    Serial.println( "Connected" );  // Inform the serial monitor.
}

// For updating the data obtained on ThingSpeak
int HTTPPost( int numFields , String fieldData[] ){
  
    if (client.connect( THING_SPEAK_ADDRESS , 80 )){

       // Build the postData string.  
       // If you have multiple fields, make sure the sting does not exceed 1440 characters.
       String postData= "api_key=" + writeAPIKey ;
       for ( int fieldNumber = 1; fieldNumber < numFields+1; fieldNumber++ ){
            String fieldName = "field" + String( fieldNumber );
            postData += "&" + fieldName + "=" + fieldData[ fieldNumber ];
            
            }

        // Print statements for debugging
        Serial.println( "Connecting to ThingSpeak for update..." );
        Serial.println();
        client.println( "POST /update HTTP/1.1" );
        client.println( "Host: api.thingspeak.com" );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( postData.length() ) );
        client.println();
        client.println( postData );
        Serial.println( postData );
        String answer=getResponse();
        Serial.println( answer );
    }
    else
    {
      Serial.println ( "Connection Failed" );
    }
    
}

// Function for getting the acknowledgement from ThingSpeak after posting data
String getResponse(){
  String response;
  long startTime = millis();

  delay( 200 );
  while ( client.available() < 1 && (( millis() - startTime ) < TIMEOUT ) ){
        delay( 5 );
  }
  
  if( client.available() > 0 ){ // Get response from server.
     char charIn;
     do {
         charIn = client.read(); // Read a char from the buffer.
         response += charIn;     // Append the char to the string response.
        } while ( client.available() > 0 );
    }
  client.stop();
        
  return response;
}

// Input pin for Lm35
int lm35=13;

// Detials for DHT22 sensor
#define DHTPIN 4    
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(lm35, INPUT); 
  SerialBT.begin("Tushar ESW");
  connectWifi();
  Serial.println(F("DHTxx test!"));
  pinMode(alert, OUTPUT);
  pinMode(not_alert, OUTPUT);
  dht.begin();
}

void loop() {

  // A short delay after every measurement
  delay(30000);
  
  // Getting the data from DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  // Getting data from Lm35
  int analogValue = analogRead(lm35);
  float millivolts = (analogValue/1024.0) * 3300;
  float lmc = millivolts/10;
  float lmf = ((lmc * 9)/5 + 32);

  t = (t+lmc)/2;
  f = (f+lmf)/2;
  
  // Printing the data on serial monitor
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("째C "));
  Serial.print(f);
  Serial.print(F("째F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("째C "));
  Serial.print(hif);
  Serial.println(F("째F"));

  // Printing the data to bluetooth connected mobile device
  SerialBT.println("Humidity: ");
  SerialBT.println(h);
  SerialBT.println("Temperature (celsius): ");
  SerialBT.println(t);
  SerialBT.println("Heat Index: ");
  SerialBT.println(hic);
  SerialBT.println(" ");

  // Checking if the data lies in the safe range
  if (t>=safe_t)
  {
    SerialBT.println("Temperature is outside the safety range!");
    Serial.println("Temperature is outside the safety range!");
    digitalWrite(alert, HIGH);
    digitalWrite(not_alert, LOW);   
  }

  else
  {
    digitalWrite(alert, LOW);
    digitalWrite(not_alert, HIGH);   
  }

  // Calling the function to update data on ThingSpeak
  String fieldData[ NUM_FIELDS ];  
  fieldData[1]=h;
  fieldData[2]=t;
  fieldData[3]=hif;
  HTTPPost( NUM_FIELDS , fieldData );
}
