/*
base64_utils.c - handy base64 utilities based on libb64

This is adapted from the example code of libb64 project, which has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdecode.h"
#include "cencode.h"
#include "base64_utils.h"

#define SIZE 4096

char* encode_str(const char* input)
{
    /* length of input string */
    int len = strlen(input);
    /* set up a destination buffer large enough to hold the encoded data */
    char* output = (char*) malloc(len*2);
    /* keep track of our encoded position */
    char* c = output;
    /* store the number of bytes encoded by a single call */
    int cnt = 0;
    /* we need an encoder state */
    base64_encodestate s;

    /* String length should not be greater than 3,000 */
    if (len > 3000)
    {
        fprintf(stderr, "Input too long!\n");
        return NULL;
    }
    
    /*---------- START ENCODING ----------*/
    /* initialize the encoder state */
    base64_init_encodestate(&s);
    /* gather data from the input and send it to the output */
    cnt = base64_encode_block(input, len, c, &s);
    c += cnt;
    /* since we have encoded the entire input string, we know that 
        there is no more input data; finalise the encoding */
    cnt = base64_encode_blockend(c, &s);
    c += cnt;
    /*---------- STOP ENCODING  ----------*/
    
    /* we want to print the encoded data, so null-terminate it: */
    *c = 0;
    
    return output;
}

void encode_file(FILE* inputFile, FILE* outputFile)
{
    /* set up a destination buffer large enough to hold the encoded data */
    int size = SIZE;
    char* input = (char*)malloc(size);
    char* encoded = (char*)malloc(2*size); /* ~4/3 x input */
    /* we need an encoder and decoder state */
    base64_encodestate es;
    /* store the number of bytes encoded by a single call */
    int cnt = 0;
    
    /*---------- START ENCODING ----------*/
    /* initialise the encoder state */
    base64_init_encodestate(&es);
    /* gather data from the input and send it to the output */
    while (1)
    {
        cnt = fread(input, sizeof(char), size, inputFile);
        if (cnt == 0)
            break;
        cnt = base64_encode_block(input, cnt, encoded, &es);
        /* output the encoded bytes to the output file */
        fwrite(encoded, sizeof(char), cnt, outputFile);
    }
    /* since we have reached the end of the input file, we know that 
       there is no more input data; finalise the encoding */
    cnt = base64_encode_blockend(encoded, &es);
    /* write the last bytes to the output file */
    fwrite(encoded, sizeof(char), cnt, outputFile);
    /*---------- STOP ENCODING  ----------*/
	
    free(encoded);
    free(input);
}
