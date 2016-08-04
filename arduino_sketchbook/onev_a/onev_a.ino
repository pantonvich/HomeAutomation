
#include <SPI.h>
#include <Ethernet.h>
#include <avr/wdt.h>

//#define FREERAM
#define DEBUG
//#define NO_CLIENTMODE
#define TEST

//#define IP176  //out
//#define VERSION_BUILD 3
#define IP177   //ard
#define VERSION_BUILD 1
//#define IP178
//#define IP179
//#define VERSION_BUILD 0
//#define IP180  //wd

 
#define ALERT_NONE 0
#define ALERT_WATER 1
#define ALERT_BELL 2
#define ALERT_RAIN 3

int AlertType = ALERT_NONE; //0=none, 1=rain, 2=water, 3 = bell
long AlertValue = 0;

#ifdef DEBUG
  #define DEBUG_PRINT(str0,str1)    \
     Serial.print(millis());     \
     Serial.print(": ");    \
     Serial.print(__PRETTY_FUNCTION__); \
     Serial.print(' ');      \
     Serial.print(__FILE__);     \
     Serial.print(':');      \
     Serial.print(__LINE__);     \
     Serial.print(' ');      \
     Serial.print(str0);      \
     Serial.print(' ');      \
     Serial.println(str1);
#else
    #define DEBUG_PRINT(str0,str1)
#endif

  #define VERSION_MAJOR '_'
  #define VERSION_MINOR 'a'
  
#ifdef IP176
  #ifndef TEST
    #define DEFINED_SEND_INTERVAL 300000
    #define ip4 176
    #define getURL F("GET /outbash.cgi?")
  #endif
  //#define POWER_APIN A0
  #define RAIN_APIN A5
  #define PYRANOMETER_APIN A2
  //#define GAS_APIN A1 
  #define SD_CARD_DPIN 4
  //#define WATER_DPIN 6
  //#define INSIDE_ONEWIRE_DPIN 7
  #define OUTSIDE_ONEWIRE_DPIN 8
  //#define RESET_DPIN 9

#endif

#ifdef IP177
  #ifndef TEST
    #define DEFINED_SEND_INTERVAL 300000   
    //300000;
    #define ip4 177
    #define getURL F("GET /ardbash.cgi?")
  #endif

  #define POWER_APIN A0
  //#define RAIN_APIN A5
  //#define PYRANOMETER_APIN A2
  #define GAS_APIN A1  
  #define SD_CARD_DPIN 4
  #define WATER_DPIN 6
  #define INSIDE_ONEWIRE_DPIN 7
  #define OUTSIDE_ONEWIRE_DPIN 8
  #define RESET_DPIN 9
 
#endif

#ifdef IP179
  #ifndef TEST
    #define DEFINED_SEND_INTERVAL 120000
    #define ip4 179
    #define getURL F("GET /windbash.cgi?")
  #endif
  //#define POWER_APIN A0
  #define RAIN_APIN A5
  #define PYRANOMETER_APIN A2
  //#define GAS_APIN A1  
  #define SD_CARD_DPIN 4
  //#define WATER_DPIN 6
  //#define lightning_pin 7
  #define INSIDE_ONEWIRE_DPIN 7
  //#define OUTSIDE_ONEWIRE_DPIN 8

  #define DHT_BASEMENT_PIN 2
  #define DHT_BASEMENT_PIN1 3
  //#define DHT_BASEMENT_PIN2 6
  
  #define BELL_PIN 5
  #define WD_PIN 8
  #define WS_PIN 9
  #define RESET_DPIN 14  //A0
  
  #define WS_DEBOUNCE 2
  #define BELL_DEBOUNCE 500
  
  #include "DHT.h"
  //#define DHTTYPE DHT11   // DHT 11 
  #define DHTTYPE DHT22   // DHT 22  (AM2302)
  //#define DHTTYPE DHT21   // DHT 21 (AM2301) 
 #ifdef DHT_BASEMENT_PIN 
  DHT dht(DHT_BASEMENT_PIN, DHTTYPE);
 #endif
 #ifdef DHT_BASEMENT_PIN1 
  DHT dht1(DHT_BASEMENT_PIN1, DHTTYPE);
 #endif
 #ifdef DHT_BASEMENT_PIN2 
  DHT dht2(DHT_BASEMENT_PIN2, DHTTYPE);
 #endif 

  #define GUST_INTERVAL 10000 
#endif

#ifdef TEST
  //#define  ip4  15
  //#define getURL F("GET /tstbash.cgi?")
  #define getURL "GET /testbash.cgi?" 
  #define ip4 180
  #define DEFINED_SEND_INTERVAL 10000
#endif


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, ip4};
byte ip[] = { 192, 168, 0, ip4};
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[]  = { 255, 255, 255, 0 };
byte rip[4];

#if defined(INSIDE_ONEWIRE_DPIN) || defined(OUTSIDE_ONEWIRE_DPIN)
  unsigned long ONEWIRE_DELAY = 10;
#endif
unsigned long RESET_INTERVAL = 300000;
unsigned long SEND_INTERVAL = DEFINED_SEND_INTERVAL;

unsigned long currentMillis = 0;
unsigned long lastConnectMillis=SEND_INTERVAL;
unsigned long ClientMillis=0;
unsigned long resetMillis=0;

unsigned long ctLoop = 0;

#ifdef RESET_DPIN 
  unsigned int resetSendCt = 0; //reboot hack
#endif

#define STRING_BUFFER_SIZE 325
char outbuffer[STRING_BUFFER_SIZE+2];
unsigned long outbufferIdx = 0;

#define STRING_GetBuffer_SIZE 20
char GetBuffer[STRING_GetBuffer_SIZE+2];
unsigned int GetBufferIdx=0;
//int lastday = 0;

EthernetServer server(80);
EthernetClient client;

void setup()
{
  #ifdef RESET_DPIN 
    digitalWrite(RESET_DPIN, HIGH); //this is a hack!
    wdt_disable();
    pinMode(RESET_DPIN, OUTPUT);
  #endif
  
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
#ifdef DEBUG
  Serial.begin(115200);
  DEBUG_PRINT("str","1");
#endif

  
  analogReference(DEFAULT);

#ifdef GAS_APIN
  pinMode(GAS_APIN, INPUT);
#endif

#ifdef POWER_APIN
  pinMode(POWER_APIN, INPUT);
#endif

#ifdef PYRANOMETER_APIN
  pinMode(PYRANOMETER_APIN, INPUT);
#endif

#ifdef RAIN_APIN
  pinMode(RAIN_APIN, INPUT);
#endif

#ifdef WATER_DPIN
  //pinMode(WATER_DPIN, INPUT); 
  pinMode(WATER_DPIN, INPUT_PULLUP); 
  //waterLastRead = digitalRead(WATER_DPIN);
  //for(int x = 0;x<5;x++) water5[x] = 0;
  //digitalWrite(WATER_DPIN,HIGH);
#endif

#ifdef SD_CARD_DPIN 
  //disable SD card
  pinMode(SD_CARD_DPIN, OUTPUT);
  digitalWrite(SD_CARD_DPIN, HIGH);
#endif

#ifdef BELL_PIN
  pinMode(BELL_PIN, INPUT);
  digitalWrite(BELL_PIN,HIGH);
#endif

#ifdef WS_PIN
  pinMode(WS_PIN, INPUT);
  pinMode(WD_PIN, INPUT);
  digitalWrite(WS_PIN,HIGH);
  digitalWrite(WD_PIN,HIGH);
#endif

#ifdef RESET_DPIN 
  //wdt_enable(WDTO_8S);
  // clear various "reset" flags
 MCUSR = 0;
 // allow changes, disable reset
 WDTCSR = bit (WDCE) | bit (WDE);
 // set interrupt mode and an interval 
 WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
#endif

 outbuffer[outbufferIdx++] = '*';
}

void runLoops() 
{
#ifdef RESET_DPIN
  wdt_reset();
#endif

  currentMillis = millis();
  
#ifdef POWER_APIN
  PowerLoop();
#endif
 
#ifdef GAS_APIN
  GasLoop();
#endif
 
#ifdef WATER_DPIN 
  WaterLoop();
#endif
 
#ifdef PYRANOMETER_APIN
  PyranometerRead();
#endif
 
#ifdef RAIN_APIN
   RainRead();
#endif

#ifdef BELL_PIN
  BellLoop();
#endif

#ifdef WS_PIN
  WindLoop(); 
#endif

  ctLoop++;
}

void runDelayLoops(int milliseconds)
{
  static unsigned long holdMillis;
  holdMillis = currentMillis;
  
  while (holdMillis - currentMillis < milliseconds)
  {
    runLoops();
  }
}

void loop()
{

  runLoops();
  ServerMode();
  
   if ((currentMillis - lastConnectMillis > SEND_INTERVAL) )  //||  outbuffer[1] == '*'
   {   
     ClientMode();
     resetAll();
     lastConnectMillis = currentMillis;
     //TimeAdj();
#ifdef WS_PIN
     SetLastDay();
#endif
     //we got a connect so reset or add one to reset count
#ifdef RESET_DPIN  
     if(GetBufferIdx>13) {resetSendCt = 0;} else {resetSendCt++; }
     DEBUG_PRINT("resetSendCt",resetSendCt);
     DEBUG_PRINT("GetBufferIdx",GetBufferIdx);
     //reboot hack
     if (resetSendCt > 3) { digitalWrite(RESET_DPIN, LOW); }
#endif
   } 
    

}
 
#ifdef RESET_DPIN 
   ISR (WDT_vect)
   {
     DEBUG_PRINT("ISR","");
     wdt_disable();
     delay(5000);
     digitalWrite(RESET_DPIN, LOW);
     
   }
#endif

void ServerMode()
{    
  char c;
  int flag = 0;
  unsigned long connectLoop = 0;
 
  client = server.available();
  
  if (client) {    
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    
    GetBufferIdx = 0;
    
    while (client.connected()) {
      unsigned long startTime = millis();

      while ((!client.available()) && ((millis() - startTime ) < 5000)){
        runLoops();  
      };

      if (client.available()) {
        if (connectLoop++ > 10000) break;
        c = client.read();
        if (flag<2 && GetBufferIdx < STRING_GetBuffer_SIZE ) 
        {
          if (flag==1 && c==' ')
            flag=2;
          
          if (flag==1) {           
            GetBuffer[GetBufferIdx++] = c;
          }
          
          if (flag==0 && c == '/') 
            flag = 1;
        }
        
        // if we've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so we can send a reply
        if (c == '\n' && current_line_is_blank) {
         
          client.println(F("HTTP/1.1 200 OK"));
          client.println(F("Content-Type: text/html"));
          client.println(F("Connection: close"));
          client.println();
          client.print(F("<!DOCTYPE html><html><head><link rel=""icon"" type=""image/png"" href=""data:image/png;base64,iVBORw0KGgo=""></head><body>"));

          BuildOutput();
          DEBUG_PRINT(F("outbuffer"), outbuffer);
          client.print(outbuffer);
          client.println(F("</body></html>"));
          client.println();
          break;
        }
        if (c == '\n') {
          // we're starting a new line
          current_line_is_blank = true;
        } else if (c != '\r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }
      if (connectLoop++ > 10000) break;
    }
    DEBUG_PRINT(F("connectLoop"),connectLoop);
    // give the web browser time to receive the data   
    runDelayLoops(2);
    client.flush();
    client.stop();

    GetBuffer[GetBufferIdx]='\0';
    //DEBUG_PRINT(GetBuffer);
  }
}	

#if defined(INSIDE_ONEWIRE_DPIN) || defined(OUTSIDE_ONEWIRE_DPIN)

#include <OneWire.h>	

void GetOneWireTemps(int  pinToRead) {
    int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract, Tf_100;
    OneWire  ds(pinToRead);
    byte i;
    byte present = 0;
    byte data[12];
    byte addr[8];
    int ct = 0;
    
    while (ct != -1) {
      if ( !ds.search(addr)) {
        ds.reset_search();
        return;
      }

      if ( OneWire::crc8( addr, 7) != addr[7]) {
        //DEBUG_PRINT("CRC is not valid!\n");
        return;
      }

  if ( addr[0] == 0x28) { 
    // The DallasTemperature library can do all this work for you!
    ds.reset();
    ds.select(addr);
    ds.write(0x44,0);         // start conversion, with parasite power on at the end
    
    runDelayLoops(ONEWIRE_DELAY);
    //delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
  
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
    }
  
    runDelayLoops(10);
    
    LowByte = data[0];
    HighByte = data[1];
    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if (SignBit) // negative
    {
      TReading = (TReading ^ 0xffff) + 1; // 2's comp
    }
    Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  
    //Tf_100 = (9/5)*Tc_100+32;
    //Whole = ((9 * Tc_100 / 5) + 3200) / 100;  // separate off the whole and fractional portions
    //Fract = ((9 * Tc_100 / 5) + 3200) % 100;
    int W100, W10, W1, F1, F10;
    char temp[6];
    
    itoa(addr[7],temp,16);
    
    bTitle(temp[0],temp[1]);
    
    W100 = Tc_100 / 10000;
    W10 = Tc_100 / 1000 - W100 * 10;
    W1 =  Tc_100 / 100 - W10 * 10 - W100 * 100;
    F1 =  Tc_100 / 10 - W1 *10  - W10 * 100 - W100 * 1000;
    F10 = Tc_100 - F1 * 10 - W1 * 100 - W10 * 1000 - W100 * 10000;
    
    if (SignBit) {
      outbuffer[outbufferIdx++] = '-';
    }
   
   //buffer[outbufferIdx++] = 48+W100;
   outbuffer[outbufferIdx++] = 48+W10;
   outbuffer[outbufferIdx++] = 48+W1;
   outbuffer[outbufferIdx++] = '.';
   outbuffer[outbufferIdx++] = 48+F1;
   outbuffer[outbufferIdx++] = 48+F10;

    }
    }
  }
#endif

#ifdef POWER_APIN

  float powerTripLow = .965; //.900
  float powerTripHigh = 1.008;
  unsigned long powerCount = 0;
  unsigned long powerCountLast = 0;
  unsigned long powerLastBlink = 0;
  unsigned long powerK5count =0;
  unsigned long powerW1=0;
  unsigned long powerW2=0;
  unsigned long powerW3=0;
  unsigned long powerW4=0;
  unsigned long powerW5=0;
  //unsigned long powerW6=0;
  
  float powerBlinkLength = 80;
  float powerBlinkRate = 200;
  int powerBlinkFlag = 1;
  float powerRatio = 1.0;
  float powerFilterH = 600;
  float powerFilterC = 600;
  float powerMaxL = 1;
  float powerMaxH = 0;
  int powerTripLowOffset=8;
  int powerTripHighOffset=8;
  
  int powerReading = 0;
  
  void PowerLoop() {
      
    powerReading = analogRead(POWER_APIN);
    powerFilterH += .01 * (powerReading-powerFilterH);
    powerFilterC += .9 * (powerReading-powerFilterC);
    powerRatio = powerFilterC/powerFilterH;
    
    if (powerRatio < powerMaxL) powerMaxL = powerRatio;
    if (powerRatio > powerMaxH) powerMaxH = powerRatio;
  
    if ((powerRatio < powerTripLow && powerBlinkFlag ==1 && currentMillis-powerLastBlink > 50) )
    {
        powerBlinkFlag = 0;
        powerCount++;
        float rate = currentMillis-powerLastBlink;
        if(rate>0) powerBlinkRate += .05 *(rate-powerBlinkRate);
        powerLastBlink = currentMillis;   
    }
    else if (powerRatio > powerTripHigh && powerBlinkFlag == 0)
    {
      if ( currentMillis-powerLastBlink > 30 ){
        float rate = currentMillis-powerLastBlink;
        if (rate>0) powerBlinkLength +=  .05*(rate-powerBlinkLength);
        powerBlinkFlag = 1;
      }
    }
   
   unsigned long w = currentMillis - lastConnectMillis;
   
   if ( w < 60000) { powerW1 = powerCount; }
   else if (w < 120000) { powerW2 = powerCount; }
   else if (w < 180000) { powerW3 = powerCount; }
   else if (w < 240000) { powerW4 = powerCount; }
   //else if (w < 300000) { powerW5 = powerCount; }
   else { powerW5 = powerCount;}
  }
  
  void bPrintPower(char c1, char c2, unsigned long p1, unsigned long p0)
  {
     if(p1 == 0 )
      bPrint(c1,c2,0,100);
     else
      bPrint(c1,c2,p1-p0,100);
   }
#endif

#ifdef GAS_APIN
  
  int gasTripOffset = 15; //75;
  unsigned long gasTotalCount = 0;
  unsigned long gasLastCount = 0;
  unsigned long gaslastMillis = 0;
  unsigned long gasReading = 0;
  unsigned int gasFlag = 0;
  unsigned long gasTripHigh = 0;
  unsigned long gasTripLow = 1023;
  
  float gasFilterL = 900;
  unsigned long gasLowLow = 1023;
  unsigned long gasMaxMax = 0;
  
  void GasLoop()
  {
    
    gasReading = analogRead(GAS_APIN); 
    gasFilterL += .01 * (gasReading-gasFilterL);
    
    gasReading = gasFilterL;
      
    if (gasReading>gasTripHigh) {
      if (gasFlag == 1){
        gasFlag =0;
        gasTotalCount++;
      }
      gasTripLow = gasReading - gasTripOffset;
      gasTripHigh = gasReading;
    }
    if (gasReading<gasTripLow) {
      gasFlag = 1;
      gasTripHigh = gasReading + gasTripOffset;
      gasTripLow = gasReading;
    }
    
    if(gasReading<gasLowLow) gasLowLow=gasReading;
    if(gasReading>gasMaxMax) gasMaxMax=gasReading;
  }
#endif

#ifdef WATER_DPIN

unsigned long waterTotal = 0;
int waterLastRead = LOW;
unsigned long waterHitMillis = 0;
unsigned int water5[] = {0,0,0,0,0,0};
unsigned long WATER_DEBOUNCE = 500;

  void WaterLoop()
  {
  
    if (digitalRead(WATER_DPIN) != waterLastRead && (currentMillis - waterHitMillis > WATER_DEBOUNCE) ) {
      DEBUG_PRINT(F("waterhit"),currentMillis - waterHitMillis);
      DEBUG_PRINT(F("water"),!waterLastRead);
      waterHitMillis = currentMillis;	
      AlertType = ALERT_WATER; 
      waterLastRead = !waterLastRead; 
      waterTotal++; //if(waterLastRead == LOW)
      ClientMode();
      //AlertType = ALERT_NONE;  
    }
       
   unsigned long w = currentMillis - lastConnectMillis;
   
   if ( w < 60000) { water5[1] = waterTotal; }
   else if (w < 120000) { water5[2] = waterTotal; }
   else if (w < 180000) { water5[3] = waterTotal; }
   else if (w < 240000) { water5[4] = waterTotal; }
   
   water5[5] = waterTotal; 
  }
#endif

#ifdef PYRANOMETER_APIN

//   pyranometer
//  ------+-----------A
//        |
//       10K
//        |
//  +-----+-----------gnd
//
  #define PYRANOMETER_INTERVAL 1000
  unsigned int PyranometerReading = 0;
  unsigned long LastPyranometerMillis=PYRANOMETER_INTERVAL;;
  float PyranometerFilter = 0.0;
  
  void PyranometerRead(){
    
     if (currentMillis - LastPyranometerMillis > PYRANOMETER_INTERVAL ) 
     { 
       PyranometerReading = analogRead(PYRANOMETER_APIN);
       PyranometerFilter += .01 * ((float)PyranometerReading-PyranometerFilter); 
       LastPyranometerMillis = currentMillis;
     }
  }
#endif

#ifdef RAIN_APIN

//   rain_detector
//        +-----------A
//        |
//  ||----+----100K---5+
//  +-----------------gnd
//

#define RAIN_INTERVAL 1000
//3hours
#define RAIN_ALERT_INTERVAL 10800000
unsigned long rainLow = 1024;
unsigned long rainHigh = 0;
float rainFilter = 1023.0;
float rainFilterLast = 1023.0;
unsigned long LastRainMillis = 0;
unsigned long LastRainAlertMillis800 = RAIN_ALERT_INTERVAL;
unsigned long LastRainAlertMillis700 = RAIN_ALERT_INTERVAL;
unsigned long LastRainAlertMillis600 = RAIN_ALERT_INTERVAL;
unsigned long LastRainAlertMillis500 = RAIN_ALERT_INTERVAL;
unsigned long LastRainAlertMillis400 = RAIN_ALERT_INTERVAL;
unsigned int rainRead;

void RainRead() {
     if (currentMillis - LastRainMillis > RAIN_INTERVAL ) 
     { 
        LastRainMillis = currentMillis;
        
        rainRead = analogRead(RAIN_APIN);
        if (rainRead < rainLow) rainLow = rainRead;
        if (rainRead > rainHigh) rainHigh = rainRead;
        rainFilter += .01 * ((float)rainRead-rainFilter); 
        
        if (rainFilter < 800 && (currentMillis - LastRainAlertMillis800 > RAIN_ALERT_INTERVAL)) {
          AlertType = ALERT_RAIN;
          LastRainAlertMillis800=currentMillis;
        }
        if (rainFilter < 700 && (currentMillis - LastRainAlertMillis700 > RAIN_ALERT_INTERVAL)) {
          AlertType = ALERT_RAIN;
          LastRainAlertMillis700=currentMillis;
        }
        if (rainFilter < 600 && (currentMillis - LastRainAlertMillis600 > RAIN_ALERT_INTERVAL)) {
          AlertType = ALERT_RAIN;
          LastRainAlertMillis600=currentMillis;
        }
        if (rainFilter < 500 && (currentMillis - LastRainAlertMillis500 > RAIN_ALERT_INTERVAL)) {
          AlertType = ALERT_RAIN;
          LastRainAlertMillis500=currentMillis;
        }
        if (rainFilter < 400 && (currentMillis - LastRainAlertMillis400 > RAIN_ALERT_INTERVAL)) {
          AlertType = ALERT_RAIN;
          LastRainAlertMillis400=currentMillis;
        }
        
        if (AlertType != ALERT_NONE ) {
          DEBUG_PRINT("ClientMode","AlertType");
          ClientMode();
          //Reset AlertType = ALERT_NONE;
          ClientMode();
        }  
    }
}
#endif

#ifdef BELL_PIN

int bellLastRead = LOW;
unsigned long bellHitMillis=0;
unsigned long bellTotal = 0;
unsigned int bell2Min = 0;

void BellLoop()
{
  
  int bellRead =digitalRead(BELL_PIN);
  if (bellRead == HIGH && bellLastRead==LOW && (currentMillis - bellHitMillis > BELL_DEBOUNCE)) 
  { 
    bellTotal++;
    bell2Min++;
    bellHitMillis=currentMillis;

    AlertType = ALERT_BELL;
    ClientMode();
  }

  bellLastRead=bellRead;   
}
#endif

#ifdef WS_PIN
int wsLastRead = LOW;
unsigned long wsCt = 0;
unsigned long wsSendCt = 0;
unsigned long wsInterval=0;
unsigned long wsTime;
unsigned long wsHitMillis = 0;

unsigned long wgMaxSendCt = 0;
unsigned long wgNextIntervalMillis = 0;
unsigned long wgMaxTodayCt=0;
unsigned long wgMaxYesterdayCt = 0; //yesterdays max wind

int wdLastRead=LOW;
unsigned long wdInterval;
int wdFlag=LOW;

float wd;

void WindLoop()
{  
  int wsRead=digitalRead(WS_PIN);
  int wdRead=digitalRead(WD_PIN);
  if (wsRead==HIGH && wsLastRead==LOW && (currentMillis - wsHitMillis > WS_DEBOUNCE)) {   
    wsHitMillis=currentMillis;  	 
    wsPulse();
    wsCt++;
    wsSendCt++;
  }
  
  if (wdRead==HIGH && wdLastRead==LOW && wdFlag==HIGH ) {	 
    wdPulse();
  }

  //check 10s gust
  //filter 2min wind
  if (currentMillis - wgNextIntervalMillis > GUST_INTERVAL)
  {
    if (wgMaxTodayCt<wsCt) wgMaxTodayCt=wsCt;
    //ws2 = ws2 * .91667 + (float)wsCt;
   //ws2 += .15 * (wsCt - ws2);
    if (wsCt > wgMaxSendCt) wgMaxSendCt = wsCt;
    wsCt=0;
    //DEBUG_PRINT(bellTotal);
    //DEBUG_PRINT(" ");
//DEBUG_PRINTLN(bellLastRead);
    wgNextIntervalMillis=currentMillis;
  }  
  wsLastRead=wsRead;
  wdLastRead=wdRead;
}

//Use Pulse_A_Interval to calculate wind speed. Use Pulse_B_Interval*360.0/Pulse_A_Interval to get direction of wind.
//http://code.ohloh.net/file?fid=plfSM3O7RAnLnqzfqC-IfeGTw3g&cid=LIfw5qQ_Ip0&s=&fp=490510&mp=&projSelected=true#L0
// * In ULTIMETER Weather Stations, speed is determined by measuring the time interval between
// * two successive closures of the speed reed. Calibration is done as follows (RPS = revolutions
// * per second):
// * 0.010 < RPS < 3.229 (approximately 0.2 < MPH < 8.2):
// * windSpeedDur<309693
// * MPH = -0.1095(RPS2) + 2.9318(RPS)  0.1412
// * KNTS = -0.09515(RPS2) + 2.5476(RPS)  0.1226
// *
// * 3.230 < RPS < 54.362 (approximately 8.2 < MPH < 136.0):
// * windSpeedDur < 18395
/// * MPH = 0.0052(RPS2) + 2.1980(RPS) + 1.1091
// * KNTS = 0.0045(RPS2) + 1.9099(RPS) + 0.9638
// *
// * 54.363 < RPS < 66.332 (approximately 136.0 < MPH < 181.5):
// *
// * MPH = 0.1104(RPS2) - 9.5685(RPS) + 329.87
// * KNTS = 0.09593(RPS2) - 8.3147(RPS) + 286.65
// *
// * Conversions used are: mph * 0.86897 = knots; mph * 1.6094 = kmph; mph * 0.48037 = m/s

int getMPH(float r, float s)
{ 
  if (s<=0 || r <= 0 || r == 4294967295) return 0;
  float rps = r/s;
  
  if (rps<.05) return 0;
  if (rps< 3.230) return rps * rps * -11 + rps * 293 - 14;  
//0.0052(RPS2) + 2.1980(RPS) + 1.1091  
  if (rps< 54.362) return rps * rps * .52 + rps * 220 + 111;
// * MPH = 0.1104(RPS2) - 9.5685(RPS) + 329.87
  return rps * rps * 11 - rps * 957 + 32987;
}

void wsPulse() {
   unsigned long currentTime = micros();
   wsInterval = currentTime - wsTime;
   wsTime = currentTime;  
   
   //if (wsInterval>wgMax) wgMax=wsInterval;
   wdFlag=HIGH;
}

void wdPulse() {
   unsigned long currentTime = micros();
   wdInterval = currentTime - wsTime;
   if (wsInterval >= wdInterval)
   {
     wd = wdInterval*360.0/wsInterval;
     wdFlag=LOW;
   }
}

#endif

#ifdef DHTTYPE
  void  PrintHum() {      
    float humidity;
    float temperature;

    #ifdef DHT_BASEMENT_PIN
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();    
      bPrint('0','H',humidity*10,100);      
      bPrint('0','T',temperature*10,100);    
      humidity = dht1.readHumidity();
    #endif
    
    #ifdef DHT_BASEMENT_PIN1
      humidity = dht1.readHumidity();
      temperature = dht1.readTemperature();
      bPrint('1','H',humidity*10,100);     
      bPrint('1','T',temperature*10,100);
    #endif
    
    #ifdef DHT_BASEMENT_PIN2
      humidity = dht2.readHumidity();    
      temperature = dht2.readTemperature();
      bPrint('2','H',humidity*10,100);     
      bPrint('2','T',temperature*10,100);
    #endif
   }
#endif

void BuildOutput() {
 DEBUG_PRINT(F("BuildOutput"),"");
 
  if (AlertType==ALERT_WATER) {
#ifdef WATER_DPIN
    outbuffer[outbufferIdx++] = 'W';
    outbuffer[outbufferIdx++] = 'A';
    outbuffer[outbufferIdx++] = 'T';
    outbuffer[outbufferIdx++] = 'E';
    outbuffer[outbufferIdx++] = 'R';
    outbuffer[outbufferIdx++] = '0'+waterLastRead;

#endif
  } else if (AlertType==ALERT_RAIN) {
#ifdef RAIN_APIN
    outbuffer[outbufferIdx++] = 'R';
    outbuffer[outbufferIdx++] = 'A';
    outbuffer[outbufferIdx++] = 'I';
    outbuffer[outbufferIdx++] = 'N';
    bPrint('R','f',rainFilter,100);

#endif   
  } else if (AlertType==ALERT_BELL) {
#ifdef BELL_PIN
    outbuffer[outbufferIdx++] = 'B';
    outbuffer[outbufferIdx++] = 'E';
    outbuffer[outbufferIdx++] = 'L';
    outbuffer[outbufferIdx++] = 'L';

#endif    
  }else {
#ifdef INSIDE_ONEWIRE_DPIN
    GetOneWireTemps(INSIDE_ONEWIRE_DPIN);
#endif
#ifdef OUTSIDE_ONEWIRE_DPIN
    GetOneWireTemps(OUTSIDE_ONEWIRE_DPIN);
#endif
     
    Print();
  }
  AlertType=ALERT_NONE;
  outbuffer[outbufferIdx] = '\0';
  outbufferIdx=0;
}

void Print() {
 DEBUG_PRINT(F("Print"),"");
#ifdef POWER_APIN  
  bPrint('K','0',powerCount,10000);
  bPrintPower('w','1', powerW1, powerCountLast);
  bPrintPower('w','2', powerW2, powerW1);
  bPrintPower('w','3', powerW3, powerW2);
  bPrintPower('w','4', powerW4, powerW3);
  bPrintPower('w','5', powerW5, powerW4); 
  bPrint('K','5',powerCount - powerCountLast ,1000);
  DEBUG_PRINT(F("K0"),"");
#endif
  
#ifdef WATER_DPIN
   bPrint('H','0',waterTotal,100);
   DEBUG_PRINT(F("H0"),waterTotal);
   bPrint('H','5',waterTotal-water5[0],1);
   DEBUG_PRINT(F("Hw0"),water5[0]);
   DEBUG_PRINT(F("Hw1"),water5[1]);
   DEBUG_PRINT(F("Hw2"),water5[2]);
   DEBUG_PRINT(F("Hw3"),water5[3]);
   DEBUG_PRINT(F("Hw4"),water5[4]);
   DEBUG_PRINT(F("Hw5"),water5[5]);
   
   for(int x = 1;x<6;x++) {
    int t = 0;
    DEBUG_PRINT(F("Hw"),water5[x]);
    if (water5[x] <= water5[5]) t=water5[x]-water5[x-1];
    DEBUG_PRINT(F("t"),t);
    if (t<0) t=0;
    bPrint('h','0' + x,t,1);
   }
   bPrint('H','t',WATER_DEBOUNCE,100);
   DEBUG_PRINT(F("H0"),"");
#endif

#ifdef PYRANOMETER_APIN  
   bPrint('P','r',PyranometerReading,100);
   bPrint('P','f',PyranometerFilter*10,100);
#endif

#ifdef RAIN_APIN
   bPrint('R','r',rainRead,1000);
   bPrint('R','f',rainFilter,1000);
   bPrint('R','L',rainLow,1000);
   bPrint('R','H',rainHigh,1000);
#endif

#ifdef POWER_APIN
     //bPrint('f','H',powerFilterH,1000);
     //bPrint('f','C',powerFilterC,1000);
    bPrint('B','L',powerBlinkLength,1000);
    bPrint('B','R',powerBlinkRate,1000);
    bPrint('M','H',powerMaxH*1000,1000);
    bPrint('M','h',powerTripHigh*1000,1000);
    bPrint('M','h',powerTripHighOffset,10);
    bPrint('M','L',powerMaxL*1000,100);
    bPrint('M','l',powerTripLow*1000,100);
    bPrint('M','l',powerTripLowOffset,10);   
    DEBUG_PRINT(F("BL"),"");
#endif

#ifdef GAS_APIN
    bPrint('G','0',gasTotalCount,1000);
    bPrint('G','5', gasTotalCount - gasLastCount,1);
    //bPrint('G','o', gasMaxMax - gasLowLow,10);
    bPrint('G','r',gasReading,1000);
    bPrint('G','f',gasFilterL,1000);
    bPrint('G','l',gasLowLow,1000);
    bPrint('G','h',gasMaxMax,1000);
    bPrint('G','d', gasMaxMax - gasLowLow,10);
    bPrint('G','t',gasTripOffset,10);
    DEBUG_PRINT(F("G0"),"");
#endif

#ifdef WS_PIN
    bPrint('W','D',wd,100);
    bPrint('W','S',getMPH(wsSendCt,SEND_INTERVAL/1000),1000);  
    bPrint('W','G',getMPH(wgMaxSendCt,GUST_INTERVAL/1000),1000);
    bPrint('w','2',wsSendCt,100);    
    bPrint('g','m',wgMaxTodayCt,100);   
    bPrint('g','y',wgMaxYesterdayCt,100);
    DEBUG_PRINT(F("WD"),"");
#endif

#ifdef DHTTYPE
  PrintHum();
  DEBUG_PRINT(F("Hum"),"");
#endif

#ifdef BELL_PIN
    bPrint('B','0',bellTotal,1);   
    bPrint('B','2',bell2Min,1);
#endif

    bPrint('C','l',ctLoop,1000);
#ifdef FREERAM
    bPrint('F','r',freeRam(),10000);
    DEBUG_PRINT(F("Fr"),"");
#endif

    //bPrint(' ','v',ip[3],100);
    
   bPrint(VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,1); 
   bPrint('m','e',ip[3],100); 
   DEBUG_PRINT(F("me"),"");
//bPrint('v','6',ONEWIRE_DELAY,100);

 }

void resetAll()
{
  DEBUG_PRINT(F("resetAll"),ctLoop);
#ifdef WATER_DPIN
  water5[0]=waterTotal;
#endif
#ifdef POWER_APIN
  powerCountLast = powerCount;
  DEBUG_PRINT(("MTh"),powerTripHigh*1000);
  DEBUG_PRINT(("MMh"),powerMaxH*1000);
  DEBUG_PRINT(("MOh"),powerTripHighOffset);
  
  powerTripHigh = powerMaxH - powerTripHighOffset/1000.0;
  
  DEBUG_PRINT(("Mh"),powerTripHigh*1000);
  DEBUG_PRINT(("Ml"),powerTripLow*1000);
  
  powerTripLow = powerMaxL + powerTripLowOffset/1000.0;
  
  DEBUG_PRINT(("Ml"),powerTripLow*1000);
  
  powerMaxH = 0;
  powerMaxL = 1;
  powerW1=0;powerW2=0;powerW3=0;powerW4=0;powerW5=0;
  
#endif
#ifdef GAS_APIN
  gasLastCount = gasTotalCount; 
  gasLowLow = 1023;
  gasMaxMax = 0; 
#endif
#ifdef RAIN_APIN
  rainLow=1024;
  rainHigh=0;
  rainFilterLast = rainFilter;
#endif
#ifdef BELL_PIN
    bell2Min=0;
#endif
#ifdef WS_PIN
    wsSendCt=0;
    wgMaxSendCt = 0;
#endif
ctLoop=0;
}
	
void bTitle(char c1, char c2) {
  
  if ((outbufferIdx + 4) > STRING_BUFFER_SIZE) return;
  
   outbuffer[outbufferIdx++] = '[';
   outbuffer[outbufferIdx++] = c1;
   outbuffer[outbufferIdx++] = c2;
   outbuffer[outbufferIdx++] = ']';
}
	
void bPrint(char c1, char c2, unsigned long reading,unsigned long i){ 
  
   bTitle(c1,c2);
  //if ((outbufferIdx + i) > STRING_BUFFER_SIZE) return;
   BufferPrint(reading,i); 
}

void BufferPrint(unsigned long reading,unsigned long i) {
  unsigned long temp; 
  unsigned int iCount = 0;

  if(reading<0) {
    outbuffer[outbufferIdx++] = '-';
    reading *= -1;
  }
  
  while (i*10 <= reading){
   i*=10;
   iCount++; 
  }
  
  if ((outbufferIdx + iCount) > STRING_BUFFER_SIZE) return;
  
  while (i>0){

     temp = reading/i;
     reading -= temp * i;
     outbuffer[outbufferIdx++] = 48+temp;
     i/=10;
    }
}  
	
//void buf_add_char(unsigned char *str,unsigned int strsz)
//{
//  if ((outbufferIdx + strsz) > STRING_BUFFER_SIZE) return;
  
//  unsigned int  i;
//  for (i = 0; i < strsz; i++)
//  {
//    outbuffer[outbufferIdx++] =str[i];
//  }
 
//}

#ifdef FREERAM
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif
//void serialPrint()
//{
 //if(false){
 // if(outbufferIdx<(OUT_SIZE-10)){
 //   if(powerReading>999){
 //     OutBufferPrint(powerReading,1000);
 //   }
 //   else if(powerReading>99){
 //     OutBufferPrint(powerReading,100);
 //   }
 //   else{ OutBufferPrint(powerReading,10);}
 // }}
 //return;
   //DEBUG_PRINT(rArg);
    //DEBUG_PRINT("");
    //DEBUG_PRINT(counter);
    //DEBUG_PRINT(" ");
    //DEBUG_PRINT(powerReading);
   // DEBUG_PRINT(" ");
    //DEBUG_PRINT(powerFilterC/powerFilterH);
    //DEBUG_PRINT(" ");
    //DEBUG_PRINT(topLow);
    //DEBUG_PRINT(" ");
    //DEBUG_PRINT(powerBlinkFlag);
    //DEBUG_PRINTLN(" "); 
    //return;
//}
void ClientMode()
{
  #ifdef NO_CLIENTMODE
    BuildOutput();
    return;
  #endif
  
  #define INVALUE_MAX 7
  IPAddress nasServerIP(192,168,0,77);
  unsigned long connectLoop = 0;
  unsigned long inValue[INVALUE_MAX];
  int inPos = -1;
  inValue[0] = 0; inValue[1] = 0; inValue[2] = 0; inValue[3] = 0; inValue[4] = 0; inValue[5] = 0; inValue[6] = 0;
  
  if (client.connect(nasServerIP,80)) {
  } else {
    Ethernet.begin(mac, ip, gateway, gateway, subnet); 
    runDelayLoops(100);
    client.connect(nasServerIP,80);
  }
  if (client.connected()) {
    DEBUG_PRINT("getURL",getURL);
    client.print(getURL); //"GET /webtest.cgi?n=123 HTTP/1.0");
    BuildOutput();
    DEBUG_PRINT("outbuffer ",outbuffer);
    client.println(outbuffer); 
    client.print(F(" HTTP/1.0"));
    client.println();
   }
  
  while(client.connected() && !client.available()) {
    runLoops(); //waits for data
    if (connectLoop++ > 10000) break;
  }
    
  GetBufferIdx = 0;
  
  DEBUG_PRINT(F("geting"),F("GetBuffer"));
  while (client.connected()) {
     
     while( client.available()) { //connected or data available
      char c = client.read();
      //GetBuffer[GetBufferIdx++] = c;
      if (c >= '0' && c <= '9' && GetBufferIdx < STRING_GetBuffer_SIZE)
      {
        //just get the date
         if (inPos == -1) GetBuffer[GetBufferIdx++] = c;
         if (inPos > -1 && c != '|' && inPos < INVALUE_MAX ) inValue[inPos] = inValue[inPos] * 10 + (c - '0');       
      }
      else if (c=='|') {inPos++;}
      if (connectLoop++ > 10000) break;
     }
     if (connectLoop++ > 10000) break;
  }
  DEBUG_PRINT(F("connectLoop"),connectLoop);
  client.stop();

  GetBuffer[GetBufferIdx] = '\0';
  DEBUG_PRINT("GetBufferIdx ",GetBufferIdx);
  DEBUG_PRINT("GetBuffer ",GetBuffer);
  DEBUG_PRINT("inPos ",inPos);
  if (inPos > 4) {
#ifdef POWER_APIN
   if(inValue[0] > 0) powerTripLowOffset =  inValue[0]; //(float)inValue[0]/1000.0;
   if(inValue[1] > 0) powerTripHighOffset = inValue[1]; //(float)inValue[1]/1000.0;
#endif
#ifdef GAS_APIN
   if(inValue[2] > 0) gasTripOffset = inValue[2];
#endif
   if(inValue[3] > 0) SEND_INTERVAL = inValue[3];
   if(inValue[4] > 0) RESET_INTERVAL = inValue[4];
#ifdef WATER_DPIN   
   if(inValue[5] > 0) WATER_DEBOUNCE = inValue[5];
#endif
#if defined(INSIDE_ONEWIRE_DPIN) || defined(OUTSIDE_ONEWIRE_DPIN)
   if(inValue[6] > 0) ONEWIRE_DELAY =  inValue[6];
#endif
  }
  
}

void SetLastDay(){
  static int lastDay = 0;
  int day;
  
  if(GetBufferIdx>13) {
    //resetSendCt = 0; //we got a connect so reset
    day = (GetBuffer[6] - '0') * 10 + (GetBuffer[7] - '0');
    if (day != lastDay)
    {
      lastDay = day;
#ifdef WS_PIN      
      wgMaxYesterdayCt = wgMaxTodayCt;
      wgMaxTodayCt = 0;
#endif
    }
  }
  
/*     
  DEBUG_PRINT("GetBufferIdx",GetBufferIdx);
  if(GetBufferIdx>13) {
      //resetSendCt = 0; //we got a connect so reset
      int day = (GetBuffer[6] - '0') * 10 + (GetBuffer[7] - '0');
      long year = (GetBuffer[0] - '0') * 1000 + (GetBuffer[1] - '0') * 100 + (GetBuffer[2] - '0') * 10 + (GetBuffer[3] - '0');
      int month = (GetBuffer[4] - '0') * 10 + (GetBuffer[5] - '0');
      
      DEBUG_PRINT("dd",day);
      DEBUG_PRINT("MM",month);
      DEBUG_PRINT("yyyy",year);
  }  
*/

}


 /* 
void TimeAdj()
{
 if(GetBufferIdx>13)
 {
   resetSendCt = 0; //we got a time so reset reboot hack
 }

   long adj = GetBuffer[16] - '0';
   if (adj > 4) adj -= 5;
  
   adj *= 6;
   //DEBUG_PRINTLN(adj);
   adj = ((adj * 60000) + ((((GetBuffer[17] - '0') * 10) + (GetBuffer[18] - '0')) * 1000));
   
   adj += (GetBuffer[17] - '0'); 
   //DEBUG_PRINTLN(adj);
   adj *= 10;
   adj += (GetBuffer[18] - '0');
   //DEBUG_PRINTLN(adj);
   adj *= 1000;
   //4 minutes adj up and make negitive
   //if (adj > RESET_INTERVAL - 5000) adj -=  RESET_INTERVAL; //300000;
   DEBUG_PRINT(F("adj "));
   DEBUG_PRINT(adj);
   DEBUG_PRINT(F(" "));
   DEBUG_PRINT(resetMillis);
   if (abs(adj) > 1000  && resetMillis > abs(adj) && abs(adj) < RESET_INTERVAL ) {
       resetMillis -= adj;
       nextConnect -= adj;
   }
   DEBUG_PRINT(F(" "));
   DEBUG_PRINTLN(resetMillis);
 }
}
*/


