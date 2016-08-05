#include <stdio.h>
    #include <time.h>
 
    int main()
    {
        time_t tim = time(NULL);
	char *data;

        printf("Content-type: text/html\n"   /* Necessary to specify the type */
	       "\n"                          /* This blank line is critical! */
	       "<html>\n"
	       "<body>\n"
	       "Hello, World!<br>\n");       /* Do the hello thing... */

		data = getenv("QUERY_STRING");
		if(data == NULL)
			printf("<P>Error! Error in passing data from form to script.");
		else
			printf(data);


        /* Print out the current time */
        printf("The time is %s<br>\n", asctime(localtime(&tim)) );

	printf("</body>\n"
	       "</html>\n");

	return 0;
    } 
