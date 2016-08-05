//#include "rw2300.h"

//LINUX 
#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>

//COM
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG 0
#define ARDUINO_BASEURL "192.168.0.177" // "weatherstation.wunderground.com"
#define ARDUINO_PATH "" // "/weatherstation/updateweatherstation.php"
#define VERSION ".01"
#define PACHUBE_API_KEY ""
#define SHARE_FEED_ID "6449"


void print_usage(void)
{
	printf("\n");
	printf("arduino - Read and interpret data\n");
	printf("and write it to a log file. Perfect for a cron driven task.\n");
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("Save current data to logfile:    arduino filename config_filename\n");
	exit(0);
}

int main(int argc, char *argv[])
{

	FILE *fileptr;
	char logline[3000] = "";

	//struct config_type config;

	char urlline[3000] = "";
	char tempstring[1000] = "";
	char datestring[50];        //used to hold the date stamp for the log file
	double tempfloat;
	time_t basictime;
	char rtnBuffer[3000] = "";
	int rtnValue;

	int flag = 0;
	int writeflag = -1;

	//get_configuration(&config, argv[1]);
	
	/* Get log filename. */

	if (argc < 1 || argc > 3)
	{
		print_usage();
	}

	fileptr = fopen(argv[1], "a+");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(-1);
	}


	/* START WITH URL, ID AND PASSWORD */
	sprintf(urlline, "GET %s", ARDUINO_PATH);

	sprintf(tempstring, " HTTP/1.0\r\nUser-Agent: open2300/%s\r\nAccept: */*\r\n"
						"Host: %s\r\nConnection: Keep-Alive\r\n\r\n",
	        VERSION, ARDUINO_BASEURL);
	strcat(urlline, tempstring);


	if (DEBUG)
	{
		printf("%s\n",urlline);
		rtnValue = http_request_url(urlline,ARDUINO_BASEURL,rtnBuffer);
		printf("%s\n",rtnBuffer);
		printf("rtn code %i",rtnValue);
	}
	else
	{
		rtnValue = http_request_url(urlline,ARDUINO_BASEURL,rtnBuffer);
	}
	
	//clean return values for logline

	do {
		if ( rtnBuffer[flag] == 0) 
				flag = -2;
		else if ( rtnBuffer[flag] == ':')
			writeflag = 0;
		else if (writeflag > -1 && rtnBuffer[flag] > 31 && rtnBuffer[flag] < 127)
            logline[writeflag++] = rtnBuffer[flag];

			flag++;
	} while (flag > -1);

	//get time
	time(&basictime);
	strftime(datestring, sizeof(datestring), "%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
    localtime(&basictime));

	/* GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE */

	time(&basictime);
	strftime(datestring, sizeof(datestring), "%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
	         localtime(&basictime));

	// Print out and leave
	fprintf(fileptr, "%i %s %s\n", rtnValue, datestring, logline);

	fclose(fileptr);

	pachube(logline);

	return(0);
}

int pachube(char *logline)
{
int content_length;
	char tempstring[1000] = "";
	char urlline[3000] = "";
    char pachube_data[1000];
	char rtnBuffer[3000];
	int rtnValue;
	char value[10];
	
	getValueFromKey(value, "a2",logline);
	sprintf(pachube_data,"%s",value);
	getValueFromKey(value, "S0",logline);
	sprintf(tempstring,",%s",value);
	strcat(pachube_data, tempstring);

	content_length = strlen(pachube_data);

	sprintf(urlline, "PUT /api/%s", SHARE_FEED_ID);
	sprintf(tempstring,".csv HTTP/1.1\nHost: pachube.com\nX-PachubeApiKey: %s",PACHUBE_API_KEY);
	strcat(urlline, tempstring);

	strcat(urlline,"\nUser-Agent: Arduino");
	sprintf(tempstring,"\nContent-Type: text/csv\nContent-Length: %d",content_length);
	strcat(urlline, tempstring);
	sprintf(tempstring,"\nConnection: close\n\n%s\n",pachube_data);
	strcat(urlline, tempstring);

	rtnValue = http_request_url(urlline,"pachube.com",rtnBuffer);

	return(0);
}
int getValueFromKey(char *valueFromKey, char *key, char *logline)
{
	int flag = 0;
	int writeflag = -1;
	//valueFromKey = "";
	do {
		if ( logline[flag] == 0) 
				flag = -2;
		else if ( logline[flag] == '[' && writeflag == -1 ){
			if (logline[flag+1] == key[0] && logline[flag+2] == key[1]) {
				writeflag = 0;
				flag += 3;
			}
		} else if ( logline[flag] == '[' && writeflag > -1){
			flag = -2;
		} else if (writeflag > -1 && logline[flag] > 31 && logline[flag] < 127)
            valueFromKey[writeflag++] = logline[flag];

			flag++;
	} while (flag > -1);
	flag = writeflag;
	do{
		valueFromKey[flag++] = '\0';
	} while (flag < 25);

	return writeflag;
}
int http_request_url(char *urlline, char *host_baseurl, char *rtnBuffer)
{
	int sockfd;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	int bytes_read;
	char tempstring[1024] = "";

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
		if ( bytes_read > 0 ) {
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