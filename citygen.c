/*
 * Filename: citygen.c
 * Title:	City generator
 * Course:	CSC337 Spring 2008 (Adhar)
 * Author:	Brett C. Buddin (brett@intraspirit.net)
 * Date:	April 29, 2008
 * Use:		./citygen <NUMBER OF CITIES> <MAX WIDTH> <MAX HEIGHT> <FILE TO SAVE TO>
*/


#include <stdio.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{	
	FILE *fp;
	int i;
	struct timeval time;
	
	gettimeofday(&time, 0);
	srandom((int)(time.tv_usec * 1000000 + time.tv_sec));

	fp = fopen(argv[4], "w");
	fprintf(fp, "%dx%d\n", atoi(argv[2]), atoi(argv[3]));
	for(i=0; i<atoi(argv[1]); i++) {
		fprintf(fp, "%d,%d\n", random() % atoi(argv[2]), random() % atoi(argv[3]));
	}
	fclose(fp);
}
