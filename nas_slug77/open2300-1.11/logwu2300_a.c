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
#define PACHUBE_API_KEY ""
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
	char datestring[50];        //used to hold the date stamp for the log file
	const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE",
	                           "S","SSW","SW","WSW","W","WNW","NW","NNW"};
	double winddir[6];
	int tempint;
	char tendency[15];
	char forecast[15];
	struct config_type config;
	time_t basictime;

	get_configuration(&config, argv[2]);

	ws2300 = open_weatherstation(config.serial_device_name);

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

	sprintf(logline,"%.1f ", temperature_indoor(ws2300, config.temperature_conv));
	//sprintf(tempstring,"%.1f ", temperature_indoor(ws2300, config.temperature_conv)); 
	//strcat(logline, tempstring);
	
	/* READ TEMPERATURE OUTDOOR */

	sprintf(tempstring,"%.1f ", temperature_outdoor(ws2300, config.temperature_conv));
	strcat(logline, tempstring);

	/* READ DEWPOINT */

	sprintf(tempstring,"%.1f ", dewpoint(ws2300, config.temperature_conv));
	strcat(logline, tempstring);

	/* READ RELATIVE HUMIDITY INDOOR */

	sprintf(tempstring,"%d ", humidity_indoor(ws2300));	
	strcat(logline, tempstring);

	/* READ RELATIVE HUMIDITY OUTDOOR */

	sprintf(tempstring,"%d ", humidity_outdoor(ws2300));	 
	strcat(logline, tempstring);

	/* READ WIND SPEED AND DIRECTION */

	sprintf(tempstring,"%.1f ",
	       wind_all(ws2300, config.wind_speed_conv_factor, &tempint, winddir));
	strcat(logline, tempstring);

	sprintf(tempstring,"%.1f %s ", winddir[0], directions[tempint]);
	strcat(logline, tempstring);

	/* READ WINDCHILL */

	sprintf(tempstring,"%.1f ", windchill(ws2300, config.temperature_conv));
	strcat(logline, tempstring);

	/* READ RAIN 1H */

	sprintf(tempstring,"%.2f ", rain_1h(ws2300, config.rain_conv_factor));
	strcat(logline, tempstring);

	/* READ RAIN 24H */

	sprintf(tempstring,"%.2f ", rain_24h(ws2300, config.rain_conv_factor));
	strcat(logline, tempstring);

	/* READ RAIN TOTAL */

	sprintf(tempstring,"%.2f ", rain_total(ws2300, config.rain_conv_factor));
	strcat(logline, tempstring);


	/* READ RELATIVE PRESSURE */

	sprintf(tempstring,"%.3f ", rel_pressure(ws2300, config.pressure_conv_factor));
	strcat(logline, tempstring);

	/* READ TENDENCY AND FORECAST */

	tendency_forecast(ws2300, tendency, forecast);
	sprintf(tempstring,"%s %s ", tendency, forecast);
	strcat(logline, tempstring);

	/* GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE */

	time(&basictime);
	strftime(datestring, sizeof(datestring), "%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
	         localtime(&basictime));

	// Print out and leave

	// printf("%s %s\n",datestring, logline); //disabled to be used in cron job
	fprintf(fileptr, "%s %s\n", datestring, logline);
	
	fclose(fileptr);
	
	int x = 0;
   	x = wu( ws2300,argv[2]);
	close_weatherstation(ws2300);
	return(0);
}

/********** MAIN PROGRAM ************************************************
 *
 * This program reads all current weather data from a WS2300
 * and sends it to Weather Underground.
 *
 * It takes one parameter which is the config file name with path
 * If this parameter is omitted the program will look at the default paths
 * See the open2300.conf-dist file for info
 *
 ***********************************************************************/
int wu(WEATHERSTATION ws2300,char *path)
{
   //int argc, char *argv[])
	//WEATHERSTATION ws2300;
	struct config_type config;
	char urlline[3000] = "";
	char tempstring[1000] = "";

	char datestring[50];        //used to hold the date stamp for the log file
	double tempfloat;
	double tmpvalue;
	time_t basictime;
	int dataFlag = 1;
	int rtnArd = -1;
	
	get_configuration(&config, path);

	get_arduino();	

	//ws2300 = open_weatherstation(config.serial_device_name);


	/* START WITH URL, ID AND PASSWORD */

	sprintf(urlline, "GET %s?ID=%s&PASSWORD=%s", WEATHER_UNDERGROUND_PATH,
	        config.weather_underground_id,config.weather_underground_password);

	/* GET DATE AND TIME FOR URL */
	
	time(&basictime);
	basictime = basictime - atof(config.timezone) * 60 * 60;
	strftime(datestring,sizeof(datestring),"&dateutc=%Y-%m-%d+%H%%3A%M%%3A%S",
	         localtime(&basictime));
	strcat(urlline, datestring);

    if ( humidity_outdoor(ws2300) > 100) dataFlag = 0;
	if (dewpoint(ws2300, FAHRENHEIT) < -10.0) dataFlag = 0;
	if (dewpoint(ws2300, FAHRENHEIT) > 100.0) dataFlag = 0;

	//soiltempf - [F soil temperature] * for sensors 2,3,4 use soiltemp2f, soiltemp3f, and soiltemp4f 
		rtnArd = getValueFromKey(tempstring,"a2"); //"ea"
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
		strcat(urlline,"&soiltempf=");
		sprintf(tempstring, "%.1f",tempfloat);
		strcat(urlline, tempstring);
		//strcat(pachube, ","); skip , for first pachube input value
		strcat(pachube, tempstring);

	//soilmoisture - [%] * for sensors 2,3,4 use soilmoisture2, soilmoisture3, and soilmoisture4 
		//rtnArd = getValueFromKey(tempstring,"S0");
		//strcat(urlline,"&soilmoisture=");
		//sprintf(tempstring, "%s",tempfloat);
		//strcat(urlline, tempstring);
		//strcat(pachube, ",");
		//strcat(pachube, tempstring);
        //pond temp
        
		rtnArd = getValueFromKey(tempstring,"e6");  //"fd"
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
                sprintf(tempstring, "%.1f",tempfloat);
                strcat(pachube, ",");
                strcat(pachube, tempstring);

	//solarradiation - [W/m^2] 

	//UV - [index]

	/* READ TEMPERATURE INDOOR */
		strcat(urlline,"&indoortempf=");
		sprintf(tempstring, "%.2f", temperature_indoor(ws2300, FAHRENHEIT) );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	/* READ RELATIVE HUMIDITY INDOOR */

		strcat(urlline,"&indoorhumidity=");
		sprintf(tempstring, "%d", humidity_indoor(ws2300) );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	if (dataFlag) {
		/* READ TEMPERATURE OUTDOOR - deg F for Weather Underground */
		strcat(urlline,"&tempf=");
		sprintf(tempstring, "%.2f", temperature_outdoor(ws2300, FAHRENHEIT) );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

		/* READ DEWPOINT - deg F for Weather Underground*/
		strcat(urlline,"&dewptf=");
		sprintf(tempstring, "%.2f", dewpoint(ws2300, FAHRENHEIT) );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

		/* READ RELATIVE HUMIDITY OUTDOOR */
		strcat(urlline,"&humidity=");
		sprintf(tempstring, "%d", humidity_outdoor(ws2300) );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

		/* READ WIND SPEED AND DIRECTION - miles/hour for Weather Underground */

		tmpvalue = wind_current(ws2300, MILES_PER_HOUR, &tempfloat);
		if (tmpvalue>100.0) tmpvalue=0.0;
		strcat(urlline,"&windspeedmph=");
		sprintf(tempstring, "%.2f", tmpvalue );
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

		strcat(urlline,"&winddir=");
		sprintf(tempstring,"%.1f", tempfloat);
		strcat(urlline, tempstring);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

		/* READ WIND GUST - miles/hour for Weather Underground */

		if (GUST)
		{
			tmpvalue = wind_minmax(ws2300, MILES_PER_HOUR, NULL, NULL, NULL, NULL);
			if (tmpvalue>100.0) tmpvalue=0.0;
			strcat(urlline,"&windgustmph=");
			sprintf(tempstring, "%.2f",tmpvalue);
			strcat(urlline, tempstring);
			strcat(pachube, ",");
			strcat(pachube, tempstring);

		}
	} else {
		strcat(pachube, ",,,,,,");
	}

	/* READ RAIN 1H - inches for Weather Underground */
	strcat(urlline,"&rainin=");
	sprintf(tempstring, "%.2f", rain_1h(ws2300, INCHES) );
	strcat(urlline, tempstring);
	strcat(pachube, ",");
	strcat(pachube, tempstring);


	/* READ RAIN 24H - inches for Weather Underground */
	strcat(urlline,"&dailyrainin=");
	sprintf(tempstring, "%.2f", rain_24h(ws2300, INCHES) );
	strcat(urlline, tempstring);
	strcat(pachube, ",");
	strcat(pachube, tempstring);

	/* READ RELATIVE PRESSURE - Inches of Hg for Weather Underground */
	strcat(urlline,"&baromin=");
	sprintf(tempstring, "%.3f", rel_pressure(ws2300, INCHES_HG) );
	strcat(urlline, tempstring);

	sprintf(tempstring,"%.3f ", rel_pressure(ws2300, config.pressure_conv_factor));
	strcat(pachube, ",");
	strcat(pachube, tempstring);

	/* ADD SOFTWARE TYPE AND ACTION */
	sprintf(tempstring, "&softwaretype=open2300-%s&action=updateraw", VERSION);
	strcat(urlline, tempstring);
	
	sprintf(tempstring, " HTTP/1.0\r\nUser-Agent: open2300/%s\r\nAccept: */*\r\n"
	                   "Host: %s\r\nConnection: Keep-Alive\r\n\r\n",
	        VERSION, WEATHER_UNDERGROUND_BASEURL);
	strcat(urlline, tempstring);


	/* Reset minimum and maximum wind readings if reporting gusts */
	if (GUST)
	{
		wind_reset(ws2300, RESET_MIN + RESET_MAX);
	}


	/* SEND DATA TO WEATHER UNDERGROUND AS HTTP REQUEST */
	/* or print the URL if DEBUG is enabled in the top of this file */

	close_weatherstation(ws2300);

	if (DEBUG)
	{
		printf("%s\n",urlline);
	}
	else
	{
	 	if (dataFlag) 
		{
			http_request_url(urlline);
		}
	}
	
	//rain sensor
	rtnArd = getValueFromKey(tempstring,"R0");
	strcat(pachube, ",");
	strcat(pachube, tempstring);

	//Pyranometer
	rtnArd = getValueFromKey(tempstring,"P0");
	strcat(pachube, ",");
	strcat(pachube, tempstring);
	
	//basement temp
		rtnArd = getValueFromKey(tempstring,"a5");
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
		sprintf(tempstring, "%.1f",tempfloat);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	//hvac temp
		rtnArd = getValueFromKey(tempstring,"26"); //"98"
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
		sprintf(tempstring, "%.1f",tempfloat);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	//hvac temp return
		rtnArd = getValueFromKey(tempstring,"e4");
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
		sprintf(tempstring, "%.1f",tempfloat);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	//attic temp
		rtnArd = getValueFromKey(tempstring,"6a");
		tempfloat = atof(tempstring);
		tempfloat = tempfloat * 9.0/5.0 + 32.0;
		sprintf(tempstring, "%.1f",tempfloat);
		strcat(pachube, ",");
		strcat(pachube, tempstring);

	//KWh5mins
	rtnArd = getValueFromKey(tempstring,"K5");
	strcat(pachube, ",");
	strcat(pachube, tempstring);
	
        //Gas5mins
	rtnArd = getValueFromKey(tempstring,"G5");
	strcat(pachube, ",");
	strcat(pachube, tempstring);

        //Total KWH
	rtnArd = getValueFromKey(tempstring,"K0");
	strcat(pachube, ",");
	strcat(pachube, tempstring);

       //Total Gas
	rtnArd = getValueFromKey(tempstring,"G0");
	strcat(pachube, ",");
	strcat(pachube, tempstring);


	put_pachube();

	return(0);
}

int put_pachube() {

	int rtnValue;
	int content_length;
	char tempstring[1000] = "";
	char urlline[3000] = "";
	char rtnBuffer[3000] = "";

	content_length = strlen(pachube);

	sprintf(urlline, "PUT /api/%s", SHARE_FEED_ID);
	sprintf(tempstring,".csv HTTP/1.1\nHost: pachube.com\nX-PachubeApiKey: %s",PACHUBE_API_KEY);
	strcat(urlline, tempstring);

	strcat(urlline,"\nUser-Agent: Arduino");
	sprintf(tempstring,"\nContent-Type: text/csv\nContent-Length: %d\nConnection: close\n\n%s\n",content_length,pachube);
	strcat(urlline, tempstring);

	rtnValue = http_request_url_ard(urlline,"pachube.com",rtnBuffer);

	return(0);
}
int get_arduino(){
	char urlline[3000] = "";
	char tempstring[1000] = "";
	char rtnBuffer[3000] = "";
	int rtnValue;

	int flag = 0;
	int writeflag = -1;

	/* START WITH URL, ID AND PASSWORD */
	sprintf(urlline, "GET %s", ARDUINO_PATH);

	sprintf(tempstring, " HTTP/1.0\r\nUser-Agent: open2300/%s\r\nAccept: */*\r\n"
						"Host: %s\r\nConnection: Keep-Alive\r\n\r\n",
	        VERSION, ARDUINO_BASEURL);
	strcat(urlline, tempstring);

	if (DEBUG)
	{
		printf("%s\n",urlline);
		rtnValue = http_request_url_ard(urlline,ARDUINO_BASEURL,rtnBuffer);
		printf("%s\n",rtnBuffer);
		printf("rtn code %i",rtnValue);
	}
	else
	{
		rtnValue = http_request_url_ard(urlline,ARDUINO_BASEURL,rtnBuffer);
	}
	
	//clean return values for logline

	do {
		if ( rtnBuffer[flag] == 0) 
				flag = -2;
		else if ( rtnBuffer[flag] == ':')
			writeflag = 0;
		else if (writeflag > -1 && rtnBuffer[flag] > 31 && rtnBuffer[flag] < 127)
            ardBuffer[writeflag++] = rtnBuffer[flag];

			flag++;
	} while (flag > -1);

	return(0);
}
int getValueFromKey(char *valueFromKey,char *key)
{
	int flag = 0;
	int writeflag = -1;

	do {
		if ( ardBuffer[flag] == 0) 
				flag = -2;
		else if ( ardBuffer[flag] == '[' && writeflag == -1 ){
			if (ardBuffer[flag+1] == key[0] && ardBuffer[flag+2] == key[1]) {
				writeflag = 0;
				flag += 3;
			}
		} else if ( ardBuffer[flag] == '[' && writeflag > -1){
			flag = -2;
		} else if (writeflag > -1 && ardBuffer[flag] > 31 && ardBuffer[flag] < 127)
            valueFromKey[writeflag++] = ardBuffer[flag];

			flag++;
	} while (flag > -1);
	
	flag = writeflag;
	do{
		valueFromKey[flag++] = '\0';
	} while (flag < 25);
	
	return writeflag;
}
int http_request_url_ard(char *urlline,char *host_baseurl,char *rtnBuffer)
{
	int sockfd;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	//char rtnBuffer[3000]="";
    char tempstring[1024] = "";

	int bytes_read;
	
	if ( (hostinfo = gethostbyname(host_baseurl)) == NULL )
	{
		perror("Host not known by DNS server or DNS server not working");
		strcat(rtnBuffer, "Host not known by DNS server or DNS server not working");
		return(-1);
	}

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Cannot open socket");
		strcat(rtnBuffer, "Cannot open socket");
		return(-1);
	}

	memset(&urladdress, 0, sizeof(urladdress));
	urladdress.sin_family = AF_INET;
	urladdress.sin_port = htons(80); /*default HTTP Server port */

	urladdress.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

	if (connect(sockfd,(struct sockaddr*)&urladdress,sizeof(urladdress)) != 0)
	{
		perror("Cannot connect to host");
		strcat(rtnBuffer, "Cannot connect to host");
		return(-1);
	}
	
	sprintf(buffer, "%s", urlline);
	send(sockfd, buffer, strlen(buffer), 0);

	/* While there's data, read and print it */
	do
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if ( bytes_read > 0 ){
			if (DEBUG) printf("%s", buffer);
			sprintf(tempstring,"%s",buffer);
			strcat(rtnBuffer, tempstring);
		}
	}
	while ( bytes_read > 0 );

	/* Close socket and clean up winsock */
	close(sockfd);
	
	return(0);
}
