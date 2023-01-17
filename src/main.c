#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "include/FormatParser.h"

uint32_t swap_endianess(uint32_t num) {
    return ((num>>24)&0xff) | // move byte 3 to byte 0
           ((num<<8)&0xff0000) | // move byte 1 to byte 2
           ((num>>8)&0xff00) | // move byte 2 to byte 1
           ((num<<24)&0xff000000); // byte 0 to byte 3
}

int main(int argc, char **argv) {

    // check if theres enough args
    if (argc < 3) {
        printf("Error: Not enough args.\n");
        return EXIT_SUCCESS;
    }

    // setup parser
    // mustn't modify format string from here on out as it is still stack allocated
    FormatParser parser = FormatParserNew(argv[1]);

    // open file
    FILE *fp = fopen(argv[2], "rb");

    // test
    int *token = malloc(sizeof(int));
    uint16_t *token_bytes = malloc(sizeof(uint16_t));
    uint64_t *block = malloc(sizeof(uint64_t));
    int finish = 0; // doubles as a finish looping indicator and stores error code from parser
    do {
        finish = FormatParserNextFormatToken(parser, token, token_bytes);

        // extract and print from bin file fp
        switch (*token) {
            case FORMAT_TOKEN_INT: {
                fread(block, *token_bytes, 1, fp);
                if (*token_bytes == 1) {
                    printf("%hd", (int8_t) *block);
                } else if (*token_bytes <= 2) {
                    printf("%hd", (int16_t) *block);
                } else if (*token_bytes <= 4) {
                    printf("%d", (int32_t) *block); // >> 4-*token_bytes
                } else if (*token_bytes <= 8) {
                    printf("%lld", (int64_t) *block); // >> 8-*token_bytes
                } else {
                    printf("\nError: Invalid \"token_bytes\" size for a signed integer. \"token_bytes\" must be less than 9.\n");
                    finish = EXIT_FAILURE; // will end loop
                }
                break;
            } case FORMAT_TOKEN_UINT: {
                fread(block, *token_bytes, 1, fp);
                if (*token_bytes == 1) {
                    printf("%hu", (uint8_t) *block);
                } else if (*token_bytes <= 2) {
                    printf("%hu", (uint16_t) *block);
                } else if (*token_bytes <= 4) {
                    printf("%u", (uint32_t) *block); // >> 4-*token_bytes
                } else if (*token_bytes <= 8) {
                    printf("%llu", (uint64_t) *block); // >> 8-*token_bytes
                } else {
                    printf("\nError: Invalid \"token_bytes\" size for an unsigned integer. \"token_bytes\" must be less than 9.\n");
                    finish = EXIT_FAILURE; // will end loop
                }
                break;
            } case FORMAT_TOKEN_FLOAT: {
                fread(block, *token_bytes, 1, fp);
                if (*token_bytes == 4) {
                    printf("(%f)", (float) *block);
                } else if (*token_bytes == 8) {
                    printf("(%lf)", (double) *block);
                } else {
                    printf("\nError: Invalid \"token_bytes\" size for floating point number. \"token_bytes\" must be either 4 or 8.\n");
                    finish = EXIT_FAILURE; // will end loop
                }
                break;
            } case FORMAT_TOKEN_CHAR: {
                fread(block, 1, 1, fp);
                printf("%c,", (char) *block);
                break;
            } case FORMAT_TOKEN_NULL: {
                finish = true;
                break;
            } default: {
                printf("?");
                break;
            } 
        }
    } while (*token != FORMAT_TOKEN_NULL && !finish);

    // clean up
    fclose(fp);
    FormatParserFree(parser);
    free(token);
    free(token_bytes);
    free(block);
    
    return 0;
}