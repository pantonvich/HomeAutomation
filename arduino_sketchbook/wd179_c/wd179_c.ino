
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
#include <avr/wdt.h>

//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

//#define SEND_INTERVAL 30000
#define SEND_INTERVAL 120000 

#define GUST_INTERVAL 10000


#define DHT_BASEMENT_PIN 2
#define DHT_BASEMENT_PIN1 3
#define SD_Card_dpin 4
#define BELL_PIN 5
#define DHT_BASEMENT_PIN1 6
#define lightning_pin 7
#define WD_PIN 8
#define WS_PIN 9
#define RESET_DPIN 14  //A0

#define WS_DEBOUNCE 2
#define BELL_DEBOUNCE 500

DHT dht(DHT_BASEMENT_PIN, DHTTYPE);
DHT dht1(DHT_BASEMENT_PIN1, DHTTYPE);

#define STRING_BUFFER_SIZE 300
char outbuffer[STRING_BUFFER_SIZE+2];
int outbufferIdx = 0;

//#define getURL "GET /testbash.cgi?" 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xB4 };
byte ip[] = { 192, 168, 0, 180 };

#define getURL F("GET /windbash.cgi?") 
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xB3 };
//byte ip[] = { 192, 168, 0, 179 };
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[]  = { 255, 255, 255, 0 };

EthernetServer server(80);
EthernetClient client;

unsigned long lastConnectMillis=SEND_INTERVAL;
unsigned int resetSendCt = 0L;
int bellLastRead = LOW;

unsigned long bellHitMillis=0;
unsigned long bellTotal = 0;
unsigned int bell2Min = 0;

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
unsigned long currentMillis;

void setup()
{
  digitalWrite(RESET_DPIN, HIGH); //this is a hack!
  wdt_disable();
  pinMode(RESET_DPIN, OUTPUT);
  
  //disable SD card
  pinMode(SD_Card_dpin, OUTPUT);
  digitalWrite(SD_Card_dpin, HIGH);
  
  //wdt_disable();
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
  
  Serial.begin(115200);
 Serial.println(F("str"));
  
  pinMode(BELL_PIN, INPUT);
  pinMode(WS_PIN, INPUT);
  pinMode(WD_PIN, INPUT);
   
  digitalWrite(BELL_PIN,HIGH);
  digitalWrite(WS_PIN,HIGH);
  digitalWrite(WD_PIN,HIGH);
  
  bellLastRead = digitalRead(BELL_PIN);
  wsLastRead = digitalRead(WS_PIN);
  
//wdt_enable(WDTO_8S);
// clear various "reset" flags
 MCUSR = 0;
 // allow changes, disable reset
 WDTCSR = bit (WDCE) | bit (WDE);
 // set interrupt mode and an interval 
 WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay;
  
  outbuffer[outbufferIdx++] = '*';
}

void runLoops() 
{
  wdt_reset();
  currentMillis = millis();
  BellLoop();
  WindLoop();   
}

void runDelayLoops(int milliseconds)
{
  static unsigned long holdMillis;
  holdMillis = currentMillis;
  
  while ( currentMillis - holdMillis < milliseconds)
  {
    runLoops();
  }
}

void loop()
{
  wdt_reset();
  ServerMode();
  runLoops();

  
  if (currentMillis - lastConnectMillis > SEND_INTERVAL ) {
    lastConnectMillis = currentMillis;
    resetSendCt++;

    BuildOutput();
    SendGET();

    //reset values
    //wgMax = 0; //set gust to to max 4294967295
    bell2Min=0;
    wsSendCt=0;
    wgMaxSendCt = 0; 
  }
  if (resetSendCt > 3) {
    digitalWrite(RESET_DPIN, LOW); 
  }
}

 ISR (WDT_vect)
 {
   wdt_disable();
   //Serial.println(F("ISR"));
   delay(5000);
   digitalWrite(RESET_DPIN, LOW);
   
 }
 
void BellLoop()
{
  
  int bellRead =digitalRead(BELL_PIN);
  if (bellRead == HIGH && bellLastRead==LOW && (currentMillis - bellHitMillis > BELL_DEBOUNCE)) 
  { 
    bellTotal++;
    bell2Min++;
    bellHitMillis=currentMillis;

    outbuffer[outbufferIdx++] = 'B';
    outbuffer[outbufferIdx++] = 'E';
    outbuffer[outbufferIdx++] = 'L';
    outbuffer[outbufferIdx++] = 'L';
    outbuffer[outbufferIdx] ='\0';
    outbufferIdx=0;
    
    SendGET();
  }

    //Serial.println(bellRead);
   // delay(2);
  bellLastRead=bellRead;   
}


void WindLoop()
{
  
  int wsRead=digitalRead(WS_PIN);
  int wdRead=digitalRead(WD_PIN);
  
  //m = millis();
  
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
    //Serial.print(bellTotal);
    //Serial.print(" ");
//Serial.println(bellLastRead);
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

void PrintWind()
{

        bPrint('W','D',wd,100);
        
        //OutTitle('W','S');
        //OutBufferPrint(1,getMPH(wsCt,GUST_INTERVAL/1000),1000);
        
        bPrint('W','S',getMPH(wsSendCt,SEND_INTERVAL/1000),1000);  
        //OutBufferPrint(1,getMPH(ws2,GUST_INTERVAL/1000),1000);   
        
        bPrint('W','G',getMPH(wgMaxSendCt,GUST_INTERVAL/1000),1000);
        
        //OutTitle('W','g');
        //OutBufferPrint(1,wgMax,100);
        
        bPrint('w','2',wsSendCt,100); 
        
        bPrint('g','m',wgMaxTodayCt,100);
        
        bPrint('g','y',wgMaxYesterdayCt,100);
        
}

void  PrintHum() {   

      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();
      
      //Serial.print(humidity);
      
      bPrint('b','H',humidity*10,100);
      
      bPrint('b','T',temperature*10,100);
      
      humidity = dht1.readHumidity();
      temperature = dht1.readTemperature();
      
      //Serial.print(humidity);
      
      bPrint('1','H',humidity*10,100);
      
      bPrint('1','T',temperature*10,100);
}      


void  PrintBell() {   

      bPrint('B','0',bellTotal,1);
      
      bPrint('B','2',bell2Min,1);
} 

void PrintMeIp()
{
  bPrint('m','e',ip[3],1);
}

void BuildOutput() {
    PrintWind();  
    PrintHum();
    PrintBell();
    PrintMeIp();
    outbuffer[outbufferIdx] ='\0';
    Serial.println(outbuffer);
    outbufferIdx=0;
}

void SendGET() //client function to send and receive GET data from external server.
{
  static int lastDay = 0;
  IPAddress nasServerIP(192,168,0,77);
  int flag = 0;
  char getBuffer[76];
  int getBufferIdx = 0;
  int day;
  unsigned long connectLoop = 0;

  if (client.connect(nasServerIP,80)) {
    client.print(getURL);
    client.println(outbuffer); //"GET /webtest.cgi?n=123 HTTP/1.0");
    client.print(F(" HTTP/1.0"));
    client.println();
  } 
  else {
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    if (client.connect(nasServerIP,80)) {
      client.print(getURL);
      client.println(outbuffer); //"GET /webtest.cgi?n=123 HTTP/1.0");
      client.print(F(" HTTP/1.0"));
      client.println();
    }
  }
  
  while(client.connected() && !client.available()) {
    runLoops(); //waits for data
    if (connectLoop++ > 10000) break;
  }
  while (client.connected()) {
    Serial.println("C");
    
    while (client.available()) { //connected or data available
    Serial.println("A");
    char c = client.read();
    if (c >= '0' && c <= '9' && getBufferIdx < 74)
     {
       getBuffer[getBufferIdx++] = c;
     }
    if (connectLoop++ > 10000) break;
    }
    if (connectLoop++ > 10000) break;
  }

  client.stop();
  
  if(getBufferIdx>13) {
    resetSendCt = 0; //we got a connect so reset
    day = (getBuffer[6] - '0') * 10 + (getBuffer[7] - '0');
    if (day != lastDay)
    {
      lastDay = day;
      wgMaxYesterdayCt = wgMaxTodayCt;
      wgMaxTodayCt = 0;
    }
  }
  getBuffer[getBufferIdx]='\0';
  Serial.println(getBuffer);
}

void ServerMode()
{  
  
  char c;
  int flag = 0;
  char getBuffer[100];
  int getIdx = 0;
  unsigned long connectLoop = 0;
  
  client = server.available();
  
  if (client) {
    Serial.println(F("SM "));
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    while (client.connected()) {
      unsigned long startTime = millis();

      while ((!client.available()) && ((millis() - startTime ) < 5000)){
        runLoops();  
      };
      
      if (client.available()) {
        if (connectLoop++ > 10000) break;

        c = client.read();
        if (outbufferIdx<STRING_BUFFER_SIZE && flag<2) 
        {
          if (flag==1 && c==' ')
            flag=2;
          
          if (flag==1) {           
            getBuffer[getIdx++] = c;
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
          client.println(outbuffer);
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
    // give the web browser time to receive the data
    //delay(1);
    
    runDelayLoops(2);
    //delay(1);
    client.flush();
    client.stop();
    
    //getBuffer[getIdx++]='\0';
    //Serial.println(getBuffer);
}
}

void bTitle(char c1, char c2) {
  
  if ((outbufferIdx + 4) > STRING_BUFFER_SIZE) return;
  
   outbuffer[outbufferIdx++] = '[';
   outbuffer[outbufferIdx++] = c1;
   outbuffer[outbufferIdx++] = c2;
   outbuffer[outbufferIdx++] = ']';
}
/*
void Out(char c)
{
  if (outbufferIdx < STRING_BUFFER_SIZE ) {
  outbuffer[outbufferIdx++] = c;
  }
}

void buf_add_char(unsigned char *str,unsigned int strsz)
{
  if ((outbufferIdx + strsz) > STRING_BUFFER_SIZE) return;
  unsigned int  i;
  for (i = 0; i < strsz; i++)
  {
    outbuffer[outbufferIdx++] =str[i];
  }
}
*/

void bPrint(char c1, char c2, unsigned long reading,unsigned long i){ 
  
  bTitle(c1,c2);
  BufferPrint(reading,i); 
}

void BufferPrint( unsigned long reading,unsigned long  i) {
  unsigned long temp; 
  unsigned int iCount = 0;

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
