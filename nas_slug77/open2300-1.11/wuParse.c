//cmdarg: C:\personal\Logs\wu-201409.log C:\personal\Logs\wu-201410.log
// gcc -Wall -O3 wuParse.c -o wuParse
// cp wuParse /opt/bin/

#include <string.h>
//#include <fcntl.h>
#include <stdio.h>
//#include <time.h>
#include <stdlib.h>
//#include <stdbool.h>
#include <math.h>

#define OutputLine "%s<br />\n"
#define LINE_ROW "%ld %s i:%3ld %3ld %3ld o:%3ld %3ld %3ld i:%3ld %3ld %3ld o:%3ld %3ld %3ld r:%3.2f<br/>\n"
#define LINE_BREAK "<br/>\n"
#define NumOfStrings 20

//C:\Users\pantonvi\Documents\personal\Arduino\Logs\wu-201003.log 
//3:Indoor, 4:outdoor, 5:dewpoint,6:Hum In,7:Hum Out, 8:speed, 9:direction, 10:direction string, 11:windchill, 12:rain 1H, 13:24HR, 14:Total,15:Relative Press, 16:Tendency and 17:Forecast


const int ColTempIn = 3;
const int ColTempOut = 4;
const int ColHumIn = 6;
const int ColHumOut = 7;
const int ColRainTot = 15;
const int HI = 0;
const int LOW = 1;
const int AVG = 2;
const double rain24RolloverValue = 98.42;
const int InvalidTempInMin = 290;
const int InvalidTempInMax = 1150;
const int InvalidTempOutMin = -150;
const int InvalidTempOutMax = 1760;
const int InvalidHumMin = 0;
const int InvalidHumMax = 1100;

void maxminavg(char* s,long* v,long invalidMin, long invalidMax);
void reset(long* v);
char* dayofweek(char *datestring, int d);


int main(int argc, char *argv[])
{

	FILE *fileptr;
	char str[3000];
	char *pch;
	char strArray[NumOfStrings][NumOfStrings];
	char lstDate[NumOfStrings];
	int pos = 0;
	long iT[3], oT[3], iH[3], oH[3];
	double rain24 = 0.0;
	double rain24tmp = 0.0;
	double tmp;
	int ct = 0;
	int curDay;
	int lstDay=0;
	int argcPos=1;
	
	if (argc < 1 ) 
	{
		printf(OutputLine,"No file specified\n");
	}
	/* Get log filename. */
	//fileptr = fopen( "C:\\Users\\pantonvi\\Documents\\personal\\Arduino\\Logs\\wu-201410.log" , "r");
	while(argcPos < argc)
	{
		int firstRead = 0;

		/* Get log filename. */
		fileptr = fopen(argv[argcPos], "r");

		if (fileptr == NULL)
		{
			printf("Cannot open file %s\n",argv[1]);
			exit(-1);
		}

		while(! feof(fileptr))
		{
			fscanf(fileptr, "\n%[^\n]", str);
			pos = 0;
			pch = strtok (str," ");

			//take each part and push it into array
			while (pch != NULL)
			{
				strcpy(strArray[pos],pch);
				pch = strtok (NULL, " ");
				pos++;
			}

			//get the current day: 2011-Dec-07 01
			curDay = ((strArray[1]+9)[0] - '0') * 10 + (strArray[1]+10)[0] - '0';

			if(curDay != lstDay){  
				//diff day so print out results unless first record
				if(lstDay > 0){
					
					tmp = atof(strArray[ColRainTot]);
					rain24tmp = tmp - rain24;
					if (rain24tmp < 0 )  {
						rain24tmp += rain24RolloverValue;
					}
					
					//padd with zero
					if ( lstDay < 10) printf("0"); 

					printf(LINE_ROW,
						lstDay,
						dayofweek(lstDate, lstDay),
						iT[HI], iT[LOW],iT[AVG]/ct,
						oT[HI], oT[LOW],oT[AVG]/ct,
						iH[HI], iH[LOW],iH[AVG]/ct,
						oH[HI], oH[LOW],oH[AVG]/ct,
						rain24tmp);

					 rain24 = tmp;

					 //rollover to next month
					 if (lstDay > curDay) printf(LINE_BREAK); 

				} else if (lstDay == 0) {
					//first record so get rain value 
					rain24 = atof(strArray[ColRainTot]);
				}

				reset(iT);
				reset(oT);
				reset(iH);
				reset(oH);
				ct = 0;
				lstDay = curDay;
				strncpy(lstDate,strArray[1],sizeof(lstDate));
				
				if (firstRead == 0) {
					printf("<B>wu %c%c%c-%c%c%c%c</B><br />\n",
						(lstDate[5]),(lstDate[6]),(lstDate[7]),
						(lstDate[0]),(lstDate[1]),(lstDate[2]),(lstDate[3]));
					firstRead = 1;
				}
			} 

			maxminavg(strArray[ColTempIn],iT,InvalidTempInMin,InvalidTempInMax);
			maxminavg(strArray[ColTempOut],oT,InvalidTempOutMin,InvalidTempOutMax);
			maxminavg(strArray[ColHumIn],iH,InvalidHumMin,InvalidHumMax);
			maxminavg(strArray[ColHumOut],oH,InvalidHumMin,InvalidHumMax);

			ct++;

		}
		fclose(fileptr);
		argcPos++;


	}

	if(lstDay > 0){
	tmp = atof(strArray[ColRainTot]);
	if ( lstDay < 10) printf("0"); 

		printf(LINE_ROW,
			lstDay,
			dayofweek(lstDate, lstDay) ,

			iT[HI], iT[LOW],iT[AVG]/ct,
			oT[HI], oT[LOW],oT[AVG]/ct,
			iH[HI], iH[LOW],iH[AVG]/ct,
			oH[HI], oH[LOW],oH[AVG]/ct,
			tmp - rain24);

	}

	//always print lastline
	printf(LINE_BREAK);


	return(0);
}

void maxminavg(char* s,long* v,long invalidMin, long invalidMax)
{
	long t = atof(s) * 10;
	if (t > invalidMin && t < invalidMax) {
		if (v[HI] < t) v[HI] = t;
		if (v[LOW] > t) v[LOW] = t;
		if (sizeof(v)>2) v[AVG] += t;
	}
}

void reset(long* v)
{
	v[HI] = -999;
	v[LOW] = 9999;
	if (sizeof(v)>2) v[AVG] =0;
}

char* dayofweek(char *datestring, int dayToUse) 
{
	static char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	static char days[][3] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
	static char year4[]="2014"; 
	static char month3[]="Jan";
	int y, m;
	
	//datestring: 2011-Dec-07
	strncpy(month3,&datestring[5],3);
	m = (strstr(months,month3) - months)/3 + 1;

	strncpy(year4,&datestring[0],4);
	y = atoi(year4) - (m < 3);

    return days[( y + y/4 - y/100 + y/400 + t[m-1] + dayToUse) % 7];
}
