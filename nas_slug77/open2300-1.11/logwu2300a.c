/*  open2300 - log2300.c
 *  
 *  Version 1.11
 *  
 *  Control WS2300 weather station
 *  
 *  Copyright 2003-2006, Kenneth Lavrsen
 *  This program is published under the GNU General Public license
 */

#include "rw2300.h"
#define DEBUG 0  // wu2300 stops writing to standard out if setting this to 0
#define GUST  1  // report wind gust information (resets wind min/max)

#define ARDUINO_BASEURL "192.168.0.177" //"weatherstation.wunderground.com"
#define ARDUINO_PATH "" ///raw" //"/weatherstation/updateweatherstation.php"
#define PACHUBE_API_KEY "2fb084f01917313ef0494f87c5b03a5269f68f50dc06c71f874d94f0450cd631"
#define PACHUBE_BASEURL "api.pachube.com/v2/feeds/6449.csv"
#define SHARE_FEED_ID "6449"

char ardBuffer[3000] = "";
char pachube[1050] = "";

int http_request_url_ard(char *urlline,char *host_baseurl,char *rtnBuffer);
int getValueFromKey(char *valueFromKey,char *key);
int get_arduino();
int put_pachube();
int wu(WEATHERSTATION ws2300,char *path);

/********************************************************************
 * print_usage prints a short user guide
 *
 * Input:   none
 * 
 * Output:  prints to stdout
 * 
 * Returns: exits program
 *
 ********************************************************************/
void print_usage(void)
{
	printf("\n");
	printf("log2300 - Read and interpret data from WS-2300 weather station\n");
	printf("and write it to a log file. Perfect for a cron driven task.\n");
	printf("Version %s (C)2003-2006 Kenneth Lavrsen.\n", VERSION);
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("Save current data to logfile:    log2300 filename config_filename\n");
	exit(0);
}
 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads current weather data from a WS2300
 * and writes the data to a log file.
 *
 * Log file format:
 * Timestamp Date Time Ti To DP RHi RHo Wind Dir-degree Dir-text WC
 *              Rain1h Rain24h Rain-tot Rel-Press Tendency Forecast
 *
 * Just run the program without parameters for usage.
 *
 * It takes two parameters. The first is the log filename with path
 * The second is the config file name with path
 * If this parameter is omitted the program will look at the default paths
 * See the open2300.conf-dist file for info
 *
 ***********************************************************************/
int main(int argc, char *argv[])
{
	WEATHERSTATION ws2300;
	FILE *fileptr;
	char tempstring[1000] = "";
	char logline[1000] = "";
    char urlline[3000] = "";
	char pachubeline[2000] = "";
	char datestring[50];        //used to hold the date stamp for the log file
	const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE",
	                           "S","SSW","SW","WSW","W","WNW","NW","NNW"};
	double winddir[6];
	int tempint;
	char tendency[15];
	char forecast[15];
	struct config_type config;
	time_t basictime;
	int dataFlag = 1;

	get_configuration(&config, argv[2]);

	ws2300 = open_weatherstation(config.serial_device_name);
	
	/* START WITH URL, ID AND PASSWORD */

	sprintf(urlline, "GET %s?ID=%s&PASSWORD=%s", WEATHER_UNDERGROUND_PATH,
	        config.weather_underground_id,config.weather_underground_password);

			
	/* GET DATE AND TIME FOR URL */
	
	time(&basictime);
	basictime = basictime - atof(config.timezone) * 60 * 60;
	strftime(datestring,sizeof(datestring),"&dateutc=%Y-%m-%d+%H%%3A%M%%3A%S",
	         localtime(&basictime));
	strcat(urlline, datestring);
	
	/* Get log filename. */

	if (argc < 2 || argc > 3)
	{
		print_usage();
	}

	fileptr = fopen(argv[1], "a+");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(-1);
	}


	/* READ TEMPERATURE INDOOR */

	//sprintf(logline,"%.1f ", temperature_indoor(ws2300, config.temperature_conv));
	sprintf(tempstring,"%.1f", temperature_indoor(ws2300, config.temperature_conv)); 
	strcat(logline, tempstring);
	
	strcat(pachubeline,"2,");
	strcat(pachubeline,tempstring);
	
	double outdoor = humidity_outdoor(ws2300);
	double dew = dewpoint(ws2300, config.temperature_conv);

    if ( outdoor > 100) dataFlag = 0;
	if (dew < -10.0) dataFlag = 0;
	if (dew > 100.0) dataFlag = 0;
	
	/* READ TEMPERATURE OUTDOOR */

		//sprintf(tempstring,"%.1f ", temperature_outdoor(ws2300, config.temperature_conv));
		sprintf(tempstring,"%.1f", outdoor);
		strcat(logline, " ");
		strcat(logline, tempstring);
		
		if (dataFlag) {
			strcat(pachubeline,"\r\n4,");
			strcat(pachubeline,tempstring);
		
			strcat(urlline,"&tempf=");
			strcat(urlline, tempstring);
		}

		/* READ DEWPOINT */

		//sprintf(tempstring,"%.1f ", dewpoint(ws2300, config.temperature_conv));
		sprintf(tempstring,"%.1f", dew);
		strcat(logline, " ");
		strcat(logline, tempstring);
		if (dataFlag) {
			strcat(pachubeline,"\r\n5,");
			strcat(pachubeline,tempstring);
			
			strcat(urlline,"&dewptf=");
			strcat(urlline, tempstring);
		}
	
	/* READ RELATIVE HUMIDITY INDOOR */

	sprintf(tempstring,"%d", humidity_indoor(ws2300));	
	strcat(logline, " ");
	strcat(logline, tempstring);
	
	strcat(pachubeline,"\r\n3,");
	strcat(pachubeline,tempstring);

	/* READ RELATIVE HUMIDITY OUTDOOR */

	sprintf(tempstring,"%d", humidity_outdoor(ws2300));	 
	strcat(logline, " ");
	strcat(logline, tempstring);
	if (dataFlag) {
		strcat(pachubeline,"\r\n6,");
		strcat(pachubeline,tempstring);
			
		strcat(urlline,"&humidity=");
		strcat(urlline, tempstring);
	}
		
	/* READ WIND SPEED AND DIRECTION */

	sprintf(tempstring,"%.1f",
	       wind_all(ws2300, config.wind_speed_conv_factor, &tempint, winddir));
	
	strcat(logline, " ");
	strcat(logline, tempstring);
	if (dataFlag) {
		strcat(pachubeline,"\r\n7,");
		strcat(pachubeline,tempstring);
			
		strcat(urlline,"&windspeedmph=");
		strcat(urlline, tempstring);
	}
	
	sprintf(tempstring,"%.1f %s ", winddir[0], directions[tempint]);
	strcat(logline, tempstring);

	sprintf(tempstring,"%.1f", winddir[0]);
	strcat(pachubeline,"\r\n8,");
	strcat(pachubeline,tempstring);
			
	/* READ WIND GUST - miles/hour for Weather Underground */

	if (GUST)
	{
		tmpvalue = wind_minmax(ws2300, config.wind_speed_conv_factor, NULL, NULL, NULL, NULL);
		if (tmpvalue>100.0) tmpvalue=0.0;
		sprintf(tempstring, "%.1f",tmpvalue);
		strcat(logline, " ");
		strcat(logline, tempstring);
		
		strcat(pachubeline,"\r\n9,");
		strcat(pachubeline,tempstring);
	
		strcat(urlline,"&windgustmph=");
		strcat(urlline, tempstring);
		
		wind_reset(ws2300, RESET_MIN + RESET_MAX);
	}
	
	/* READ WINDCHILL */

	sprintf(tempstring,"%.1f ", windchill(ws2300, config.temperature_conv));
	strcat(logline, tempstring);

	/* READ RAIN 1H */

	sprintf(tempstring,"%.2f", rain_1h(ws2300, config.rain_conv_factor));
	strcat(logline, " ");
	strcat(logline, tempstring);
	
	strcat(pachubeline,"\r\n10,");
	strcat(pachubeline,tempstring);
	
	strcat(urlline,"&rainin=");
	strcat(urlline, tempstring);	

	/* READ RAIN 24H */

	sprintf(tempstring,"%.2f", rain_24h(ws2300, config.rain_conv_factor));
	strcat(logline, " ");
	strcat(logline, tempstring);
	
	strcat(pachubeline,"\r\n11,");
	strcat(pachubeline,tempstring);
	
	strcat(urlline,"&dailyrainin=");
	strcat(urlline, tempstring);	

	/* READ RAIN TOTAL */

	sprintf(tempstring,"%.2f ", rain_total(ws2300, config.rain_conv_factor));
	strcat(logline, tempstring);


	/* READ RELATIVE PRESSURE */

	sprintf(tempstring,"%.3f", rel_pressure(ws2300, config.pressure_conv_factor));
	strcat(logline, " ");
	strcat(logline, tempstring);
	
	strcat(pachubeline,"\r\n12,");
	strcat(pachubeline,tempstring);
	
	
	sprintf(tempstring, "%.3f", rel_pressure(ws2300, INCHES_HG) );
	strcat(urlline,"&baromin=");
	strcat(urlline, tempstring);

	/* READ TENDENCY AND FORECAST */

	tendency_forecast(ws2300, tendency, forecast);
	sprintf(tempstring,"%s %s ", tendency, forecast);
	strcat(logline, tempstring);

	/* GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE */

	//time(&basictime);
	strftime(datestring, sizeof(datestring), "%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
	         localtime(&basictime));

	// Print out and leave

	// printf("%s %s\n",datestring, logline); //disabled to be used in cron job
	fprintf(fileptr, "%s %s\n", datestring, logline);
	
	fclose(fileptr);
	
	/* ADD SOFTWARE TYPE AND ACTION */
	sprintf(tempstring, "&softwaretype=open2300-%s&action=updateraw", VERSION);
	strcat(urlline, tempstring);
	
	sprintf(tempstring, " HTTP/1.0\r\nUser-Agent: open2300/%s\r\nAccept: */*\r\n"
	                   "Host: %s\r\nConnection: Keep-Alive\r\n\r\n",
	        VERSION, WEATHER_UNDERGROUND_BASEURL);
	strcat(urlline, tempstring);

	
	close_weatherstation(ws2300);
	
	if (DEBUG)
	{
		printf("%s\n",logline);
		printf("%s\n",urlline);
		printf("%s\n",pachubeline);
	}
	else
	{
	 	if (dataFlag) 
		{
			http_request_url(urlline);
		}
	}
	
	return(0);
}
