#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESP8266WiFi.h>

#define YOUR_WIFI_SSID "<YOUR_SSID>"
#define YOUR_WIFI_PASSWD "<YOUR_PASSWORD>"

#define NTP_TIMEOUT 1500

int8_t timeZone = 5;
int8_t minutesTimeZone = 30;
const PROGMEM char *ntpServer = "asia.pool.ntp.org";
bool wifiFirstConnected = false;

String timeString;
bool updated = true;
int sc = 0, hr = 12, mt = 34, cln = 0;

// For GREEN 7-Segment Displays
//int baseDelay = 1000;
//const int OFF_FACTOR = 2;

// For BLUE 7-Segment Displays
int baseDelay = 1000;
const int OFF_FACTOR = 0;

// For RED 7-Segment Displays
//int baseDelay = 1000;
//const int OFF_FACTOR = 1;


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



void onSTAConnected (WiFiEventStationModeConnected ipInfo) {
    //Serial.printf ("Connected to %s\r\n", ipInfo.ssid.c_str ());
}


// Start NTP only after IP network is connected
void onSTAGotIP (WiFiEventStationModeGotIP ipInfo) {
    //Serial.printf ("Got IP: %s\r\n", ipInfo.ip.toString ().c_str ());
    //Serial.printf ("Connected: %s\r\n", WiFi.status () == WL_CONNECTED ? "yes" : "no");
    //digitalWrite (ONBOARDLED, LOW); // Turn on LED
    wifiFirstConnected = true;
}

// Manage network disconnection
void onSTADisconnected (WiFiEventStationModeDisconnected event_info) {
    //Serial.printf ("Disconnected from SSID: %s\n", event_info.ssid.c_str ());
    //Serial.printf ("Reason: %d\n", event_info.reason);
    //digitalWrite (ONBOARDLED, HIGH); // Turn off LED
    //NTP.stop(); // NTP sync can be disabled to avoid sync errors
    WiFi.reconnect ();
}

void processSyncEvent (NTPSyncEvent_t ntpEvent) {
    if (ntpEvent < 0) {
        Serial.printf ("Time Sync error: %d\n", ntpEvent);
        if (ntpEvent == noResponse)
            Serial.println ("NTP server not reachable");
        else if (ntpEvent == invalidAddress)
            Serial.println ("Invalid NTP server address");
        else if (ntpEvent == errorSending)
            Serial.println ("Error sending request");
        else if (ntpEvent == responseError)
            Serial.println ("NTP response error");
    } else {
        if (ntpEvent == timeSyncd) {
            Serial.print ("Got NTP time: ");
            Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
        }
    }
}

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

void setup () {
    pinMode(Tx, FUNCTION_3); // TX to GPIO
    pinMode(Rx, FUNCTION_3); // RX to GPIO
    for(int i=0; i < 8; i++) {
      pinMode(segmentPins[i], OUTPUT); 
    }
    for(int i=0; i < nbrDigits; i++) {
      pinMode(digitPins[i], OUTPUT);
    }

  
    static WiFiEventHandler e1, e2, e3;

    //Serial.begin (115200);
    //Serial.println ();
    WiFi.mode (WIFI_STA);
    WiFi.begin (YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

    //pinMode (ONBOARDLED, OUTPUT); // Onboard LED
    //digitalWrite (ONBOARDLED, HIGH); // Switch off LED

    NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
    });

    // Deprecated
    /*WiFi.onEvent([](WiFiEvent_t e) {
        Serial.printf("Event wifi -----> %d\n", e);
    });*/

    e1 = WiFi.onStationModeGotIP (onSTAGotIP);// As soon WiFi is connected, start NTP Client
    e2 = WiFi.onStationModeDisconnected (onSTADisconnected);
    e3 = WiFi.onStationModeConnected (onSTAConnected);
}

void loop () {
    timeString = NTP.getTimeStr();
    int hr = timeString.substring(0, 2).toInt();
    int mt = timeString.substring(3, 5).toInt();
    int sc = timeString.substring(6, 8).toInt();

    if(mt > 0) updated = false;
  
    if(hr > 12) {
      hr = hr - 12;
    }
    if(hr == 0) hr = 12;
    
    int value = hr * 100 + mt;
     
    showNumber(value);


    if (wifiFirstConnected) {
        wifiFirstConnected = false;
        NTP.setInterval (63);
        NTP.setNTPTimeout (NTP_TIMEOUT);
        NTP.begin (ntpServer, timeZone, false, minutesTimeZone);
    }

//    if (syncEventTriggered) {
//        processSyncEvent (ntpEvent);
//        syncEventTriggered = false;
//    }

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
      if((millis() % 1000)  < 500) cln = 0; else cln = 1;
      if (segment == 2) digitalWrite( segmentPins[segment], cln);
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
  delayMicroseconds(OFF_FACTOR * baseDelay);
}
