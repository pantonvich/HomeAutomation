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

#define power_apin 0
//#define rain_detector_apin 4 //1
//#define pyranometer_apin 2
#define gas_apin 1 //4

#define SD_Card_dpin 4
#define water_dpin 6
#define inside_onewire_dpin 7
#define outside_onewire_dpin 8
#define RESET_DPIN 9  

#define getURL F("GET /ardbash.cgi?")
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xB1};
byte ip[] = { 192, 168, 0, 177};

//#define getURL F("GET /tstbash.cgi?")
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0F};
//byte ip[] = { 192, 168, 0, 15};

byte gateway[] = { 192, 168, 0, 1 };
byte subnet[]  = { 255, 255, 255, 0 };
byte rip[4];

int gasTripOffset = 15; //20; //75;
float powerTripLow = 1.000; //.98; //was .90
float powerTripHigh = 1.003;   //1.008;
unsigned long SEND_INTERVAL =  60000; //300000;
unsigned long RESET_INTERVAL = 300000;
unsigned long WATER_DEBOUNCE = 10;

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
unsigned long lastConnectMillis=0;
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
//int lastday = 0;

//   rain_detector
//        +-----------A
//        |
//  ||----+----100K---5+
//  +-----------------gnd
//

//   pyranometer
//  ------+-----------A
//        |
//       ???K
//        |
//  +-----+-----------gnd
//

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
  
  //pinMode(gas_digital_out,OUTPUT);
  pinMode(gas_apin, INPUT);
  pinMode(power_apin, INPUT);
//  pinMode(pyranometer_apin, INPUT);
//  pinMode(rain_detector_apin, INPUT);
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
  PowerLoop();
  GasLoop();
  WaterLoop();
  //  PyranometerRead();
  //rainDetectorTotal += analogRead(rain_detector_apin);
  //count++;
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

   if ((currentMillis - lastConnectMillis) > SEND_INTERVAL ) 
   { 
    Serial.println("send");
     lastConnectMillis = currentMillis; 
     ClientMode();
     resetAll();
     TimeAdj(); //if got returned time from client reset resetSendCt
     resetSendCt++; //reboot hack
   } 

   //reboot hack
  if (resetSendCt > 3) { digitalWrite(RESET_DPIN, LOW); }
}
 
 ISR (WDT_vect)
 {
   wdt_disable();
   Serial.println(F("ISR"));
   delay(5000);
   digitalWrite(RESET_DPIN, LOW);
   
 }
void ClientMode()
{
  IPAddress nasServerIP(192,168,0,77);
  unsigned long connectLoop = 0;
  unsigned long inValue[5];
  int inPos =-1;
  inValue[0] = 0;inValue[1] = 0;inValue[2] = 0;	inValue[3] = 0;inValue[4] = 0;inValue[5] = 0;
  
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
         if (inPos > -1 && c != '|') inValue[inPos] = inValue[inPos] * 10 + (c - '0');       
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
  
  Serial.println(GetBuffer);
  //for(int x = 0;x<inPos+1;x++) Serial.println(inValue[x]);
  //Serial.println(getBuffer2);
  if (inPos > 4) {
   //Serial.println(F("set"));
   if(inValue[0] > 0) powerTripLow =  (float)inValue[0]/1000.0;
   if(inValue[1] > 0) powerTripHigh = (float)inValue[1]/1000.0;
   if(inValue[2] > 0) gasTripOffset = inValue[2];
   if(inValue[3] > 0) SEND_INTERVAL = inValue[3];
   if(inValue[4] > 0) RESET_INTERVAL = inValue[4];
   if(inValue[5] > 0) WATER_DEBOUNCE = inValue[5];
  }

}

void TimeAdj()
{
 if(GetBufferIdx>13)
 {
   resetSendCt = 0; //we got a time so reset reboot hack
  } 
  /*
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
 */
}

void ServerMode()
{    
  char c;
  int flag = 0;
  unsigned long connectLoop = 0;
  GetBufferIdx = 0; 
  client = server.available();
  
  if (client) {    
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
 
  if (AlertType>0) {
    outbuffer[outbufferIdx++] = 'W';
    outbuffer[outbufferIdx++] = 'A';
    outbuffer[outbufferIdx++] = 'T';
    outbuffer[outbufferIdx++] = 'E';
    outbuffer[outbufferIdx++] = 'R';
    outbuffer[outbufferIdx++] = '0'+waterLastRead;
    
    
  } else {
	GetOneWireTemps(inside_onewire_dpin);
	//GetOneWireTemps(outside_onewire_dpin);
	Print();
  }
  outbuffer[outbufferIdx] = '\0';
  outbufferIdx=0;
}
	
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
        //Serial.print("CRC is not valid!\n");
        return;
      }

  if ( addr[0] == 0x28) { 
    // The DallasTemperature library can do all this work for you!
    ds.reset();
    ds.select(addr);
    ds.write(0x44,0);         // start conversion, with parasite power on at the end
    
    runDelayLoops(10);
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
int powerReading = 0;

void PowerLoop() {
    
  powerReading = analogRead(power_apin);
  powerFilterH += .01 * (powerReading-powerFilterH);
  powerFilterC += .9 * (powerReading-powerFilterC);
  powerRatio = powerFilterC/powerFilterH;
  
  if (powerRatio < powerMaxL) powerMaxL = powerRatio;
  if (powerRatio > powerMaxH) powerMaxH = powerRatio;

  if ((powerRatio < powerTripLow && powerBlinkFlag ==1 && currentMillis-powerLastBlink > 50) || powerLastBlink > currentMillis)  {
      powerBlinkFlag = 0;
      powerCount++;
      float rate = currentMillis-powerLastBlink;
      if(rate>0) powerBlinkRate += .05 *(rate-powerBlinkRate);
      powerLastBlink = currentMillis;   
  }
  else if (powerRatio > powerTripHigh && powerBlinkFlag == 0)
  {
    if ( currentMillis-powerLastBlink > 30 || powerLastBlink > currentMillis ){
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
 else { powerW5 = powerCount;}
 //else if (w < 300000) { powerW5 = powerCount; }
 //else { powerW6 = powerCount;}
}

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
  
  gasReading = analogRead(gas_apin); 
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
     
 unsigned long w = currentMillis - lastConnectMillis;
 
 if ( w < 60000) { water5[1] = waterTotal; }
 else if (w < 120000) { water5[2] = waterTotal; }
 else if (w < 180000) { water5[3] = waterTotal; }
 else if (w < 240000) { water5[4] = waterTotal; }
 
 water5[5] = waterTotal; 
}

void Print() {

   bPrint('K','0',powerCount,10000);
   
   if(powerW1 == 0 )
    bPrint('w','1',0,100);
   else
    bPrint('w','1',powerW1-powerCountLast,100);

   if(powerW2 == 0 )
    bPrint('w','2',0,100);
   else   
    bPrint('w','2',powerW2-powerW1,100);
   
   if(powerW3 == 0 )
    bPrint('w','3',0,100);
   else
    bPrint('w','3',powerW3-powerW2,100);
   
   if(powerW4 == 0 )
    bPrint('w','4',0,100);
   else
    bPrint('w','4',powerW4-powerW3,100);
   
   if(powerW5 == 0 )
    bPrint('w','5',0,100);
   else
    bPrint('w','5',powerW5-powerW4,100);
   
   //if(powerW6 == 0 )
   // bPrint('w','6',0,100);
   //else
   // bPrint('w','6',powerW6-powerW5,100);
  
  
   bPrint('K','5',powerCount - powerCountLast ,1000);
   bPrint('H','0',waterTotal,100);
   bPrint('H','5',waterTotal-water5[0],1);
	
   for(int x = 1;x<6;x++) {
	   int t = 0;
	   if (water5[x] <= water5[5]) t=water5[x]-water5[x-1];
	   bPrint('h','0' + x,t,1);
   }

      //bPrint('P','0',(unsigned long)pyranometerReading,1000);
      //bPrint('P','r',pr,1000);
      //bPrint('P','A',pyramometerTotal/(count/100),1000);
      //bPrint('R','0',analogRead(rain_detector_apin),1000);
      //bPrint('R','A',rainDetectorTotal/count,1000); 
    bPrint('G','0',gasTotalCount,1000);
    bPrint('G','5', gasTotalCount - gasLastCount,1);
     //bPrint('G','o', gasMaxMax - gasLowLow,10);
     //bPrint('h','r',waterLastRead,10);
     //bPrint('f','H',powerFilterH,1000);
     //bPrint('f','C',powerFilterC,1000);
    bPrint('B','L',powerBlinkLength,1000);
    bPrint('B','R',powerBlinkRate,1000);
    bPrint('M','H',powerMaxH*1000,1000);
    bPrint('M','L',powerMaxL*1000,100);
    bPrint('G','r',gasReading,1000);
    bPrint('G','f',gasFilterL,1000);
    bPrint('G','l',gasLowLow,1000);
    bPrint('G','h',gasMaxMax,1000);
    bPrint('G','d', gasMaxMax - gasLowLow,10);
    bPrint('G','t',gasTripOffset,10);
    bPrint('M','l',powerTripLow*1000,100);
    bPrint('M','h',powerTripHigh*1000,1000);
    bPrint('C','l',ctLoop,1000);
    bPrint('F','r',freeRam(),1000);
    bPrint('m','e',ip[3],100);
    //outbuffer[outbufferIdx] = '\0';
 }

void resetAll()
{
  water5[0]=waterTotal;
  powerCountLast = powerCount;
  powerMaxH = 0;
  powerMaxL = 1;
  gasLastCount = gasTotalCount; 
  gasLowLow = 1023;
  gasMaxMax = 0; 
  powerW1=0;powerW2=0;powerW3=0;powerW4=0;powerW5=0;//powerW6=0;
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
  
   bTitle(c1, c2);
   BufferPrint(reading,i); 
}

void BufferPrint(unsigned long reading,unsigned long i) {
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
	
//void buf_add_char(unsigned char *str,unsigned int strsz)
//{
//  if ((outbufferIdx + strsz) > STRING_BUFFER_SIZE) return;
  
//  unsigned int  i;
//  for (i = 0; i < strsz; i++)
//  {
//    outbuffer[outbufferIdx++] =str[i];
//  }
 
//}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
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

