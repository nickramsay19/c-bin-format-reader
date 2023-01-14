#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "include/FormatParser.h"

int main() {

    int token = 0;
    uint16_t token_bytes = 0;



    FormatParser fp = FormatParserNew("%d5%d%c");
    int fail = 0;
    do {
        printf("===============\n");

        int start = fp->curr_pos;
        fail = FormatParserNextFormatToken(fp, &token, &token_bytes);
        int end = fp->curr_pos;
        printf("[%d:%d] %d, %d\n", start, end, token, token_bytes);
    } while (token != FORMAT_TOKEN_EOF && !fail);

    if (fail) {
        printf("failed!\n");
    }

    return 0;
}