#include "DHT.h"


DHT dht;
// Synchronization loop for long intervals (more than 32 seconds)
#define TimeLapLong(t1) (long)((unsigned long)millis()-(unsigned long)t1)
#define TimeLapWord(t1) (int)((word)millis()-(word)t1)
const long WATER_DEBOUNCE = 5;
const long LIGHTNING_1 = 250;
const long LIGHTNING_2 = 500;
const long TIME_SEND = 10000;
const long TIME_CLICK = 60000;

const int dht_basement_pin = 3;
const int lightning_pin = 2;
const int water_pin = 4;

const int errorCt = 50;
unsigned long waterTotal = 0;

int waterLastRead = LOW;

unsigned long waterHit = 0;
//unsigned int waterDebounce = 50;

#define OUTBUFFER_SIZE 300

char outbuffer[OUTBUFFER_SIZE+1];
int outbufferIdx = 0;

unsigned long updatesSince = 0;

unsigned int lightning=0;
unsigned long lightning10=0;
unsigned long lightningTotal=0;
unsigned long lightningTotal60=0;
unsigned int lightning60[60];
unsigned int lightningPos = 0;
unsigned long lightningAttach1 = 0;
unsigned long lightningAttach2 = 0;

unsigned int water5[5];

void setup(){
  
  pinMode(lightning_pin, INPUT);
  digitalWrite(lightning_pin,HIGH);
  for(int x = 0;x<60;x++) lightning60[x] = 0;
  
  pinMode(water_pin,INPUT);
  digitalWrite(water_pin,HIGH);
  waterLastRead = digitalRead(water_pin);
  for(int x = 0;x<5;x++) water5[x] = 0;
  
  dht.setup(dht_basement_pin);
  
  Serial.begin(115200);
}


int interupoff = 1;
int swappedState = 1;

unsigned long timetosend =0;
unsigned long timeclick=0;
//unsigned long timetoattach=0;


void loop()
{
  
  //if (digitalRead(water_pin) != waterLastRead &&  (millis()-waterHit) > waterDebounce ) {
   // if (digitalRead(water_pin) != waterLastRead &&  TimeLapWord(waterHit) >=0 ) {
     
    if (digitalRead(water_pin) != waterLastRead &&  (millis() >= waterHit) ) {
      if(waterLastRead == LOW)	waterTotal++;
      waterLastRead = !waterLastRead;
    waterHit = millis() + WATER_DEBOUNCE; //millis();
 }
  
  int dRead = digitalRead(lightning_pin);

  //Serial.println(dRead);

  //if (swappedState == 0 && interupoff == 0 && dRead == 1 && (millis() - timetoattach) > 250) swappedState = 1;
  if (swappedState == 0 && interupoff == 0 && dRead == 1 && TimeLapWord(lightningAttach1)) swappedState = 1;
  
   if (interupoff == 1) {
     if ( dRead == 0) {
       lightningAdd();
     }
	 } else if(TimeLapWord(lightningAttach2) && swappedState == 1){
   
      interupoff = 1;
   }
    
    //if(millis() - timeclick > 60000) {
      if(TimeLapLong(timeclick)>=0) {
        timeclick += TIME_CLICK; //millis();
        
        for(int x = 0;x<4;x++) water5[x] = water5[x+1];
        water5[4] = waterTotal;
        
        lightningTotal60 = 0; 
        lightning60[lightningPos]=lightning;

        for(int x = 0;x<60;x++) {
          if (lightning60[x] < errorCt) lightningTotal60 += lightning60[x];
        }
        
        lightning10=0;
        int z;
        for(int x = 0;x<10;x++) {
          z = lightningPos - x;
          if (z<0) z += 60;
          if (lightning60[z] < errorCt) lightning10 += lightning60[z];
        }
      
        lightningPos++;
        if (lightningPos>59) lightningPos = 0;

        lightning = 0;
    }
    
    //if(millis() - timetosend > 10000)   // 10000 every 10 seconds
    if(TimeLapWord(timetosend)>=0)
    {
      timetosend += TIME_SEND; //millis(); // += 10000; //millis();
      
      outbufferIdx = 0;
          
      OutTitle('T','0');
      OutBufferPrint(0,lightning,1);
      
      OutTitle('T','t');     
      OutBufferPrint(1,lightningTotal,1);

      OutTitle('T','l');
      if (lightning < errorCt) {
        OutBufferPrint(2,lightning10 + lightning,1);
      } else {
        outbuffer[outbufferIdx++] = '0';
      }
      
      OutTitle('T','6');
      if (lightning < errorCt) {
        OutBufferPrint(2,lightningTotal60 + lightning,1);
      } else {
        outbuffer[outbufferIdx++] = '0';
      }
      
      OutTitle('H','0');
      OutBufferPrint(1,waterTotal,1);
      
      OutTitle('H','5');
      OutBufferPrint(1,waterTotal-water5[0],1);
      
      OutTitle('H','r');
      OutBufferPrint(1,waterLastRead,1);
            
      float humidity = dht.getHumidity();
      float temperature = dht.getTemperature();
      
      OutTitle('b','H');
      OutBufferPrint(4,humidity,100);
      
      OutTitle('b','T');
      OutBufferPrint(5,temperature,100);
      
      int hold = temperature * 10.0 - ((int)temperature) * 10;
      
      
      OutTitle('T','u');
      OutBufferPrint(3,updatesSince,1000);
      updatesSince++;

      //OutTitle('T','i');
      //
      //for (int x=0;x<60;x++){
      //  int v = (int)lightning60[x];
      //    if (v>9) Out('.');
      //    if (x == lightningPos) Out('>');
      //    OutBufferPrint(4,v,1);
      //    if (v>9) Out('.');
      //}
            
      Out(':');
      //OutBufferPrint(5,millis(),1);
      //Out(':');
      //OutBufferPrint(6,millis()- timeclick,1);
      //Out(':');
      //OutBufferPrint(7,dRead,1);
      //Out(':');
      OutBufferPrint(8,outbufferIdx,1);
      outbuffer[outbufferIdx++] ='\n';
      outbuffer[outbufferIdx++] ='\0';
      
      Serial.println(outbuffer);   
    }
  }
  
void OutTitle(char c1, char c2)
{
  if (outbufferIdx < OUTBUFFER_SIZE -4) {
    outbuffer[outbufferIdx++] = '[';
    outbuffer[outbufferIdx++] = c1; //'T';
    outbuffer[outbufferIdx++] = c2;
    outbuffer[outbufferIdx++] = ']';
  }
}
void Out(char c)
{
  if (outbufferIdx < OUTBUFFER_SIZE -1) {
  outbuffer[outbufferIdx++] = c;
  }
}
void OutBufferPrint(int id, unsigned long reading,unsigned long  i) {
  unsigned long temp; 
  
  //if (id > 10){
  //  outbuffer[outbufferIdx++] = 48+(id/10);
   // id -= (id/10)*10;
  //}
  
  //outbuffer[outbufferIdx++] = ',';
  
  while (i*10 <= reading){
   i*=10; 
  }
  
  while (i>0){
    if (outbufferIdx >= OUTBUFFER_SIZE -1) break; 
     temp = reading/i;
     reading -= temp * i;
     outbuffer[outbufferIdx++] = 48+temp;
     i/=10;
    }
}  

void lightningAdd()
{
  //timetoattach = millis();
  lightningAttach1 = millis() + LIGHTNING_1;
  lightningAttach2 = millis() + LIGHTNING_2;
  interupoff = 0;
  swappedState = 0;
  lightning++;
  lightningTotal++;
}

//int freeRam1 () {
//  extern int __heap_start, *__brkval; 
//  int v; 
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
//}


