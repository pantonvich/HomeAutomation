/*  open2300 - gust2300.c
 *  
 *  Version 1.5
 *  
 *  Control WS2300 weather station
 *  
 *  Copyright 2003,2004, Kenneth Lavrsen
 *  This program is published under the GNU General Public license
 */

#include "rw2300.h"

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
	printf("gust2300 - Read max wind data from WS-2300 weather station\n");
	printf("and write it to a log file. Reset max wind.\n");
	printf("Version %s (C)2003-2004 Kenneth Lavrsen.\n", VERSION);
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("Save gust data to logfile:    gust2300 filename config_filename\n");
	exit(0);
}
 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads the max wind data from a WS2300
 * and writes the data to a log file.
 *
 * Log file format:
 * Timestamp Date Time Gust
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
	struct config_type config;
	double tempfloat_max;
	struct timestamp time_max;
	
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


	/* READ MAX WIND */

	//Get Windspeed max
	wind_minmax(ws2300, config.wind_speed_conv_factor, NULL,
	            &tempfloat_max, NULL, &time_max);
	
	fprintf(fileptr, "%02d/%02d/%04d %02d:%02d",
			time_max.month, time_max.day, time_max.year, time_max.hour, time_max.minute);
	fprintf(fileptr, " %.1f\n", tempfloat_max);

	
	/* RESET MAX WIND */
	
	wind_reset(ws2300, RESET_MAX);

	close_weatherstation(ws2300);
	
	fclose(fileptr);

	exit(0);
}

