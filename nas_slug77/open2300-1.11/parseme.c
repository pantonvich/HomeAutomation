
#define DEBUG 1 
// cmdarg: K0 3.0f 7.0f 0 0 7.2f C:\personal\Logs\ard-201109.log C:\personal\Logs\ard-201110.log
// gcc -Wall -O3 parseme.c -o parseme
// cp parseme /opt/bin/

//#include "win2300.h"
#include <string.h>
//#include <fcntl.h>
#include <stdio.h>
//#include <time.h>
#include <stdlib.h>
//#include <stdbool.h>
#include <math.h>
//#include <sys/types.h>
//#include <sys/stat.h>

//e4 hvac return, 98 hvac out,
//6a attic,  a5 basement, 
//ea soil, fd pond

#define HourlyStep 3
#define EndLine "<br />\n"

void DisplayHourly();
void DisplaySubTotalAverage();
void print_arr(int start,int end, const char* format, long* arr);

char* dayofweekInt(int m, int d, int y) ;

long hourlyArr[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
long hourlyCountArr[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
long subTotalArr[]={0,0,0,0,0,0,0,0};
int subTotalCountArr[]={0,0,0,0,0,0,0,0};
float subLowArr[]={100,100,100,100,100,100,100,100};
float subHighArr[]={-100,-100,-100,-100,-100,-100,-100,-100};

long totalMonth=0;
long totalDay=0;
int totalDayCount=0;


float dayMonthHigh=-99999.0;
float dayMonthLow=99999.0;

int curDay = 0;
int curMonDay = 0;
int curYear;
long rangeStart;
long rangeEnd;
float rangeSum = 0.0;

int isTemperature = 0;
int smallerDivider=1;

float fToC_Multipler = 9.0/5.0;
int fToC_Offset = 32;

char *searchTag;
char FormatItem[10];
char FormatHead[10];
char FormatSum[10];
int resetCt = 0;
int resetCtLast = 0;
long blCt=0;
long blCtLast=0;

void print_usage(void)
{
	printf("\n");
	printf("parseme - Read and interpret data\n");
	printf("arg1 tag that has value [K0] \n");
	printf("arg2 format item 3.0f \n");
	printf("arg3 format head 3.0f \n");
	printf("arg(*) filepath name.\n");
	printf("\n");
	exit(0);
}



int main(int argc, char *argv[])
{


	FILE *fileptr;
	char str[3000];
	int argcPos=7;
	char mon_pos[]="JanFebMarAprMayJunJulAugSepOctNovDec";

	if (argc < 4 ) //|| argc > 3)
	{
		print_usage();
	}

	//[K0]
	searchTag = argv[1];



	strcpy(FormatItem," %");
	strcat(FormatItem,argv[2]);

	strcpy(FormatHead," %");
	strcat(FormatHead,argv[3]);

	rangeStart = strtol(argv[4],NULL,0);
	rangeEnd = strtol(argv[5],NULL,0);

	strcpy(FormatSum," %");
	strcat(FormatSum,argv[6]);

	while(argcPos < argc)
	{
		/* Get log filename. */
		fileptr = fopen(argv[argcPos], "r");
		if (fileptr == NULL)
		{
			printf("Cannot open file %s\n",argv[argcPos]);
			exit(-1);
		}

		{
			char *foundPos;
			long rtnValue;
			long lastRtnValue = 0;

			long blValue=0;
			
			int curHr = 0;
			char * curMonPt;
			char curMonChar[4];

			int lastDay = 0;
			int firstRead = 1;

			//printf("<b>%s</b>%s",searchTag,EndLine);

			while(! feof(fileptr))
			{
				fscanf(fileptr, "\n%[^\n]", str);

				foundPos = strstr(str,"-");
				if (foundPos)
				{
					if ((foundPos-5)[0] == ' ')
					{
						//2011-Dec-07 01: foundPos is -Dec-07 01:
						curDay = ((foundPos+5)[0] - '0') * 10 + (foundPos+6)[0] - '0';
						curHr = ((foundPos+8)[0] - '0') * 10 + (foundPos+9)[0] - '0';

						curYear = ((foundPos-4)[0] - '0') * 1000 
							+ ((foundPos-3)[0] - '0') * 100 
							+ ((foundPos-2)[0] - '0') * 10 
							+ ((foundPos-1)[0] - '0');

					}
					else if ((foundPos-3)[0] == ' ')
					{	
						//30-Mar-2013 20: : foundPos is -Mar-2013 20:
						curDay = ((foundPos-2)[0] - '0') * 10 + (foundPos-1)[0] - '0';
						curHr = ((foundPos+10)[0] - '0') * 10 + (foundPos+11)[0] - '0';
						curYear = ((foundPos+5)[0] - '0') * 1000 
							+ ((foundPos+6)[0] - '0') * 100 
							+ ((foundPos+7)[0] - '0') * 10 
							+ ((foundPos+8)[0] - '0');
					}


					if (firstRead == 1)
					{
						curMonChar[0]=(foundPos+1)[0];
						curMonChar[1]=(foundPos+2)[0];
						curMonChar[2]=(foundPos+3)[0];
						curMonChar[3]='\0';
						printf("<b>%s %s-%i</b>%s",searchTag,curMonChar,curYear,EndLine);
					}
				}

				if (totalDay > 1000) smallerDivider=100;

				if (firstRead == 0 && lastDay != curDay)
				{
					curMonChar[0]=(foundPos+1)[0];
					curMonChar[1]=(foundPos+2)[0];
					curMonChar[2]=(foundPos+3)[0];
					curMonChar[3]='\0';
					curMonPt=strstr(mon_pos,curMonChar);
					if (curMonPt != NULL)
					{
						curMonDay = ( curMonPt-mon_pos+3)/3 * 100 + curDay;
					}



					if (lastDay != 0) DisplayHourly();

					//foundPos = strstr(str,"-");	
					//next print date
					if ((foundPos-5)[0] == ' ')
						printf("%.*s %s ", 2, foundPos +5, dayofweekInt(curMonDay/100,curDay,curYear));
					else if ((foundPos-3)[0] == ' ')
						printf("%.*s %s ", 2, foundPos - 2, dayofweekInt(curMonDay/100,curDay,curYear));

					lastDay = curDay;
				}

				//find Tag i.e. [K0]
				foundPos = strstr(str,searchTag);
				if (foundPos)
				{
					//skip over or start after tag sizeof [K0] = 4
					foundPos += sizeof(searchTag);
					rtnValue = 0;

					//only want numbers everything else is the end...
					//strip off leading zeros "00012"
					while(((foundPos[0] >= '0' && foundPos[0] <= '9') || foundPos[0] == '.' ) )  
					{
						if (foundPos[0] != '.')
							rtnValue = rtnValue * 10 + foundPos[0] - '0';
						else
							isTemperature = 1;
						foundPos++;
					} 

					if (firstRead)
					{
						lastRtnValue=rtnValue;
						firstRead = 0;
						if (isTemperature == 0)
						{
							fToC_Multipler = 1;
							fToC_Offset = 0;
						}
					}
					else if (curHr >-1 && curHr<24)
					{
						if (isTemperature == 1)
						{
							hourlyArr[curHr] +=  rtnValue;
							hourlyCountArr[curHr] += 1;
							totalDay += rtnValue;
							totalDayCount++;
							
						}
						else if (rtnValue == lastRtnValue)
						{
							//printf("");
						} 
						else if (rtnValue < lastRtnValue)
						{
							hourlyArr[curHr] +=  rtnValue;
							totalDay += rtnValue;
							resetCt++;
						} 
						else 
						{
							hourlyArr[curHr] +=  rtnValue - lastRtnValue;
							totalDay += rtnValue - lastRtnValue;
						}
					}

					lastRtnValue=rtnValue;
				}	
				
				if (strcmp(searchTag,"K0")==0) {
					foundPos = strstr(str, "[BL]");
					if (foundPos)
					{
						//skip over or start after tag sizeof [K0] = 4
						foundPos += sizeof(searchTag);
						blValue = 0;

						//only want numbers everything else is the end...
						//strip off leading zeros "00012"
						while ((foundPos[0] >= '0' && foundPos[0] <= '9'))
						{
							blValue = blValue * 10 + foundPos[0] - '0';
							foundPos++;
						}

						if (blValue < 85 || blValue > 95) blCt++;
					}
				}				
			}

			DisplayHourly();
			DisplaySubTotalAverage();

			printf(EndLine);
		}
		fclose(fileptr);
		argcPos++;
	}

	return(0);
}

void DisplayHourly()
{
	int i; int y;
	int x=0;

	float totalDayTmp;
    long curYearMonDay;
	
	if (isTemperature == 1)
		totalDayTmp=(float)totalDay/totalDayCount/100 * fToC_Multipler + fToC_Offset;
	else
	{
		totalDayTmp=(float)totalDay/smallerDivider;
		
		curYearMonDay = curYear * 10000 + curMonDay;
		
		if ((curMonDay>rangeStart && curMonDay<=rangeEnd) || (curYearMonDay>rangeStart && curYearMonDay<=rangeEnd))
			rangeSum += totalDayTmp;

		if (rangeStart>0) printf("(%3.0f)",rangeSum);
	}

	printf(FormatHead,totalDayTmp);

	if(totalDayTmp>dayMonthHigh) dayMonthHigh = totalDayTmp;
	if(totalDayTmp<dayMonthLow) dayMonthLow = totalDayTmp;

	for(i=0;i<24;i+=HourlyStep)
	{
		long subTotal = 0; long scount = 0; float value;

		for(y=0;y<HourlyStep;y++)
		{
			subTotal += hourlyArr[i+y];
			hourlyArr[i+y] = 0;

			scount += hourlyCountArr[i+y];
			hourlyCountArr[i+y] = 0;
		}

		if (isTemperature == 0) scount = 1;

		value = (float)subTotal/scount/smallerDivider;
		printf(FormatItem, value  * fToC_Multipler + fToC_Offset);

		if (subHighArr[x]<value) 
			subHighArr[x] = value;

		if (subLowArr[x]>value) 
			subLowArr[x] = value;

		subTotalCountArr[x]+=scount;
		subTotalArr[x]+=subTotal;
		x++;
	}

	//if float then the count - if not then 1
	//subTotalCount++;
	totalDay=0;
	totalDayCount=0;
	
	printf(" { %i",resetCt - resetCtLast);
	resetCtLast = resetCt;
	
	printf(" %ld }", blCt - blCtLast);
	blCtLast = blCt;

	printf(EndLine);
}

void DisplaySubTotalAverage()
{
	int i;
	int sumMonth=0;
	int sumMonthCount=0;
	//float value;  

	printf("%s%s",EndLine,"A ");

	for(i=0;i<24/HourlyStep;i++)
	{
		sumMonth+=subTotalArr[i];
		//sumMonthCount+=subTotalCountArr[i];
	}

	sumMonthCount=curDay;

	printf(FormatHead,(float)sumMonth/sumMonthCount/smallerDivider * fToC_Multipler + fToC_Offset);

	for(i=0;i<24/HourlyStep;i++)
		printf(FormatHead, (float)subTotalArr[i]/subTotalCountArr[i]/smallerDivider * fToC_Multipler + fToC_Offset);

	if (isTemperature == 0)
	{
		printf("%s%s",EndLine,"T ");
		printf(FormatHead, (float)sumMonth/smallerDivider);

		for(i=0;i<24/HourlyStep;i++)
			printf(FormatHead, (float)subTotalArr[i]/smallerDivider);
	}

	printf("%s%s",EndLine,"H ");

	//value=-100;
	//for(i=0;i<24/HourlyStep;i++)
	//	if (subHighArr[i]>value) value=subHighArr[i];

	//printf(FormatSum, value * fToC_Multipler + fToC_Offset );
	printf(FormatHead,dayMonthHigh);

	for(i=0;i<24/HourlyStep;i++)
		printf(FormatHead, subHighArr[i]  * fToC_Multipler + fToC_Offset );

	printf("%s%s",EndLine,"L ");
	//value=100;
	//for(i=0;i<24/HourlyStep;i++)
		//if (subLowArr[i]<value) value=subLowArr[i];

	//printf(FormatSum, value  * fToC_Multipler + fToC_Offset);
	printf(FormatHead,dayMonthLow);

	for(i=0;i<24/HourlyStep;i++)
		printf(FormatHead, subLowArr[i]  * fToC_Multipler + fToC_Offset);

	for(i=0;i<24/HourlyStep;i++)
	{
		subTotalArr[i] = 0;
		subTotalCountArr[i]=0;
		subHighArr[i]=-100;
		subLowArr[i]=100;
	}		

	totalMonth=0;
	dayMonthHigh = -99999.0;
	dayMonthLow = 99999.0;
	printf(EndLine);

	if (rangeStart>0)
	{
	printf("%ld to %ld = ", rangeStart,rangeEnd);
	printf(FormatSum,rangeSum);
	printf(EndLine);
	
	}
}

char* dayofweekInt(int m, int d, int y) 
{
	static char days[][3] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
	static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

	y -= m < 3;

    return days[( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7];
}
