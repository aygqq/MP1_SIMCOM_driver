#ifndef __UTILS_CRC_H
#define __UTILS_CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

uint16_t calcCrc16(unsigned char* pcBlock, unsigned short len);

#ifdef __cplusplus
}
#endif

#endif /* __UTILS_CRC_H */
