#include <ctype.h>

#include "debug.h"
#include "usart.h"
#include "simcom.h"

#include "common.h"
#include "sms_modem.h"
#include "machine.h"

extern pc_t pc;

extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart3;

SMS_STRUCT smsMsg[3];
uint32_t simcom_timeout = 0;
static uint8_t simcom_state = SIM_STATE_READ;
static uint8_t next_state;
static uint8_t reset_step = 0;

sms_modem_t modem;


void phone_to_hex(char *dst, char *src, uint16_t len)
{
	uint16_t i;
    char *ptr;
    ptr = dst;
    for (i = 0; i < len; i++) {
        sprintf(ptr, "%04X", src[i]);
        ptr += 4;
    }
}
void hex_to_phone(char *dst, char *src, uint16_t len)
{
	uint16_t i, j;
    char cc[3];
    char cccc[5];
    uint16_t temp;
    cccc[4] = '\0';
    cc[2] = '\0';
    for (i = 0, j = 0; i < len; i+=4, j++) {
        memcpy(cccc, &src[i], 4);
        temp = (uint16_t)strtoul(cccc, NULL, 16);
        memcpy(cc, &temp, 2);
        dst[j] = cc[0];
    }
}
void russian_to_hex(char *dst, char *src, uint16_t len)
{
	uint16_t i;
    char *ptr;
    ptr = dst;
    for (i = 0; i < len; i++) {
        if (src[i] < 0x80) {
            // Latin
            sprintf(ptr, "%04X", src[i]);
            ptr += 4;
        } else if (src[i] == 0xD0 && src[i+1] > 0x80) {
            // Russian big
            src[i] -= 0xCC;
            src[i+1] -= 0x80;
            sprintf(ptr, "%02X", src[i]);
            ptr += 2;
            i++;
            sprintf(ptr, "%02X", src[i]);
            ptr += 2;
        } else if (src[i] == 0xD1 && src[i+1] > 0x40) {
            // Russian small
            src[i] -= 0xCD;
            src[i+1] -= 0x40;
            sprintf(ptr, "%02X", src[i]);
            ptr += 2;
            i++;
            sprintf(ptr, "%02X", src[i]);
            ptr += 2;
        }
      
    }
}
void hex_to_russian(char *dst, char *src, uint16_t len)
{
    uint16_t i;
    uint16_t ptr = 0;
    char cc[3];
    char cccc[5];
    uint16_t temp;
    cccc[4] = '\0';
    cc[2] = '\0';
    for (i = 0; i < len; i+=4) {
        memcpy(cccc, &src[i], 4);
        temp = (uint16_t)strtoul(cccc, NULL, 16);
        memcpy(cc, &temp, 2);
        if (cc[1] == 0x04) {
            if (cc[0] < 0x40) {
                cc[1] += 0xCC;
                cc[0] += 0x80;
            } else {
                cc[1] += 0xCD;
                cc[0] += 0x40;
            }
        }
        if (cc[1] != 0) {
            dst[ptr++] = cc[1];
        } 
        dst[ptr++] = cc[0];
    }
    dst[ptr] = 0;
}

uint8_t check_number(char *num) {
	int i;

	if (num[0] == '*') {
		return 0;
	}

	for (i = 0; i < PHONE_SIZE; ++i) {
		if (i > 2 && num[i] == '\0')
			break;
		if ((num[i] < '0' || num[i] > '9') && num[i] != '+') {
			return 0;
		}
	}

	return 1;
}

uint8_t sms_send(phone_number_t *num, char *data) {
	if (!check_number((char*) num)) {
		LOG_SMS(LEVEL_ERROR, "Empty phone number! %s\r\n", num->digit);
		return 0;
	}

	LOG_SMS(LEVEL_INFO, "Send message to %s, data: %s\n", (uint8_t*)num, data);

	return SIMCOM_SendSMS(&huart3, (char*) num, data);
}

void sms_recv_handler(char *data, phone_number_t *num) {
	pc_send_sms(1, num, data);
}

void sms_control_task() {
    uint8_t cnt, i;

    switch (simcom_state) {
        case SIM_STATE_READ:
            if (modem.info.inited)
            {
                cnt = SIMCOM_ReadSMS(&huart3, smsMsg);
                if (cnt > 0)
                {
                    for (i = 0; i < cnt; i++)
                    {
                        sms_recv_handler(smsMsg[i].content,
                                        (phone_number_t *)smsMsg[i].number);
                    }
                }
                SIMCOM_ReadStatus(&huart3);
                if (modem.error_cnt >= 8)
                {
                    LOG_SIMCOM(LEVEL_ERROR, "Modem will be reloaded\n\r");
                    modem.error_cnt = 0;
                    modem.error_recv_cnt = 0;
                    modem.info.inited = 0;
                }
            
                simcom_timeout = HAL_GetTick() + 2000;
                simcom_state = SIM_STATE_DELAY;
                next_state = SIM_STATE_READ;
            } else {
                simcom_state = SIM_STATE_RESET;
                reset_step = 0;
            }
            break;
        case SIM_STATE_RESET:
        	switch (reset_step) {
        	case 0:
        	    HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, GPIO_PIN_SET);
        	    simcom_timeout = HAL_GetTick() + 4000;
				simcom_state = SIM_STATE_DELAY;
				next_state = SIM_STATE_RESET;
                LOG_SIMCOM(LEVEL_DEBUG, "Reset step 0\n\r");
        		reset_step++;
        		break;
        	case 1:
        	    HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, GPIO_PIN_RESET);
        	    simcom_timeout = HAL_GetTick() + 8000;
				simcom_state = SIM_STATE_DELAY;
				next_state = SIM_STATE_RESET;
                LOG_SIMCOM(LEVEL_DEBUG, "Reset step 1\n\r");
        		reset_step++;
        		break;
        	case 2:
        		HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, GPIO_PIN_SET);
				HAL_Delay(200);
				HAL_GPIO_WritePin(GSM_POWER_GPIO_Port, GSM_POWER_Pin, GPIO_PIN_RESET);
        	    simcom_timeout = HAL_GetTick() + 12000;
				simcom_state = SIM_STATE_DELAY;
				next_state = SIM_STATE_INIT;
                LOG_SIMCOM(LEVEL_DEBUG, "Reset step 2\n\r");
        		reset_step = 0;
        		break;
        	default:
        		simcom_state = SIM_STATE_READ;
        		reset_step = 0;
        		break;
        	}
            break;
        case SIM_STATE_INIT:
            USART_Start_receive(&huart3);
            modem.info.inited = SIMCOM_Init(&huart3);
            
            simcom_timeout = HAL_GetTick() + 2000;
            simcom_state = SIM_STATE_DELAY;
            next_state = SIM_STATE_READ;
            break;
        case SIM_STATE_DELAY:
            if (HAL_GetTick() < simcom_timeout) {
                HAL_Delay(10);
                break;
            } else {
                simcom_state = next_state;
            }
            break;
        default:
            break;
        }
}
