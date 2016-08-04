/*
* Web Server
*
* A simple web server that shows the value of the analog input pins.
*/
#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <avr/wdt.h>

//#define moisture_input -1

#define PT_APIN 0

#define SD_Card_dpin 4
#define water_dpin 6

#define RESET_DPIN 9  

//#define getURL F("GET /ardbash.cgi?")
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xB1};
//byte ip[] = { 192, 168, 0, 177};

#define getURL F("GET /tstbash.cgi?")
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0F};
byte ip[] = { 192, 168, 0, 15};

byte gateway[] = { 192, 168, 0, 1 };
byte subnet[]  = { 255, 255, 255, 0 };
byte rip[4];

unsigned long SEND_INTERVAL = 3600000;
unsigned long RESET_INTERVAL =  10000;
unsigned long WATER_DEBOUNCE = 10;
unsigned int PSI_TRIP = 3;

EthernetServer server(80);
EthernetClient client;

#define STRING_BUFFER_SIZE 400
char outbuffer[STRING_BUFFER_SIZE+2];
unsigned long outbufferIdx = 0; 

//unsigned long pyramometerTotal = 0;
//unsigned long rainDetectorTotal = 0;
unsigned long waterTotal = 0;
int waterLastRead = LOW;
unsigned long waterHitMillis = 0;
unsigned int water5[6];
unsigned long currentMillis = 0;
unsigned long lastSendMillis=0;
unsigned long lastResetMillis=0;
unsigned long ClientMillis=0;
//unsigned long nextConnect=millis()+RESET_INTERVAL;
unsigned long resetMillis=0;
//unsigned long heartBeat=0;

int AlertType = 0; //0=none, 1=water

unsigned long ctLoop = 0;
unsigned int resetSendCt = 0; //reboot hack

#define STRING_GetBuffer_SIZE 75
char GetBuffer[STRING_GetBuffer_SIZE+2];
unsigned int GetBufferIdx=0;

unsigned long ptLowLow = 1023;
unsigned long ptMaxMax = 0;
float ptFilterL = 900;
unsigned long threshold = 0;
unsigned long ptArr[] = {0,0,0,0,0,0,0,0,0,0,0};

void setup()
{
  digitalWrite(RESET_DPIN, HIGH); //this is a hack!
  wdt_disable();
  pinMode(RESET_DPIN, OUTPUT);

  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
  Serial.begin(115200);
  Serial.println(F("str"));
  
  analogReference(DEFAULT);
  
  pinMode(PT_APIN, INPUT);
  pinMode(water_dpin, INPUT);
  digitalWrite(water_dpin,HIGH);
  
  //disable SD card
  pinMode(SD_Card_dpin, OUTPUT);
  digitalWrite(SD_Card_dpin, HIGH);

  waterLastRead = digitalRead(water_dpin);
  for(int x = 0;x<5;x++) water5[x] = 0;
  //wdt_enable(WDTO_8S);
  // clear various "reset" flags
 MCUSR = 0;
 // allow changes, disable reset
 WDTCSR = bit (WDCE) | bit (WDE);
 // set interrupt mode and an interval 
 WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
 
 outbuffer[outbufferIdx++] = '*';
}

//unsigned long networkConnectTime = 0;

void runLoops() 
{
  wdt_reset();
  currentMillis = millis();
  PressureLoop();
  WaterLoop();
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

int ptSendNextCt = 0;
  
void loop()
{

  runLoops();
  ServerMode();
  bool sendFlag = ( currentMillis - lastSendMillis > SEND_INTERVAL);
  bool tripFlag = ( ptMaxMax - ptLowLow > PSI_TRIP );

  
   if (currentMillis - lastResetMillis > RESET_INTERVAL || sendFlag ) 
   { 
    Serial.println("send");
     lastResetMillis = currentMillis;
     if (tripFlag || sendFlag || ptSendNextCt > 0) {
       //|| ptFilterL < threshold ) {
       if (tripFlag) {
         ptSendNextCt = 3;
         threshold = ptMaxMax - (ptMaxMax - ptLowLow)/2;
       } else if (!sendFlag) { 
         ptSendNextCt--;
       }
       
       if (sendFlag) lastSendMillis = currentMillis;
       
       ClientMode();
       //resetSendCt++; //reboot hack
     }
     resetAll();
     //TimeAdj();
     
   } 

   //reboot hack
  //if (resetSendCt > 3) { digitalWrite(RESET_DPIN, LOW); }
}
 
 ISR (WDT_vect)
 {
   wdt_disable();
   //Serial.println(F("ISR"));
   delay(5000);
   digitalWrite(RESET_DPIN, LOW);
   
 }
void ClientMode()
{
  IPAddress nasServerIP(192,168,0,77);
  unsigned long connectLoop = 0;
  
  int inPos =-1;
  const int inValueCnt = 6;
  unsigned long inValue[] = {0,0,0,0,0,0};
 // for(int x = 0;x<=inValueCnt;x++) {inValue[0] = 0;}
  
  GetBufferIdx = 0;
  
  if (client.connect(nasServerIP,80)) {
  } else {
    Ethernet.begin(mac, ip, gateway, gateway, subnet); 
    runDelayLoops(100);
    client.connect(nasServerIP,80);
  }
  if (client.connected()) {
    client.print(getURL); //"GET /webtest.cgi?n=123 HTTP/1.0");
    BuildOutput();
    Serial.println(outbuffer);
    client.println(outbuffer); 
    client.print(F(" HTTP/1.0"));
    client.println();
   }
  
  while(client.connected() && !client.available()) {
    runLoops(); //waits for data
    if (connectLoop++ > 10000) break;
  }
  while (client.connected()) {
     
     while( client.available()) { //connected or data available
      char c = client.read();
      if (c >= '0' && c <= '9' && GetBufferIdx < STRING_GetBuffer_SIZE)
       {
         GetBuffer[GetBufferIdx++] = c;
         if (inPos > -1 && inPos < inValueCnt) inValue[inPos] = inValue[inPos] * 10 + (c - '0');       
       }
     
       if (c=='|') {inPos++;}
      if (connectLoop++ > 10000) break;
     }
     if (connectLoop++ > 10000) break;
  }
  
  client.stop();
  //Serial.print(F("stp "));
  //Serial.println(connectLoop);	

  GetBuffer[GetBufferIdx] = '\0';
  Serial.print(F("get "));
  Serial.print(GetBufferIdx);
  Serial.print(F("buf "));
  Serial.println(GetBuffer);
  for(int x = 0;x<inValueCnt;x++) { Serial.print(x); Serial.print(" "); Serial.println(inValue[x]); }
  //Serial.println(getBuffer2);
  if (inPos > inValueCnt-2) {
   if(inValue[3] > 0) SEND_INTERVAL = inValue[3];
   if(inValue[4] > 0) RESET_INTERVAL = inValue[4];
   if(inValue[5] > 0) PSI_TRIP = inValue[5];
  }
}
/*
void TimeAdj()
{
 if(GetBufferIdx>18)
 {
   resetSendCt = 0; //we got a time so reset reboot hack
   
   long adj = GetBuffer[16] - '0';
   if (adj > 4) adj -= 5;
  
   adj *= 6;
   //Serial.println(adj);
   adj = ((adj * 60000) + ((((GetBuffer[17] - '0') * 10) + (GetBuffer[18] - '0')) * 1000));
   
   adj += (GetBuffer[17] - '0'); 
   //Serial.println(adj);
   adj *= 10;
   adj += (GetBuffer[18] - '0');
   //Serial.println(adj);
   adj *= 1000;
   //4 minutes adj up and make negitive
   //if (adj > RESET_INTERVAL - 5000) adj -=  RESET_INTERVAL; //300000;
   Serial.print(F("adj "));
   Serial.print(adj);
   Serial.print(F(" "));
   Serial.print(resetMillis);
   if (abs(adj) > 1000  && resetMillis > abs(adj) && abs(adj) < RESET_INTERVAL ) {
       resetMillis -= adj;
       nextConnect -= adj;
   }
   Serial.print(F(" "));
   Serial.println(resetMillis);
 }
}
*/
void ServerMode()
{    
  char c;
  int flag = 0;
  unsigned long connectLoop = 0;
  GetBufferIdx = 0; 
  client = server.available();

  
  if (client) {    
      
    Serial.println("Sever");
    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    while (client.connected()) {
      unsigned long startTime = millis();

      while ((!client.available()) && ((millis() - startTime ) < 5000)){
        runLoops();  
      };

      if (client.available()) {

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
          client.print(outbuffer);
          Serial.println(outbuffer);
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
    runDelayLoops(2);
    client.flush();
    client.stop();

    GetBuffer[GetBufferIdx]='\0';
    Serial.println(GetBuffer);
}
}	
void BuildOutput() {
 
 // if (AlertType>0) {
//    outbuffer[outbufferIdx++] = 'W';
//    outbuffer[outbufferIdx++] = 'A';
//    outbuffer[outbufferIdx++] = 'T';
//    outbuffer[outbufferIdx++] = 'E';
//    outbuffer[outbufferIdx++] = 'R';
//    outbuffer[outbufferIdx++] = '0'+waterLastRead;
    
    
//  } else {
	Print();
//  }
  Serial.println(outbuffer);
  outbufferIdx=0;
}
	
//unsigned long ptTotalCount = 0;
//unsigned long ptLastCount = 0;
//unsigned long ptlastMillis = 0;
unsigned long ptReading = 0;
//unsigned int ptFlag = 0;
//unsigned long ptTripHigh = 0;
//unsigned long ptTripLow = 1023;


float ptFilterC = 900;


void PressureLoop()
{
  
  ptReading = analogRead(PT_APIN); 
    
  ptFilterL += .01 * (ptReading-ptFilterL);
  ptFilterC += .9 * (ptReading-ptFilterC);

  //Serial.print(ptReading);
  //Serial.print(" ");
  //Serial.print(ptFilterL);
  //Serial.print(" ");
  //Serial.println(ptFilterC);
 
  ptReading = ptFilterL;
  
  if(ptReading<ptLowLow) ptLowLow=ptReading;
  if(ptReading>ptMaxMax) ptMaxMax=ptReading;
  unsigned long w = (currentMillis - lastResetMillis) / 1000;
  if (w < 10 && ptArr[w] < ptReading ) ptArr[w] = ptReading;
  //Serial.print(w);
  //Serial.print(" ");
  //Serial.println(ptFilterC);
}

void WaterLoop()
{

  if (digitalRead(water_dpin) != waterLastRead &&  (currentMillis - waterHitMillis > WATER_DEBOUNCE) ) {
    waterHitMillis = currentMillis;	
    AlertType = 1; 
    waterLastRead = !waterLastRead; 
    waterTotal++; //if(waterLastRead == LOW)
    ClientMode();
    AlertType = 0;  
  }
     
 unsigned long w = currentMillis - lastSendMillis;
 
 if ( w < 60000) { water5[1] = waterTotal; }
 else if (w < 120000) { water5[2] = waterTotal; }
 else if (w < 180000) { water5[3] = waterTotal; }
 else if (w < 240000) { water5[4] = waterTotal; }
 
 water5[5] = waterTotal; 
}

void Print() {
   // bPrint('G','r',ptReading,100);
    bPrint('P','f',ptFilterL,100);
    bPrint('P','c',ptFilterC,100);
    bPrint('P','l',ptLowLow,100);
    bPrint('P','h',ptMaxMax,100);
    bPrint('P','d', ptMaxMax - ptLowLow,10);
    bPrint('P','t',threshold,100);
    bPrint('p','T',PSI_TRIP,1);
    bPrint('s','c',ptSendNextCt,1);
  bTitle('P','a');
    for (int i=0;i<10;i++) {
      if (i!=0) outbuffer[outbufferIdx++] = ',';
    BufferPrint(ptArr[i],100);
  }
    
    bPrint('m','e',ip[3],100);
    outbuffer[outbufferIdx] = '\0';
 }

void resetAll()
{

  ptLowLow = 1023;
  ptMaxMax = 0; 
  for (int i=0;i<10;i++) {ptArr[i]=0;}
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
  BufferPrint(reading,i); 
}

void BufferPrint(unsigned long reading,unsigned long i) {
  unsigned long temp; 
  unsigned int iCount = 0;
   if(reading<0) {
    outbuffer[outbufferIdx++]='-';
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

//int freeRam () {
//  extern int __heap_start, *__brkval; 
//  int v; 
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
//}
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
   //Serial.print(rArg);
    //Serial.print("");
    //Serial.print(counter);
    //Serial.print(" ");
    //Serial.print(powerReading);
   // Serial.print(" ");
    //Serial.print(powerFilterC/powerFilterH);
    //Serial.print(" ");
    //Serial.print(topLow);
    //Serial.print(" ");
    //Serial.print(powerBlinkFlag);
    //Serial.println(" "); 
    //return;
//}

