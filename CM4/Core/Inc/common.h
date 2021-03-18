#ifndef __COMMON_H
#define __COMMON_H

#include "main.h"


#define BYTE_BUF_SIZE   512

#define PHONE_SIZE      16

typedef struct byte_buffer_s {
    char data[BYTE_BUF_SIZE];
    uint16_t ptr;
} byte_buffer_t;

typedef struct phone_number_s {
    char digit[PHONE_SIZE];
} phone_number_t;

#endif /* __COMMON_H */
