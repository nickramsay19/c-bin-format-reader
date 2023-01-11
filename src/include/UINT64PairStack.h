#ifndef UINT64_PAIR_STACK_H
#define UINT64_PAIR_STACK_H

#define MAX_PAIRS 255

#include <stdlib.h>

struct UINT64PairStruct {
    uint64_t first;
    uint64_t second;
};

struct UINT64PairStackStruct {
    struct UINT64PairStruct pairs[MAX_PAIRS];
    uint8_t top;
};

typedef struct UINT64PairStackStruct *UINT64PairStack;

UINT64PairStack UINT64PairStackNew() {
    UINT64PairStack ips = (UINT64PairStack) malloc(sizeof(struct UINT64PairStackStruct));
    ips->top = 0;
    return ips;
}

int UINT64PairStackPush(UINT64PairStack ips, uint64_t first, uint64_t second) {
    // check if there is enough space
    if (ips->top > MAX_PAIRS - 1) {
        return EXIT_FAILURE;
    }

    ips->pairs[++ips->top].first = first;
    ips->pairs[++ips->top].second = second;
    
    return EXIT_SUCCESS;
}

int UINT64PairStackPop(UINT64PairStack ips, uint64_t *first, uint64_t *second) {
    // check if there is an element to pop
    if (ips->top == 0) {
        return EXIT_FAILURE;
    }

    *first = ips->pairs[ips->top].first;
    *second = ips->pairs[ips->top].second;

    return EXIT_SUCCESS;
}

#endif // INT_PAIR_STACK_H