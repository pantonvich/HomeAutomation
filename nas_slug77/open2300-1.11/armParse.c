#include <string.h>
//#include <fcntl.h>
#include <stdio.h>
//#include <time.h>
#include <stdlib.h>
//#include <stdbool.h>
#include <math.h>

#define OutputLine "%s<br />\n"

int main(int argc, char *argv[])
{
	FILE *fileptr;
	char str[3000];
	char *foundPos;
	char lastPos[50] = "";
	int printLine = 0;

	if (argc < 1 ) //|| argc > 3)
	{
		printf(OutputLine,"No file specified");
	}
	/* Get log filename. */
	//fileptr = fopen( "C:\\Users\\pantonvi\\Documents\\personal\\Arduino\\Logs\\arm-201407.log" , "r");
	fileptr = fopen(argv[1], "r");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(-1);
	}

	while(! feof(fileptr))
	{
		fscanf(fileptr, "\n%[^\n]", str);

		foundPos = strstr(str,"|");
		printLine = 1;
		if (foundPos)
		{				
			if (strcmp(foundPos,lastPos) == 0) printLine = 0; //skip printing line
			strcpy(lastPos,foundPos);
		}

		if (printLine == 1) printf(OutputLine, str);
	}

	//always print lastline
	if (str != NULL && printLine == 0) printf(OutputLine, str);
    return (0);
}

