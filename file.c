#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>


typedef char* string;

extern int errno;

string getString(){
    string buffer = NULL;
    
    unsigned int capacity = 0;

    unsigned int n = 0;
    int c;
    while ((c = fgetc(stdin)) != '\n'){
        if (n + 1 > capacity){
            if (capacity == 0){
                capacity = 32;
	}else if (capacity <= (UINT_MAX / 2)){
          capacity *= 2;
	}
    else{
        	free(buffer);
                return NULL;
    }

    string temp = (char *) realloc(buffer, capacity * sizeof(char));
    if (temp == NULL){
   		int err = errno;
	 	fprintf(stderr, "Error opening file: %s\n", strerror(err));
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }

        buffer[n++] = c;
    }

    if (n == 0 && c == EOF){
 	int err = errno;
   	fprintf(stderr, "Error: %s\n", strerror(err));
        return NULL;
     }

    string minimal = (char *)malloc((n + 1) * sizeof(char));
    strncpy(minimal, buffer, n);
    free(buffer);

    minimal[n] = '\0';
    return minimal;
}


string readFile(const char *filename){
	FILE *file = fopen(filename,"r+");
	
    string buffer = NULL;
    
    unsigned int capacity = 0;

    unsigned int n = 0;
    int c;
    //reads characters from file until the end of file
    while ((c = fgetc(file)) && c != EOF){
        if (n + 1 > capacity){
            if (capacity == 0){
                capacity = 32;
    	}else if (capacity <= (UINT_MAX / 2)){
                capacity *= 2;
	}else{
        free(buffer);
        return NULL;
    }

    string temp = (char *) realloc(buffer, capacity * sizeof(char));
    if (temp == NULL){
   	int err = errno;
	fprintf(stderr, "Error opening file: %s\n", strerror(err));
        free(buffer);
        return NULL;
    }
            buffer = temp;
    }

        buffer[n++] = c;
    }

    if (n == 0 && c == EOF){
    	fclose(file);
 	int err = errno;
   	fprintf(stderr, "Error: %s\n", strerror(err));
        return NULL;
    }

    string minimal = (char *)malloc((n + 1) * sizeof(char));
    strncpy(minimal, buffer, n);
    free(buffer);

    minimal[n] = '\0';

    fclose(file);
    return minimal;
}
void writeFile(const string fileName,bool overwrite,const char *format,...){
	FILE *file;
	va_list args;
	va_start(args,format);
	if(overwrite){
		file = fopen(fileName,"w+");
	}else{
		file = fopen(fileName,"a+");
	}
	if(file == NULL){
	    int err = errno;
   	    fprintf(stderr, "Error: %s\n", strerror(err));
   	    fclose(file);
   	    va_end(args);
   	    return ;
	}
	vfprintf(file,format,args);
	
	fclose(file);
	va_end(args);
}

char** tokenizer(char *string,const char *delimiter){
	char *token;
	char **tokens = (char **)malloc(strlen(string) * sizeof(char*));
	int  tokensNumber = 0;
	
	char str[strlen(string)];
	strcpy(str,string);
	token = strtok(str,delimiter);

	
	while(token != NULL){
		*(tokens+tokensNumber) = (char *)malloc(sizeof(char)*(strlen(token) + 1));
	   	strcpy(*(tokens+tokensNumber),token);
	   	tokensNumber++;
		token = strtok(NULL,delimiter);
	}

	*(tokens+tokensNumber) = NULL;
	return tokens;
}
