#include <stdio.h>
#include <time.h>
#include <stdio.h>

    int main()
    {
    time_t tim = time(NULL);
	char *data;
	long m,n;
	char filename = "/etc/ard-$(date +%Y%m).log";
	
        printf("Content-type: text/html\n"   /* Necessary to specify the type */
	       "\n"                          /* This blank line is critical! */
	       "<html>\n"
	       "<body>\n"
	       "Hello, World!<br>\n");       /* Do the hello thing... */

		data = getenv("QUERY_STRING");
		if(data == NULL)
			printf("<P>Error! Error in passing data from form to script.");
else if(sscanf(data,"m=%ld&n=%ld",&m,&n)!=2)
  printf("<P>Error! Invalid data. Data must be numeric.");
else
  printf("<P>The product of %ld and %ld is %ld.",m,n,m*n);

        /* Print out the current time */
        printf("The time is %s<br>\n", asctime(localtime(&tim)) );

	printf("</body>\n"
	       "</html>\n");

	return 0;
    } 
