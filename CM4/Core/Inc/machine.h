#ifndef __MACHINE_H
#define __MACHINE_H

#include "main.h"
#include "common.h"


#define HDR_SIZE        sizeof(cmd_header_t)
#define CRC_SIZE        sizeof(uint16_t)

typedef enum pc_cmd_type_e
{
    CMD_NONE = 0,
    CMD_LOCK = 1,
    CMD_UNLOCK,
    CMD_FLYMODE,
    CMD_AT_CMD,
    CMD_POWER,
    CMD_CHANGE_SIM,
    CMD_LCD_PRINT,
    CMD_LCD_BLINK,
    CMD_SET_IMEI,
    CMD_SET_CONFIG = 11,
    CMD_GET_CONFIG,
    CMD_MODEM_READY,
    CMD_CFG_ERROR,
    CMD_CTRL_ERROR,
    CMD_PC_WAITMODE,
    CMD_PC_SHUTDOWN,
    CMD_PC_READY = 18,
    CMD_NEW_PHONES,
    CMD_SEND_SMS = 20,
    CMD_REQ_MODEM_INFO = 21,
    CMD_REQ_CONN_INFO = 22,
    CMD_REQ_PHONES,
    CMD_REQ_REASON,
    CMD_OUT_SHUTDOWN,
    CMD_OUT_SAVE_STATE,
    CMD_OUT_SIM_CHANGE,
    CMD_OUT_SMS = 28,
    CMD_OUT_AT_CMD
} pc_cmd_type_t;

typedef struct __attribute__((__packed__)) cmd_header_s {
    uint8_t         cmd_type;
    uint16_t        length;
} cmd_header_t;

typedef struct pc_s {
    byte_buffer_t send_buf;
    byte_buffer_t recv_buf;
} pc_t;

extern pc_t         pc;

void pc_send_sms(uint8_t type, phone_number_t *phone, char *data);

int  pc_recv_handler(char *buf, uint32_t length);

#endif /* __MACHINE_H */
