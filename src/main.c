#include <stdio.h>
//#include <stdlib.h>

#include "include/FormatParser.h"

int main() {
    // parse the first arg
    char *str1 = "19318";
    uint16_t pos1 = 0;
    uint64_t val1 = parse_number(&pos1, str1);

    printf("%llu\n", val1);

    return 0;
}