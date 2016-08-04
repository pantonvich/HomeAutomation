#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

#define STRING_buf_SIZE 300
#define SEND_INTERVAL 900000

//3 is key pad bus
//2 is alarm bus
SoftwareSerial ademco(2,3,true);

#define getURL "GET /webbash.cgi?"
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xB2};
byte ip[] = { 192, 168, 0, 178 };
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[]  = { 255, 255, 255, 0 };

byte remoteServer[] = { 192,168,0,77 };
EthernetClient client;

char buf[STRING_buf_SIZE];
int buf_idx = 0; // reset buffer
unsigned long lastConnectMillis=SEND_INTERVAL;

void setup()
{
  Ethernet.begin(mac, ip,gateway, gateway, subnet);
  ademco.begin(2400);
  buf[buf_idx++] = '*';
}

int inRead;
int lastRead = 0;
int clearLine = 0;
unsigned int linedata[] = {0,0,0,0,0,0,0,0,0,0};
int stat = 9;
int lastStat = -1;
int fourth = 0;
int third = 0;
void loop()
{
	//if (Serial.available() > 0) 
	//{
	//  byte inChar;
	//inChar = Serial.read();
	//  if(inChar == 'g')
	//  {
	//     sendGET(); // call client sendGET function
	//  }
	//}  

	ademo_loop();

}

void ademo_loop()
{
	int i;
        unsigned long currentMillis;
        
	if (ademco.available()>0)
	{
		inRead = ademco.read();

			//Serial.print(inRead,HEX);
			//Serial.print(" ");

		if ( inRead != 0 || lastRead != 0)  
		{
			linedata[clearLine++] = inRead;

			if (clearLine>9 ) 
			{
				buf_add_bin2hex((unsigned int *)linedata, clearLine-1,0); //sizeof(buf)
				sendGET();
				clearLine =0;
			}


		}
		else if (inRead == 0 && lastRead == 0 && clearLine>3)
		{
			stat = 0;
			for(i=clearLine-4;i<clearLine-1;i++)
			{
				stat += linedata[i];
			}
                        third = clearLine-4;
                        fourth = clearLine-5;
                        if (fourth < 0) fourth = 0;
                        
                        currentMillis=millis();
			if (lastStat != stat || lastStat==-1 || currentMillis-lastConnectMillis > SEND_INTERVAL )
			{
                                lastConnectMillis=currentMillis;
				buf_add_bin2hex((unsigned int *)linedata, clearLine-1,0); //sizeof(buf)
                                
                                if (stat ==400)
				 {buf_add_char((unsigned char *)"|ok",3);}
				else if (stat ==596)
				 {buf_add_char((unsigned char *)"|nr",3);}
				else if (stat ==566)
				 {buf_add_char((unsigned char *)"2|arm",5);}
				else if (stat ==514)
				 {buf_add_char((unsigned char *)"|disarm",7);} 
				else if (stat ==334)
				 {buf_add_char((unsigned char *)"|disarm",7);} 
				else if (stat ==466 || stat ==617 || stat ==649)
				 {buf_add_char((unsigned char *)"|2",2);}
				else if (stat ==616 && linedata[fourth]==0x3f)
				 {buf_add_char((unsigned char *)"|2",2);}
				else if (stat ==616 ) //&& linedata[fourth]==0xfe 0xe7
				 {buf_add_char((unsigned char *)"2|entry",7);}
				else if (stat ==641 || stat==664 || stat==665 ) 
				 {buf_add_char((unsigned char *)"2|entry",7);}
				else if (stat ==560)
				 {buf_add_char((unsigned char *)"4|arm",5);}
				else if (stat ==460)
				 {buf_add_char((unsigned char *)"|4",2);}
				else if (stat ==610)
				 {buf_add_char((unsigned char *)"|4",2);}
				else if (stat ==547)
				 {buf_add_char((unsigned char *)"7|arm",5);}
				else if (stat ==597 || stat ==630)
				 {buf_add_char((unsigned char *)"|7",2);}
				else if (stat ==539)
				 {buf_add_char((unsigned char *)"3|arm",5);} 
				else if (stat ==589  && (linedata[fourth]==0x7f || linedata[fourth]==0xff))
				 {buf_add_char((unsigned char *)"|3",2);}
				else if (stat ==589 || stat ==614)
				 {buf_add_char((unsigned char *)"3|entry",7);} 
				else if (stat ==590)
				 {buf_add_char((unsigned char *)"|3",2);}
				else if (stat==688 || stat==692)
				 {buf_add_char((unsigned char *)"23|nr",5);}
                                else if (stat ==496 && linedata[third]==0x7c )
				 {buf_add_char((unsigned char *)"|fc",3);} 
                                else if (stat ==496 || stat ==646)
				 {buf_add_char((unsigned char *)"nr|disarm",9);} 
                                else if (stat ==526 )
				 {buf_add_char((unsigned char *)"fc|disarm",9);} 
				else if (stat==644)
				 {buf_add_char((unsigned char *)"fc|arm",7);}
				else if (stat==628)
				 {buf_add_char((unsigned char *)"ac|arm",7);}
				else if (stat==640)
				 {buf_add_char((unsigned char *)"fc|entry",11);}
                                else if ( stat == 247)
				 {buf_add_char((unsigned char *)"|alarm",6);}
                                else if (stat ==377 || stat == 525 || stat == 535 || stat == 587)
				 {buf_add_char((unsigned char *)"3|alarm",7);}
                                else if (stat ==634 || stat ==662)
				 {buf_add_char((unsigned char *)"2|alarm",7);}
				else if (stat==674)
				 {buf_add_char((unsigned char *)"alm|disarm",10);} 
				else if (stat==678 || stat==526)
				 {buf_add_char((unsigned char *)"alm|lock",8);}
				else if (stat ==295)
				 {buf_add_char((unsigned char *)"bat|4",5);}
				else if (stat ==340)
				 {buf_add_char((unsigned char *)"bat|disarm",10);}
				else if (stat ==343)
				 {buf_add_char((unsigned char *)"bat|2",5);}
				else if (stat ==395)
				 {buf_add_char((unsigned char *)"bat-4|arm",9);}
				else if (stat ==398)
				 {buf_add_char((unsigned char *)"bat|ok",6);}
				else if (stat ==443)
				 {buf_add_char((unsigned char *)"bat-2|arm",9);}
				else if (stat ==524)
				 {buf_add_char((unsigned char *)"bat|nr",6);}
				else if (stat ==525)
				 {buf_add_char((unsigned char *)"bat-3|arm",9);}
				else if (stat ==533)
				 {buf_add_char((unsigned char *)"bat|7",5);}
				else if (stat ==498 || stat ==562)
				 {buf_add_char((unsigned char *)"fc-alm|clr",10);}
				else
				 {buf_add_char((unsigned char *)"|?",2);}   

				sendGET();

			}

			lastStat = stat;
			clearLine=0;

		}
		lastRead=inRead;
	}
}

void sendGET() //client function to send and receive GET data from external server.
{
  unsigned long connectLoop = 0;
  //set end of string
  buf[buf_idx] = '\0';
  buf_idx=0;

  if (client.connect(remoteServer,80)) {
  } else {
    Ethernet.begin(mac, ip, gateway, gateway, subnet); 
    client.connect(remoteServer,80);
  }
  if (client.connected()) {
    client.print(getURL); //"GET /webbash.cgi?n=123 HTTP/1.0");
    client.print("[me]");
    client.print(ip[3]);
    client.print("[Ah]");
    client.println(buf); 
    client.print(F(" HTTP/1.0"));
    client.println();
  }   
  while(client.connected() && !client.available()) { 
    delay(1); //waits for data
    if (connectLoop++ > 10000) break;
  }
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read();
    if (connectLoop++ > 10000) break;
  }
  client.stop();
  //lastconnect=millis();
}

void buf_add_char(unsigned char *str,unsigned int strsz)
{
	unsigned int  i;
	for (i = 0; i < strsz; i++)
	{
		buf[buf_idx++] =str[i];
	}
}

void buf_add_bin2hex(unsigned int *bin, unsigned int binsz,unsigned int binstart)
{
	//http://stackoverflow.com/questions/6357031/how-do-you-convert-buffer-byte-array-to-hex-string-in-c

	char hex_str[]= "0123456789abcdef";
	unsigned int  i;

	if (!binsz)
		return;

	for (i = binstart; i < binsz; i++)
	{
		buf[buf_idx++] = hex_str[bin[i] >> 4  ];
		buf[buf_idx++]= hex_str[bin[i] & 0x0F];
		buf[buf_idx++] = ':';
	}  
}




