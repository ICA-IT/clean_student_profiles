/*
 * cleanprofiles.c
 *
 *  Created on: Aug 16, 2020
 *      Author: dad
 *
 *		Note: be alert if inputing a Windows file into this program.
 *		The program expects an 8-bit ascii input file. Windows may produce
 *		wide format (16 bit) characters
 *
 *      Note: don't forget to run
 *      set-executionpolicy remotesigned
 *      in powershell before attempting to run the output script!
 *
 */

//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main ()
{
	FILE *fp;
	FILE *fp2;
	char * line = NULL;
	size_t len = 250;
	ssize_t read;
	size_t size = 255;
	char * redir;
	char redir1[] = ">";
	char redir2[] = ">>";

	redir = redir1;
	line = (char*)malloc(size);

	fp=fopen("StudentGroup.txt", "r");
	if (fp == NULL)return(1);

	fp2=fopen("GetSutdentSIDs.ps", "w");
	if (fp2 == NULL)return(2);

	while ((read = getline(&line, &len, fp)) != -1) {
//	        printf("Retrieved line of length %zu:\n", read);
			line[read-1] = '\000';
	        printf(" line found : \"%s\"\n", line);
	    	fprintf(fp2, "dsquery * %s -scope base -attr objectSid %s studentSIDs.txt\n",line,redir);
	    	redir = redir2;
	    }

	fclose(fp);
	if (line)free(line);
	return (0);
}


