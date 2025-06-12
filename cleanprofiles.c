/*
 * cleanprofiles.c
 *
 *  Created on: Aug 16, 2020
 *      Author: dad
 *
 *		The program takes a single (optional) calling parameter that controls which stage
 *		of the program is executed: If missing or "both", both stages are run. If "first"
 *		then only the first is run. If "second, only the second.
 *
 *		Note: be alert if inputting a Windows file into this program on linux.
 *		The program expects an 8-bit ascii input file. Windows may produce
 *		wide format (16 bit) characters. I recomemnd running dos2unix on all input files.
 *		You should also delete any blank lines at the end of the files.
 *
 *		To create the initial input file (StudentGroup.txt), run the following
 *		command on a windows domain member in an elevated cmd window:
 *		dsget group "CN=ICA students,OU=students,OU=ICA users,DC=campus,DC=islandchristianacademy,DC=com" -members > StudentGroup.txt
 *
 *		The first output file (GetSutdentSIDs.ps1) converts the list of ICA STudent group members
 *		to a list of SIDs. I run it as a powershell script back on your domain Windows computer.
 *		(It should work in an elevated cmd window, too.)
 *
 *		Copy the SIDs file created by the GetSutdentSIDs.ps1 script back to wherever you are
 *		running this program and re-run it. The second run will produce the final output file
 *		(CleanStudentProfiles.ps1). Run this second powershell script on each domain computer that you
 *		wish to clean. Be patient, it might take a while to finish cleaning; it depends on the
 *		number of profiles on the machine, the number of files to be deleted, and the fragmentation
 *		of the registry file. If you don't want to wait around, you can try the following:
 *		powershell -noexit "& ""C:\users\dad\desktop\CleanStudentProfiles.ps1"""
 *
 *      Note: don't forget to run
 *      set-executionpolicy remotesigned
 *      in powershell before attempting to run the powershell scripts!
 *
 *      Note: For help depolying the cleaning script using opsi, see:
 *      git@github.com:ICA-IT/ica-clean-profiles.git
 *
 */

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef WINDOZE
#include <windows.h>
#include <winbase.h>
#ifdef CHECKWIDE
// Function to heuristically check if a string is likely wide (UTF-16)
BOOL IsLikelyWide(const char* str, int len) {
	// Check for UTF-16 byte order mark (BOM)
	if (len >= 2 && str[0] == (char)0xFF && str[1] == (char)0xFE) {
		return TRUE;
	}
	// Check if the string contains a null byte before the end
	// This is a common indicator of UTF-16, where characters can be 2 bytes
	for (int i = 0; i < len - 1; ++i) {
		if (str[i] == '\0') {
			return TRUE;
		}
	}
	// As a fallback, use the Windows API function IsTextUnicode for more sophisticated checks
	// This function can check for BOMs and other indicators, and uses heuristics
	int result = IsTextUnicode(str, len, NULL);
//	return (result & IS_TEXT_UNICODE_STATISTICS) || (result & IS_TEXT_UNICODE_BOM);
	return (result & IS_TEXT_UNICODE_STATISTICS);
}
#endif	// CHECKWIDE
// set WIDECHAR for the Windows version only!
// Note that the test files included in git are NOT wide!
// Also note that the opsi script now produces ASCII input for phase 2. I.e., the WIDECHAR
// feature should no longer pe required.
#ifdef WIDECHAR
#include <wchar.h>
#endif // WIDECHAR
#define ssize_t int
ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
	if ( fgets(*lineptr, (int)n, stream) == NULL ) return(-1);
	return strlen( *lineptr);
}
#ifdef WIDECHAR
ssize_t getlinew(char** lineptr, size_t* n, FILE* stream)
{
	wchar_t *linew;
	char* line;
	linew = (wchar_t*)malloc((*n) * 2);
	line = *lineptr;
	if (fgetws(linew, (int)n, stream) == NULL) return(-1);
#ifdef CHECKWIDE
	if (IsLikelyWide(linew, strlen(linew))) {
		printf("The string '%s' is likely wide (UTF-16).\n", linew);
	}
	else {
		printf("The string '%s' is likely ANSI.\n", linew);
	}
#endif// CHECKWIDE
	for (int i = 0; i < (int)wcslen(linew); ++i) {
		*line = wctob(linew[i]);
		line++;
	}
	free(linew);
	*line = '\000';
	return strlen(*lineptr);
}
#endif	// WIDECHAR
#endif // WINDOZE

void trimleadingandTrailing(char *s);

int main(int argc, char* argv[])
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
	int first = 0;
	int second = 0;

	if (argc > 1) {
		if ((strcmp(argv[1], "both") == 0) | (strcmp(argv[1], "first") == 0)) first = 1;
		if ((strcmp(argv[1], "both") == 0) | (strcmp(argv[1], "second") == 0)) second = 1;
	}
	else {
		first = 1;
		second = 1;
	}

	if ( (first == 0) & (second == 0) ) {
		printf("\nProgram to generate Windows domain profile cleaning script:\n");
		printf("Calling parameters:\n");
		printf("cleanprofiles [first|second|both]\n\n");
		printf("The program processes in two stages. The first produces a script to list SIDs.\n");
		printf("The second produces a script to clean profiles.\n");
		printf("The calling parameter controls which stage of the program is executed :\n");
		printf("If missing or \"both\", both stages are run.\n");
		printf("If \"first\", then only the first is run.\n");
		printf("If \"second\", only the second.\n\n");
		printf("First stage input file = StudentGroup.txt\n");
		printf("First stage output file = GetSutdentSIDs.ps1\n");
		printf("Second stage input file = studentSIDs.txt\n");
		printf("Second stage output file = CleanStudentProfiles.ps1\n\n");
	}

	redir = redir1;
	line = (char*)malloc(size);
	
	if (first) {
		fp = fopen("StudentGroup.txt", "r");
		if (fp == NULL)return(1);

		fp2 = fopen("GetSutdentSIDs.ps1", "w");
		if (fp2 == NULL)return(2);

		while ((read = getline(&line, &len, fp)) != -1) {
			//	        printf("Retrieved line of length %zu:\n", read);
			line[read - 1] = '\000';
			printf(" line found : \"%s\"\n", line);
			if ( strlen(line) > 2 ) fprintf(fp2, "dsquery * %s -scope base -attr objectSid %s studentSIDs.txt\n", line, redir);
			redir = redir2;
		}
		fclose(fp);
		fclose(fp2);
	}

	if (second) {
#ifdef WIDECHAR
		fp = fopen("studentSIDs.txt", "r,ccs=UTF-16LE");
#else
		fp = fopen("studentSIDs.txt", "r");
#endif
		if (fp == NULL)return(3);

		fp2 = fopen("CleanStudentProfiles.ps1", "w");
		if (fp2 == NULL)return(4);

#ifdef WIDECHAR
		while ((read = getlinew(&line, &len, fp)) != -1) {
#else
		while ((read = getline(&line, &len, fp)) != -1) {
#endif
			//	        printf("Retrieved line of length %zu:\n", read);
			if (read < 7) continue;	// skip any BOM
			line[read - 1] = '\000';
			trimleadingandTrailing(line);
			printf(" line found : \"%s\"\n", line);
			if (strcmp(line, "objectSid") != 0) return(5);
#ifdef WIDECHAR
			if ((read = getlinew(&line, &len, fp)) == -1) return (6);
#else
			if ((read = getline(&line, &len, fp)) == -1) return (6);
#endif
			line[read - 1] = '\000';
			trimleadingandTrailing(line);
			printf(" line found : \"%s\"\n", line);
			if (strlen(line) > 20) 
				fprintf(fp2, "Get-CimInstance -Class Win32_UserProfile | Where-Object { $_.SID -eq \'%s\' }  | Remove-CimInstance\n", line);
		}

		fclose(fp);
		fclose(fp2);
	}

	if (line)free(line);
	return (0);
}

void trimleadingandTrailing(char *s)
{
	int  i,j;

	for(i=0;s[i]==' '||s[i]=='\t';i++);

	for(j=0;s[i];i++)
	{
		s[j++]=s[i];
	}
	s[j]='\0';
	for(i=0;s[i]!='\0';i++)
	{
		if(s[i]!=' '&& s[i]!='\t')
				j=i;
	}
	s[j+1]='\0';
}
