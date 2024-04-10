#include <FastLED.h>

#include <WiFiClientSecure.h>

//#include "FastLED.h"
#define  NUM_LEDS 77
#define DATA_PIN 6
#define CLOCK_PIN 7


CRGB leds[NUM_LEDS];
int current_state = 0;

const char* ssid = "unisiegen";
const char* password = "eLab4Zimtsuper#sicher";

const char*  server = "www.elab-siegen.de";  // Server URL
const char* url = "https://elab-siegen.de"; 
String door_status[3] = {"open","closed","brb"};

WiFiClientSecure client;

void logomove();
int stat();
void check(int state);
void post_request(int state);

void setup() {

  Serial.begin(115200);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);

  //wifi part below
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);

  client.setInsecure();

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
     client.stop();
  }
  
  
}

void loop() {
  FastLED.clear();
  
  current_state = stat(); 
 
  logomove();
  
  check(current_state);
  
  FastLED.show();
  Serial.print(current_state);
  delay(3000); 

}

void logomove()
{
  for(int led = 0; led < 47; led++)
    {
      leds[led] = CRGB::Yellow;
      FastLED.show();
      leds[led] = CRGB::Yellow;
      delay(50);
    }
}


int stat()
{
  int newCol = analogRead(14);
  int a = 0;
  Serial.println(newCol); 

 if (newCol < 750) // enspricht < 1,67V 341
  { a = 1;
    for(int led = 47; led < NUM_LEDS-20; led++) {
        leds[led] = CRGB::Green;
      }
    for(int led = 57; led < NUM_LEDS; led++) {
        leds[led] = CRGB::Black; 
    }
  }
  else if (newCol >= 4000) // entspricht >= 3,34 V 682
  { a = 2;

    for(int led = 47; led < NUM_LEDS-20; led++) {
        leds[led] = CRGB::Black; 
    }
    for(int led = 57; led < NUM_LEDS-10; led++) {
         leds[led] = CRGB::Blue;
    }
    for(int led = 67; led < NUM_LEDS; led++) {
        leds[led] = CRGB::Black;
    }
  }
  else // enspricht > 1,67V und < 3,34V
  { a = 3;
    for(int led = 47; led < NUM_LEDS-10; led++) {
        leds[led] = CRGB::Black;
    }
    for(int led = 67; led < NUM_LEDS; led++) {
       leds[led] = CRGB( 0, 89, 255);    }
          }
  
  return a;
}


void check(int state){
  static int prev_state=0;
  if (state!=prev_state){
    prev_state=state;    
    post_request(state);    
  }  
}

void post_request(int state){
  String body;
  int body_len = 0;
  client.setInsecure();

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    //Request
    client.println("POST " + (String)url + " HTTP/1.0");
    //Headers
    client.println("Host: " + (String)server); 
    client.println("Content-Type: application/x-www-form-urlencoded");

    body = door_status[state-1];
    body_len = body.length();
    
    client.print("Content-Length: "); client.println(body_len);
    client.println("Connection: Close");
    client.println();
    //Body
    client.println(body);
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("website updated");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    //while (client.available()) {
    //  char c = client.read();
    //  //Serial.write(c);
    //}
    
    client.stop();
  }
}
