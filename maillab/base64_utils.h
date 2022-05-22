#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H

#include <stdio.h>

char* encode_str(const char* input);
void encode_file(FILE* inputFile, FILE* outputFile);

#endif