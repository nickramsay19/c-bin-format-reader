#ifndef FORMAT_PARSER_H
#define FORMAT_PARSER_H

#define MAX_REPEAT 255

#define TOKEN_NULL 0
#define TOKEN_INT 1
#define TOKEN_UINT 2
#define TOKEN_CHAR 3
#define TOKEN_FLOAT 4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "UINT64PairStack.h"

struct FormatParserStruct {
    char *format_string; // keep on stack
    uint16_t format_string_len;
    uint16_t curr_pos;

    int last_token;
    int last_token_bytes;
    
    //int16_t jump_pos;

    // block repeats  
    UINT64PairStack pair_stack;

    // token repeatsd
    uint64_t repeat;
};

typedef struct FormatParserStruct *FormatParser;

FormatParser FormatParserNew(char *format_string) {
    FormatParser fp = (FormatParser) malloc(sizeof(struct FormatParserStruct));
    fp->format_string = format_string;
    fp->format_string_len = strlen(format_string);
    fp->curr_pos = 0;

    fp->last_token = TOKEN_NULL;
    fp->last_token_bytes = 0;
    
    fp->pair_stack = UINT64PairStackNew();

    fp->repeat = 0;

    return fp;
}

uint8_t char_to_uint8_t(char c) {
    return ((uint8_t) c) - 48;
}

uint64_t ten_to_pow(uint8_t pow) {
    uint64_t result = 1;
    for (int i = 0; i < pow; i++) {
        result *= 10;
    }

    return result;
}

uint64_t parse_number(uint16_t *pos, char *format_string) {

    // store digit columns in reverse
    static const uint8_t max_columns = 255;
    uint8_t columns[max_columns]; // assume numbers don't go larger than 256 digits
    uint8_t column_idx = max_columns - 1; 

    // we assume this function is called at a digit, so a do while loop will suffice
    do {
        columns[column_idx] = char_to_uint8_t(format_string[*pos]);
        *pos = *pos + 1;
    } while (columns[column_idx--] < 10 && column_idx > 0); 
    // ^ this works since if current pos isn't a digit, the value wont be < 10 due to unsigned integer overflow

    uint64_t result = 0;
    
    // loop from first digit to last
    // ignore current idx value which points to a non-digit
    for (uint8_t i = column_idx+2; i < max_columns; i++) {

        // calculate the power of ten
        const uint64_t pow = - column_idx + i - 2;

        // now calculate the val from the digit * 10^pow
        const uint64_t val = ((uint64_t) columns[i]) * ten_to_pow(pow);
        result += val;
    }

    return result;
}

int FormatParserNext(FormatParser fp, FILE *file) {

    switch (fp->format_string[fp->curr_pos]) {
        case '[':
            fp->jump_pos = fp->curr_pos;
            fp->curr_pos++;
            return FormatParserNext(fp, file);
        case ']':
            fp->curr_pos = fp->jump_pos;
            fp->curr_pos++;
            return FormatParserNext(fp, file);
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            // parse the rest of the number (if there is more)
            const uint64_t repeat = parse_number(fp->curr_pos, fp->format_string);

            switch (fp->format_string[fp->curr_pos]) {
                case '[':
                    fp->curr_pos++;
                    UINT64PairStackPush(fp->pair_stack, repeat, fp->curr_pos);
                    return FormatParserNext(fp, file); // still must get the actual next token
                default:
                    fp->repeat = repeat;
            }

            break;
        case '%':
            fp->curr_pos++;
            switch (fp->format_string[fp->curr_pos]) {

            }

    }

    return 0;
}

#endif // FORMAT_PARSER_H