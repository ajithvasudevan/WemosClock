#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 5.5 * 3600, 60 * 60000);
NTPClient timeClient(ntpUDP, "time.cloudflare.com", 5.5 * 3600, 60 * 60000);     // Use any accessible internet time-server

bool updated = false;
int sc = 0, hr = 12, mt = 34;
int baseDelay = 2000;


const char *clockName = "AjithsClock";        // Set the name of the WiFiManager SSID as well 
                                              // as the Title displayed in the WiFiManager Web UI

const int numeral[10] = {
   //ABCDEFG /dp
   B11111100, // 0
   B01100000, // 1
   B11011010, // 2
   B11110010, // 3
   B01100110, // 4
   B10110110, // 5
   B10111110, // 6
   B11100000, // 7
   B11111110, // 8
   B11110110, // 9
};
const int Tx = 1;   // 10 Min
const int Rx = 3;   // 1  Hr

                         // dp G  F  E  D  C  B  A     
const int segmentPins[] = { D7,D7,D3,D2,D1,D6,D5,D4 };
const int nbrDigits= 4; // the number of digits in the LED display

                                // 1m  10m 1h  10h
const int digitPins[nbrDigits] = { D0, Tx, Rx, D8 };


bool shouldSaveConfig = false;

void saveConfigCallback () {
  shouldSaveConfig = true;
}


void setup() {
  WiFiManager wm;
  wm.setShowInfoErase(true);
  wm.setTitle("Setup WiFi Connection");
  wm.setHostname(clockName);
  wm.setWiFiAutoReconnect(true); 
  //wm.resetSettings();
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setConfigPortalTimeout(180);
  wm.setConnectTimeout(10);
  wm.setConnectRetries(2);
  wm.setSaveConnectTimeout(10);

  if (!wm.autoConnect(clockName, "")) {    // can set a password here for WiFiManager AP
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  pinMode(Tx, FUNCTION_3); // Setting TX to GPIO
  pinMode(Rx, FUNCTION_3); // Setting RX to GPIO


  timeClient.begin();
  delay(1000);

  while ( !updated ) {
    updated = timeClient.forceUpdate();
    if(!updated ) delay ( 2000 );
  }
  updated = false;  
  
  for(int i=0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT); 
  }
  for(int i=0; i < nbrDigits; i++) {
    pinMode(digitPins[i], OUTPUT);
  }

}


void loop() {
  hr = timeClient.getHours();
  mt = timeClient.getMinutes();
  sc = timeClient.getSeconds();
  
  if ((hr == 3) && mt == 0 && !updated) {
    timeClient.update();
    updated = true;
  }
  if(mt > 0) updated = false;

  if(hr > 12) {
    hr = hr - 12;
  }
  if(hr == 0) hr = 12;
  
  int value = hr * 100 + mt;
   
  showNumber(value);
//  delay(3);
} 
 
void showNumber( int number)
{
  // display the value corresponding to each digit
  // leftmost digit is 0, rightmost is one less than the number of places
  for( int digit = 0; digit <nbrDigits; digit++)  {

      showDigit( number % 10, digit) ;
      number = number / 10;

  }
} 
 
// Displays given number on a 7-segment display at the given digit position
void showDigit( int number, int digit)
{
  digitalWrite( digitPins[digit], LOW );
  if(digit == 3) {

    for(int segment = 1; segment < 8; segment++)  {
      if (segment == 2) {
      
        if(millis() % 1000 < 500) digitalWrite( segmentPins[segment], 1);
        else digitalWrite( segmentPins[segment], 0);
      }
      else digitalWrite( segmentPins[segment], 0);
    }
    
    if(number == 0) {
      digitalWrite( digitPins[digit], HIGH );
      delayMicroseconds(baseDelay); 
      digitalWrite( digitPins[digit], LOW );
      return;
    }
  }

  
  for(int segment = 1; segment < 8; segment++)  {
    boolean isBitSet = bitRead(numeral[number], segment);
    if(!(digit == 3 && segment == 2)) digitalWrite( segmentPins[segment], isBitSet);
  }
   
  digitalWrite( digitPins[digit], HIGH );
  delayMicroseconds(baseDelay); 
  digitalWrite( digitPins[digit], LOW );
}
