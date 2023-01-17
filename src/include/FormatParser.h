#ifndef FORMAT_PARSER_H
#define FORMAT_PARSER_H

#define MAX_REPEAT 255

#define FORMAT_TOKEN_NULL 0
#define FORMAT_TOKEN_INT 1
#define FORMAT_TOKEN_UINT 2
#define FORMAT_TOKEN_CHAR 3
#define FORMAT_TOKEN_FLOAT 4
#define FORMAT_TOKEN_STRING 5
//#define FORMAT_TOKEN_EOF 6

#define UINT64_INFINITY ((uint64_t) -1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "UINT64PairStack.h"

struct FormatParserStruct {
    char *format_string; // keep on stack
    uint16_t format_string_len;
    uint16_t curr_pos;

    // repeat single token
    uint64_t repeat;
    int last_token;
    uint16_t last_token_bytes;
    
    // repeat blocks (must use a stack)
    UINT64PairStack pair_stack;
};

typedef struct FormatParserStruct *FormatParser;

FormatParser FormatParserNew(char *format_string) {
    FormatParser fp = (FormatParser) malloc(sizeof(struct FormatParserStruct));
    fp->format_string = format_string;
    fp->format_string_len = strlen(format_string);
    fp->curr_pos = 0;

    fp->repeat = 0;
    fp->last_token = FORMAT_TOKEN_NULL;
    fp->last_token_bytes = 0;
    
    fp->pair_stack = UINT64PairStackNew();

    return fp;
}

int FormatParserFree(FormatParser fp) {
    free(fp);

    return EXIT_SUCCESS;
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

int parse_number(FormatParser fp, uint64_t *number) {

    // store digit columns in reverse
    static const uint8_t max_digits = 255;
    uint64_t digits[max_digits]; // assume numbers don't go larger than 256 digits

    const uint16_t first_digit_idx = fp->curr_pos;
    uint16_t last_digit_idx = first_digit_idx + 1;

    // move cursor along each digit number
    // keep both d and curr_pos aligned (but offset to each other [by first_digit_pos])
    for (uint16_t d = 0; d < max_digits && fp->curr_pos < fp->format_string_len; ++d, ++fp->curr_pos) {
        const char c = fp->format_string[fp->curr_pos];
        const uint8_t digit = char_to_uint8_t(c);

        // check if the current digit is a number char
        // if the current char is not a digit then stop parsing now
        if (digit > 9 || digit < 0) {
            break;
        }

        // add the digit
        digits[d] = digit;
    }

    // move the last digit idx to end
    last_digit_idx = fp->curr_pos - 1;

    // calculate the highest power of ten
    const uint16_t highest_ten_power = last_digit_idx - first_digit_idx;

    // reset number
    *number = 0;

    // loop both powers and digit idx "d"
    for (uint16_t d = 0, pow = highest_ten_power; d <= last_digit_idx && pow >= 0; ++d, --pow) {
        //printf("%llu, %d\n", digits[d], pow);
        *number += digits[d] * ten_to_pow(pow);
    }

    return EXIT_SUCCESS;
}

int parse_type_specifier(FormatParser fp, int *token, uint16_t *token_bytes) {
    int token_val = FORMAT_TOKEN_NULL;
    uint16_t token_bytes_val = 0;

    switch(fp->format_string[fp->curr_pos]) {
        case 'd':
        case 'i':
            token_val = FORMAT_TOKEN_INT;
            token_bytes_val = 4; // default
            break;
        case 'u':
            token_val = FORMAT_TOKEN_UINT;
            token_bytes_val = 4; // default
            break;
        case 'c':
            token_val = FORMAT_TOKEN_CHAR;
            token_bytes_val = 1; // default
            break;
        case 'f':
            token_val = FORMAT_TOKEN_FLOAT;
            token_bytes_val = 4; // default
            break;
        case 's':
            token_val = FORMAT_TOKEN_STRING;
            token_bytes_val = 0;
            break;
        default:
            return EXIT_FAILURE;
    }

    if (token != NULL) {
        *token = token_val;
    }
    if (token_bytes != NULL) {
        *token_bytes = token_bytes_val;
    }

    return EXIT_SUCCESS;
}

int parse_var_size_type_specifier(FormatParser fp, int *token, uint16_t *token_bytes) {
    // assumption: last char in format_string = '%'

    switch (fp->format_string[fp->curr_pos]) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            // parse the byte count num
            parse_number(fp, (uint64_t*) token_bytes);
            //printf("Parsed byte count: %d\n", *token_bytes);

            //printf("PTS2 %d\n", fp->curr_pos);

            // now parse the format token following the byte count
            // give it null for token_bytes since it as already calculated
            if (parse_type_specifier(fp, token, NULL) == EXIT_SUCCESS) {

                // remember in case of repeats
                fp->last_token = *token;
                fp->last_token_bytes = *token_bytes;

                fp->curr_pos++;
                return EXIT_SUCCESS;
            } else {
                return EXIT_FAILURE;
            }
        } case 'd':
        case 'i':
        case 'u':
        case 'f':
        case 'c':
        case 's':
            //printf("PTS1 %d\n", fp->curr_pos);
            if (parse_type_specifier(fp, token, token_bytes) == EXIT_SUCCESS) {
                // remember in case of repeats
                fp->last_token = *token;
                fp->last_token_bytes = *token_bytes;

                fp->curr_pos++;

                return EXIT_SUCCESS;
            } else {
                printf("F1\n");
                return EXIT_FAILURE;
            }
        default:
            return EXIT_FAILURE;
    }
}

int FormatParserNextFormatToken(FormatParser fp, int *token, uint16_t *token_bytes) {


    // check for single token repeats
    if (fp->repeat > 0) {
        //printf("repeats: %llu\n", fp->repeat);
        *token = fp->last_token;
        *token_bytes = fp->last_token_bytes;
        fp->repeat--;
        return EXIT_SUCCESS;
    }
    
    // check that we haven't reached the end
    if (fp->curr_pos >= fp->format_string_len) {
        *token = FORMAT_TOKEN_NULL;
        *token_bytes = 0;

        // successfully parsed up to EOF
        return EXIT_SUCCESS; 
    }

    switch (fp->format_string[fp->curr_pos]) {
        case '[': {
            uint64_t block_repeat = UINT64_INFINITY;     
            if (fp->repeat > 0) {
                block_repeat = fp->repeat;
            }

            UINT64PairStackPush(fp->pair_stack, fp->curr_pos, block_repeat);
            fp->curr_pos++;
            return FormatParserNextFormatToken(fp, token, token_bytes);
            
           break;
        } case ']': {

            // pop off repeat stack to check its values
            uint64_t block_start = 0, block_repeats = 0;
            UINT64PairStackPop(fp->pair_stack, &block_start, &block_repeats);

            // push back again if there are more repeats
            if (--block_repeats > 0) {
                UINT64PairStackPush(fp->pair_stack, block_start, block_repeats);

                // move cursor back to start of block
                fp->curr_pos = block_start;
            }
            return FormatParserNextFormatToken(fp, token, token_bytes);
        } case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            //int start = fp->curr_pos;

            // set repeat to the number parsed
            parse_number(fp, &fp->repeat); // will overwrite fp->repeat
            //printf("1just read single token number: [%d:%d] %llu\n", start, fp->curr_pos, fp->repeat);
            
            // since we are performing a read now, we must decrement repeat now
            fp->repeat--;

            // TODO: parse a block

            // for now just parse single token and move along
            if (fp->format_string[fp->curr_pos++] != '%') {
                return EXIT_FAILURE;
            }

            return parse_var_size_type_specifier(fp, token, token_bytes);
        } case '%':
            ++fp->curr_pos; // skip over '%' char
            return parse_var_size_type_specifier(fp, token, token_bytes);
        default:
            return EXIT_FAILURE;

    }

    return 0;
}

#endif // FORMAT_PARSER_H