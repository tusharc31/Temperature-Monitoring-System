#include "DHT.h"
#include <WiFi.h>
#include <WiFiMulti.h>

#define WIFI_NAME "OnePlus"
#define PASSWORD "56781234"
#define TIMEOUT  5000                              // Timeout for server response.
WiFiClient client;

// ThingSpeak information.
#define NUM_FIELDS 3                               // To update more fields, increase this number and add a field label below.
#define THING_SPEAK_ADDRESS "api.thingspeak.com"
String writeAPIKey="YHZDPHX3B3S1OZB6";             // Change this to the write API key for your channel.

int connectWifi()
{
    
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin( WIFI_NAME , PASSWORD );
        Serial.println( "Connecting to Wi-Fi" );
        delay( 2500 );
    }
    Serial.println( "Connected" );  // Inform the serial monitor.
}
// This function builds the data string for posting to ThingSpeak
    // and provides the correct format for the wifi client to communicate with ThingSpeak.
    // It posts numFields worth of data entries, and takes the
    // data from the fieldData parameter passed to it. 
  
int HTTPPost( int numFields , String fieldData[] ){
  
    if (client.connect( THING_SPEAK_ADDRESS , 80 )){

       // Build the postData string.  
       // If you have multiple fields, make sure the sting does not exceed 1440 characters.
       String postData= "api_key=" + writeAPIKey ;
       for ( int fieldNumber = 1; fieldNumber < numFields+1; fieldNumber++ ){
            String fieldName = "field" + String( fieldNumber );
            postData += "&" + fieldName + "=" + fieldData[ fieldNumber ];
            
            }

        // POST data via HTTP.
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

// Wait for a response from the server indicating availability,
// and then collect the response and build it into a string.

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

#define DHTPIN 4     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  connectWifi();
  Serial.println(F("DHTxx test!"));

  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

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

  String fieldData[ NUM_FIELDS ];  
  fieldData[1]=h;
  fieldData[2]=t;
  fieldData[3]=hif;
  HTTPPost( NUM_FIELDS , fieldData );
}
