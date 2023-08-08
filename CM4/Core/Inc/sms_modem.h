#ifndef __SMS_MODEM_H
#define __SMS_MODEM_H

#include "main.h"
#include "common.h"

typedef struct modem_info_s {
    uint8_t             inited;
    uint8_t             status;
    phone_number_t      phone;
} modem_info_t;

typedef struct sms_modem_s {
    modem_info_t        info;
    volatile uint8_t    flag_wait_responce;
    volatile uint8_t	flag_responce_recvd;
    uint8_t             error_cnt;
    uint8_t             error_recv_cnt;

    byte_buffer_t       recv_buf;
} sms_modem_t;

extern sms_modem_t modem;

void phone_to_hex(char *dst, char *src, uint16_t len);
void hex_to_phone(char *dst, char *src, uint16_t len);
void russian_to_hex(char *dst, char *src, uint16_t len);
void hex_to_russian(char *dst, char *src, uint16_t len);

void sms_power_on();
void sms_modem_reset();
uint8_t check_number(char *num);

uint8_t sms_send(phone_number_t *num, char *data);

void sms_recv_handler(char *data, phone_number_t *num);
void sms_control_task();

#endif /* __SMS_MODEM_H */
