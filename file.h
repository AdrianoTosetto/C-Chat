#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef char* string;
typedef unsigned int uint;


string readFile(const char* filename);

void writeFile(const string fileName,bool overwrite,const char *format,...);

string getString();

char** tokenizer(char*,const char*);

#endif
