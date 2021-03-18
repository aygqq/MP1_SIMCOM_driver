#ifndef __DEBUG_H
#define __DEBUG_H

#include "main.h"

#define DEBUG_SIMCOM    1
// #define DEBUG_USART     1
#define DEBUG_PC        1
#define DEBUG_SMS       1

#define LEVEL_ERROR     1
#define LEVEL_MAIN      2
#define LEVEL_INFO      3
#define LEVEL_DEBUG     4

// #define LVL_SIMCOM_ERR  
// #define LVL_SIMCOM_MAIN 
// #define LVL_SIMCOM_INFO 
// #define LVL_SIMCOM_DBG  



// #define DEBUG_LEVEL     LEVEL_DEBUG
 #define DEBUG_LEVEL     LEVEL_INFO
// #define DEBUG_LEVEL     LEVEL_MAIN
#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL     LEVEL_ERROR
#endif

#define PRINT_LEVEL(level) do {\
	printf("[%05ld.%03ld]", HAL_GetTick()/1000, HAL_GetTick() % 1000);\
    if (level == LEVEL_ERROR) {\
        printf("[ERR ]:");\
    } else if (level == LEVEL_MAIN) {\
        printf("[MAIN]:");\
    } else if (level == LEVEL_INFO) {\
        printf("[INFO]:");\
    } else if (level == LEVEL_DEBUG) {\
        printf("[DBG ]:");\
    }\
} while (0)

//#define LOG(level, ...)

#define LOG(level, ...)  do {\
    if (DEBUG_LEVEL >= level) {\
        PRINT_LEVEL(level);\
        printf("Common  : ");\
        printf( __VA_ARGS__);\
    }\
} while (0)

#ifdef DEBUG_SIMCOM
    #define LOG_SIMCOM(level, ...)  do {\
        if (DEBUG_LEVEL >= level) {\
            PRINT_LEVEL(level);\
            printf("SIMCOM: ");\
            printf( __VA_ARGS__);\
        }\
    } while (0)
#else
    #define LOG_SIMCOM(level, ...)
#endif

#ifdef DEBUG_USART
    #define LOG_USART(idx, level, ...)  do {\
        if (DEBUG_LEVEL >= level) {\
            PRINT_LEVEL(level);\
            printf("USART : ");\
            log_info( __VA_ARGS__);\
        }\
    } while (0)
#else
    #define LOG_USART(idx, level, ...)
#endif

#ifdef DEBUG_PC
    #define LOG_PC(level, ...)  do {\
        if (DEBUG_LEVEL >= level) {\
            PRINT_LEVEL(level);\
            printf("PC    : ");\
            printf( __VA_ARGS__);\
        }\
    } while (0)
#else
    #define LOG_PC(level, ...)
#endif

#ifdef DEBUG_SMS
    #define LOG_SMS(level, ...)  do {\
        if (DEBUG_LEVEL >= level) {\
            PRINT_LEVEL(level);\
            printf("SMS   : ");\
            printf( __VA_ARGS__);\
        }\
    } while (0)
#else
    #define LOG_SMS(level, ...)
#endif

#endif
