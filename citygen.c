/*
 * Filename: citygen.c
 * Title:	City generator
 * Course:	CSC337 Spring 2008 (Adhar)
 * Author:	Brett C. Buddin (brett@intraspirit.net)
 * Date:	April 29, 2008
 * Use:		./citygen <NUMBER OF CITIES> <MAX WIDTH> <MAX HEIGHT> <FILE TO SAVE TO>
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
  FILE *fp;
  int i;
  struct timeval time;

  if (argc != 5) {
    fprintf(stderr, "Invalid Usage.\nUsage ./citygen <NUMBER OF CITIES> <MAX WIDTH> <MAX HEIGHT> <FILE TO SAVE TO>\n");
    return 1;
  }

  gettimeofday(&time, 0);
  srandom((int)(time.tv_usec * 1000000 + time.tv_sec));
  int num_cities = atoi(argv[1]);
  int max_width = atoi(argv[2]);
  int max_height = atoi(argv[3]);

  fp = fopen(argv[4], "w");
  fprintf(fp, "%d %d %d\n", num_cities, max_width, max_height);
  for(i=0; i < num_cities; i++) {
    fprintf(fp, "%d %d\n", (int)random() % atoi(argv[2]), (int)random() % atoi(argv[3]));
  }
  fclose(fp);
  return 0;
}

