/*
 * cleanprofiles.c
 *
 *  Created on: Aug 16, 2020
 *      Author: dad
 */

//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>


int main ()
{
	FILE *fp;
	char * line = NULL;
	size_t len = 250;
	ssize_t read;
	size_t size = 255;


	line = (char*)malloc(size);

	fp=fopen("StudentGroup.txt", "r");
	if (fp == NULL)return(1);

	while ((read = getline(&line, &len, fp)) != -1) {
	        printf("Retrieved line of length %zu:\n", read);
	        //fwrite(line, read, 1, stdout);
	        printf(" line found : \"%s\"\n", line);
	    }





	fclose(fp);
	if (line)free(line);
	return (0);
}


