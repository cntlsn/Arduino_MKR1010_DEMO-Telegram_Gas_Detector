
/*
  ++ TELEGRAM GAS DETECTOR ++
  
  DESCRIPTION:
  Reads Gas value from a Grove kit Gas Sensor(MQ9)
  and sends an HTTP request to an IFTTT applet that
  triggers a message on a Telegram channel.
  TIP - Don't forget to insert your WiFi Network name and Password
  TIP2 - Don't forget to insert your IFTTT key and event name
    
  CIRCUIT:
  Connect the Gas sensor to analog pin A4.
  Connect the NeoPixel strip to digital pin 0.

  NOTES:
  IFTTT applet is here https://ifttt.com/applets/78085983d-if-maker-event-lpg_sensed-then-send-message-to-channel-lpg_detector-lpg-detector
  Telegram channel is @lpg_detector
 
  This example code is in the public domain.
  Author: Alessandro Contini 
*/

#include <Adafruit_NeoPixel.h>
#include <WiFi1010.h>

#define PIN            0    // Pin to connect the LED strip to
#define NUMPIXELS      12   // Number of pixels in the strip
// Create NeoPixel object
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//int delayval = 500; // delay for half a second

char ssid[] = "INSERT_NETWORK_NAME"; //  your network SSID (name)
char pass[] = "INSERT_NETWORK_PASSWORD"; // your network password

int status = WL_IDLE_STATUS;
// Initialize the Wifi client library
WiFiClient client;

// IFTTT parameters:
char IFTTT_Server[] = "maker.ifttt.com";
//   Key -- Obtained when setting up/connecting the Webooks service in IFTTT //see--->https://ifttt.com/maker_webhooks
char IFTTT_Key[] = "INSERT_KEY";
//   Event -- Arbitrary name for the event; used in the IFTTT Applet.
char IFTTT_Event[] = "INSERT_EVENT_NAME";

// helper functions for constructing the POST data
// append a string or int to a buffer, return the resulting end of string
char *append_str(char *here, char *s) {
    while (*here++ = *s++);
    return here-1;
}
char *append_ul(char *here, unsigned long u) {
    char buf[20];       // we "just know" this is big enough
    return append_str(here, ultoa(u, buf, 10));
}

// set variables to only run the POST request once when triggered
int numRuns=0;
int maxRuns=1;

// set variables to check gas level only every "howLongToWait" seconds
int howLongToWait = 3000;                 // Wait this many millis()
int lastTimeItHappened = 0;              // The clock time in millis()
int howLongItsBeen;                         // A calculated value

void setup() {
    Serial.begin(9600);
//    delay(4000);

    // Start NeoPixel and set all pixels to off
    pixels.begin();
    pixels.clear();
    pixels.show();

    // attempt to connect to Wifi network
    while ( status != WL_CONNECTED) {
      Serial.println("connecting...");
      // Connect to WPA/WPA2 Wi-Fi network
      status = WiFi.begin(ssid, pass);
  
      // wait 10 seconds for connection
//      delay(10000);
  
      // NeoPixels animation for connection attempt feedback
      for(int k=0; k<10; k++){
        for(int i=0;i<NUMPIXELS;i++){
          pixels.setPixelColor(i, pixels.Color(255,255,0));
          pixels.show();
      }
      delay(500);
      
      for(int i=0;i<NUMPIXELS;i++){
        pixels.clear();
        pixels.show();
      }
      delay(500);
    }
  }
  
  Serial.println("connected!");

  // NeoPixels animation for connection success feedback
  for(int k=0; k<3; k++){
    for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,150));
      pixels.show();
    }
    delay(150);
    
    for(int i=0;i<NUMPIXELS;i++){
      pixels.clear();
      pixels.show();
    }
    delay(150);
  }

  delay(1000);
}

void loop() {
  // update counter
  howLongItsBeen = millis() - lastTimeItHappened;

  // only check if waited for "howLongToWait"
  if ( howLongItsBeen >= howLongToWait ) {

    // check sensor and do the math to output GAS value
    float sensor_volt;
    float RS_gas; // Get value of RS in a GAS
    float ratio; // Get ratio RS_GAS/RS_air
    int sensorValue = analogRead(A4);
    sensor_volt=(float)sensorValue/1024*5.0;
    RS_gas = (5.0-sensor_volt)/sensor_volt; // omit *RL

          /*-Replace the name "R0" with the value of R0 in the demo of First Test -*/
    ratio = RS_gas/0.78;  // ratio = RS/R0
          /*-----------------------------------------------------------------------*/

    Serial.print("sensor_volt = ");
    Serial.println(sensor_volt);
    Serial.print("RS_ratio = ");
    Serial.println(RS_gas);
    Serial.print("Rs/R0 = ");
    Serial.println(ratio);

    Serial.print("\n\n");

    // if GAS value below a certain range show NeoPixel feedback and send http request
    if(ratio < 2){
      for(int i=0;i<NUMPIXELS;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
//        delay(delayval); // Delay for a period of time (in milliseconds).
      }
      delay(100);
      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(0,150,0));
        pixels.show();
      }
      delay(4000);

      for(int i=0;i<NUMPIXELS;i++){
        pixels.setPixelColor(i, pixels.Color(150,0,0)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
      
      // start http request to IFTTT
      httpRequest();         
    } else {
      for(int i=0;i<NUMPIXELS;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(0,150,0)); // Moderately bright green color.
        pixels.show(); // This sends the updated pixel color to the hardware.
//        delay(delayval); // Delay for a period of time (in milliseconds).
      }
    }
    
    // update counter
    lastTimeItHappened = millis();
  }
}

void httpRequest() {
  // stop any client activity
  client.stop();

  // if connect to server successful
  if (client.connect(IFTTT_Server, 80))
    {
      Serial.println("Connected to IFTTT");  
      
      // construct the POST request
      char post_rqst[256];    // hand-calculated to be big enough
    
      char *p = post_rqst;
      p = append_str(p, "POST /trigger/");
      p = append_str(p, IFTTT_Event);
      p = append_str(p, "/with/key/");
      p = append_str(p, IFTTT_Key);
      p = append_str(p, " HTTP/1.1\r\n");
      p = append_str(p, "Host: maker.ifttt.com\r\n");
      p = append_str(p, "Content-Type: application/json\r\n");
      p = append_str(p, "Content-Length: ");
    
      // we need to remember where the content length will go, which is:
      char *content_length_here = p;
    
      // it's always two digits, so reserve space for them (the NN)
      p = append_str(p, "NN\r\n");
    
      // end of headers
      p = append_str(p, "\r\n");
    
      // construct the JSON; remember where we started so we will know len
      char *json_start = p;

      // JSON body
//        p = append_str(p, "{\"value1\":\"");
//        p = append_ul(p, 1);
//        p = append_str(p, "\",\"value2\":\"");
//        p = append_ul(p, 2);
//        p = append_str(p, "\",\"value3\":\"");
//        p = append_str(p, "hello");
//        p = append_str(p, "\"}");
    
      // go back and fill in the JSON length
      // we just know this is at most 2 digits (and need to fill in both)
      int i = strlen(json_start);
      content_length_here[0] = '0' + (i/10);
      content_length_here[1] = '0' + (i%10);
      
      // finally we are ready to send the POST to the server!
      if(numRuns < maxRuns){
        client.print(post_rqst);
        Serial.print("Posting request: ");
        Serial.println(post_rqst);
        maxRuns++;
        delay(2000);
      }
      client.stop();    
    } else {
      Serial.println("Can't connect to IFTTT");
    } 
}
