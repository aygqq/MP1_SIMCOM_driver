#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "simcom.h"
#include "usart.h"

//#include "wwdg.h"
#include "main.h"
#include "common.h"
#include "sms_modem.h"

extern sms_modem_t modem;

extern byte_buffer_t uart3_rx_buf;

int RecvResponce(UART_HandleTypeDef *huart, char *responce, uint32_t millis) {
	int length = 0;
	uint32_t delay = HAL_GetTick() + millis;

	if (huart->Instance == USART3) {
		while (!modem.flag_responce_recvd && HAL_GetTick() < delay) {
			__NOP();

//	        HAL_WWDG_Refresh(&hwwdg1);
		}
		if (!modem.flag_responce_recvd) {
			modem.flag_wait_responce = 0;
			responce[0] = '\0';
			return 0;
		}

		__disable_irq();
		memcpy(responce, uart3_rx_buf.data, uart3_rx_buf.ptr);
		responce[uart3_rx_buf.ptr] = '\0';
		length = uart3_rx_buf.ptr;

		modem.flag_wait_responce = 0;
		modem.flag_responce_recvd = 0;
	} else {
		responce[0] = '\0';
		return 0;
	}

    __HAL_DMA_DISABLE(huart->hdmarx);
    __HAL_DMA_SET_COUNTER(huart->hdmarx, BYTE_BUF_SIZE);
    __HAL_DMA_ENABLE(huart->hdmarx);

	__enable_irq();

	LOG_SIMCOM(LEVEL_DEBUG, "RecvResponce: %s\n\r", responce);

	return length;
}

/*
 * Send AT command.
 */
static void SendCmd(UART_HandleTypeDef *huart, char *cmd, uint8_t wait_responce) {
	if (wait_responce) {
		modem.flag_wait_responce = 1;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Send: %s\n\r", cmd);
    
    uart3_rx_buf.ptr = 0;
    __HAL_DMA_DISABLE(huart3.hdmarx);
    __HAL_DMA_SET_COUNTER(huart3.hdmarx, BYTE_BUF_SIZE);
    __HAL_DMA_ENABLE(huart3.hdmarx);

	USART_Send(huart, cmd, strlen(cmd));
	USART_Send(huart, "\r\n", 2);
}

/*
 * Send AT command to SIMCOM module and check response immediately.
 */
static int SendCmd_Check(UART_HandleTypeDef *huart, char *cmd, char *check) {
	int result = 1;
	char recv_str[128];

	SendCmd(huart, cmd, 1);

	if (check) {
		RecvResponce(huart, recv_str, 1000);

		if (!strstr(recv_str, check)) {
			result = 0;
		}
	}

	return result;
}

uint8_t SIMCOM_Init(UART_HandleTypeDef *huart) {
	char recv[64];
	char *pch;
	uint16_t len;

	LOG_SIMCOM(LEVEL_INFO, "Try initilize SIMCOM module bbbbbbbb\n\r");

	LOG_SIMCOM(LEVEL_DEBUG, "Try initilize communication between STM32 & SIMCOM module\n\r");
	if (SendCmd_Check(huart, "AT", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Initilization successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Try close ATE function\n\r");
	if (SendCmd_Check(huart, "ATE0", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Close successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Try disable calling number\n\r");
	if (SendCmd_Check(huart, "AT+CLIP=0", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Disable successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Try enable ATH\n\r");
	if (SendCmd_Check(huart, "AT+CVHU=0", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Enable successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Set SMS text mode\n\r");
	if (SendCmd_Check(huart, "AT+CMGF=1", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Set successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Set CREG mode\n\r");
	if (SendCmd_Check(huart, "AT+CREG=1", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Set successful!\n\r");
	} else {
		return 0;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Set SMS unicode symbols\n\r");
	if(SendCmd_Check(huart, "AT+CSMP=17,167,2,25", "OK")) {
		LOG_SIMCOM(LEVEL_DEBUG, "Set successful!\n\r");
	} else {
		return 0;
	}

    LOG_SIMCOM(LEVEL_DEBUG, "Set SMS unicode symbols\n\r");
    if(SendCmd_Check(huart, "AT+CSCS=\"UCS2\"", "OK")) {
        LOG_SIMCOM(LEVEL_DEBUG, "Set successful!\n\r");
    } else {
        return 0;
    }

	SendCmd(huart, "AT+CNUM", 1);
	RecvResponce(huart, recv, 1000);

	pch = (char*) recv;
	pch = strchr(pch, '\"');
	pch = strchr(pch + 1, '\"');
	pch = strchr(pch + 1, '\"');

	len = strchr(pch + 1, '\"') - pch - 1;
	pch++;

	memcpy(&modem.info.phone, pch, len);
	pch[12] = 0;
	LOG_SIMCOM(LEVEL_INFO, "Inited. Our number: %s\n\r", pch);

	return 1;
}

void SIMCOM_DeleteSMS(UART_HandleTypeDef *huart, uint8_t msg_num) {
	char cmd[16];
	memset(cmd, 0, 16);

	sprintf(cmd, "AT+CMGD=%d", msg_num);
	if (!SendCmd_Check(huart, cmd, "OK")) {
		LOG_SIMCOM(LEVEL_ERROR, "Message delete failed! (num %d)\n\r", msg_num);
	}
}

void SIMCOM_ClearSMS(UART_HandleTypeDef *huart) {
	char cmd[16];

	LOG_SIMCOM(LEVEL_INFO, "SIMCOM_ClearSMS start\n");

	sprintf(cmd, "AT+CMGD=%d, 4", 0);
	SendCmd(huart, cmd, 0);
	HAL_Delay(100);

	LOG_SIMCOM(LEVEL_INFO, "SIMCOM_ClearSMS finish\n");
}

int SIMCOM_ReadSMS(UART_HandleTypeDef *huart, SMS_STRUCT *msg) {
	int i, msg_num = 1;
	int count = 0;
	char *pch;
	byte_buffer_t *recv;
	uint16_t len;

    char temp[400];

	if (!modem.info.inited) {
		LOG_SIMCOM(LEVEL_INFO, "Do not read. Modem not initialized.\n");
		return 0;
	}

	recv = &modem.recv_buf;

	LOG_SIMCOM(LEVEL_DEBUG, "Check inbox SMS\n\r");

	SendCmd(huart, "AT+CMGL=\"ALL\"", 1);

	recv->ptr = RecvResponce(huart, recv->data, 4000);

	if (recv->ptr < 5) {
		LOG_SIMCOM(LEVEL_DEBUG, "No answer\n\r");
        modem.error_recv_cnt++;
        if (modem.error_recv_cnt > 20) {
            LOG_SIMCOM(LEVEL_ERROR, "Many SMS recv errors, deleting all messages.\n\r");
            SIMCOM_ClearSMS(huart);
            modem.error_recv_cnt = 0;
        }
		return 0;
	}
    modem.error_recv_cnt = 0;

	if (recv->ptr >= BYTE_BUF_SIZE - 1) {
		LOG_SIMCOM(LEVEL_ERROR, "Recv buffer is full, deleting all messages.\n\r");
		SIMCOM_ClearSMS(huart);
		return 0;
	}

	// if (!strncmp(recv, "\r\nERROR", 7)) {
	//     modem.error_cnt++;
	// } else {
	//     modem.error_cnt = 0;
	// }

	pch = (char*) recv->data;

	while ((pch = strstr(pch, "+CMGL")) != NULL && count < 3) {
		/* Cut index */
		msg_num = atoi(pch + 7);
		/* Cut Number */
		pch = strchr(pch, '\"');
		pch = strchr(pch + 1, '\"');
		pch = strchr(pch + 1, '\"');

		len = strchr(pch + 1, '\"') - pch - 1;
		if (len/4 > PHONE_SIZE) {
			LOG_SIMCOM(LEVEL_ERROR, "Bad number len. Message deleted!\n\r");
			SIMCOM_DeleteSMS(huart, msg_num);
			return 0;
		}
		strncpy(temp, pch + 1, len);

		memset(msg[count].number, 0, PHONE_SIZE);
        // memcpy(msg[count].number, temp, PHONE_SIZE);
        hex_to_phone(msg[count].number, temp, len);

		if (!check_number(msg[count].number)) {
			LOG_SIMCOM(LEVEL_ERROR, "Bad phone. Message deleted!\n\r");
			SIMCOM_ClearSMS(huart);
			// SIMCOM_DeleteSMS(huart, msg_num);
			return 0;
		}

		// Skip until message content
		pch = strchr(pch + 1, '\n');

		/* Cut message content */
		if (strstr(pch + 1, "+CMGL")) {
			len = strstr(pch + 1, "+CMGL") - pch - 1;
			if (len >= 400) {
				LOG_SIMCOM(LEVEL_ERROR, "Bad content 1. Message deleted %d!\n\r", len);
				SIMCOM_DeleteSMS(huart, msg_num);
				return 0;
			}

			strncpy(temp, pch + 1, len);
			temp[len] = 0;
            hex_to_russian(msg[count].content, temp, len);

			pch = strstr(pch + 1, "+CMGL");
		} else {
			len = strstr(pch + 1, "OK") - pch - 1;
			if (len >= 400) {
				LOG_SIMCOM(LEVEL_ERROR, "Bad content 2. Message deleted %d!\n\r", len);
				SIMCOM_DeleteSMS(huart, msg_num);
				return 0;
			}

			strncpy(temp, pch + 1, len);
			temp[len] = 0;
			hex_to_russian(msg[count].content, temp, len);

			pch = strstr(pch + 1, "OK");
		}

		count++;
		LOG_SIMCOM(LEVEL_DEBUG, "Deleting message!\n\r");
		SIMCOM_DeleteSMS(huart, msg_num);
	}

	if (count > 0) {
		LOG_SIMCOM(LEVEL_INFO, "Message\n\r");
		for (i = 0; i < count; i++) {
			LOG_SIMCOM(LEVEL_INFO, "From: %s\n", msg[i].number);
			LOG_SIMCOM(LEVEL_INFO, "Content: %s\n", msg[i].content);
		}
	} else if (recv->ptr > 20) {
		LOG_SIMCOM(LEVEL_INFO, "Long unknown data %s\n\r", recv->data);
		SIMCOM_ClearSMS(huart);
	}

	return count;
}

uint8_t SIMCOM_SendSMS(UART_HandleTypeDef *huart, char *number, char *content) {
	char cmd[512] = {0};
	char recv[32];
    char hex_num[64] = {0};
    char hex_data[400] = {0};

	if (!modem.info.inited) {
		LOG_SIMCOM(LEVEL_MAIN, "Do not sent. Modem not initialized.\n\r");
		return 0;
	}
    if (strlen(content) > 400) {
        LOG_SIMCOM(LEVEL_MAIN, "Do not sent. Message is too long.\n\r");
		return 0;
    }

	LOG_SIMCOM(LEVEL_DEBUG, "Send message to %s\n\r", number);

    phone_to_hex(hex_num, number, strlen(number));
    russian_to_hex(hex_data, content, strlen(content));

	strcpy(cmd, "AT+CMGS=\"");
	strcat(cmd, hex_num);
	strcat(cmd, "\"\r");
	SendCmd(huart, cmd, 0);
//	LOG_SIMCOM(LEVEL_INFO, "Cmd is: %s\n", cmd);
	HAL_Delay(200);

	memset(cmd, 0, 400);
	sprintf(cmd, "%s%c", hex_data, '\x1A');
	SendCmd(huart, cmd, 1);
//	LOG_SIMCOM(LEVEL_INFO, "Data is: %s\n", cmd);

	RecvResponce(huart, recv, 20000);

	if (strstr(recv, "OK")) {
		LOG_SIMCOM(LEVEL_MAIN, "Send successful!\n\r");
		return 1;
	} else if (strstr(recv, "ERROR")) {
		LOG_SIMCOM(LEVEL_ERROR, "Send failed! %s\n\r", recv);
		return 0;
	} else {
		LOG_SIMCOM(LEVEL_ERROR, "Send timeout 20s!\n\r");
		return 0;
	}
}

void SIMCOM_ReadStatus(UART_HandleTypeDef *huart) {
	char recv[32];
	char *pch;
    uint16_t len;

	if (!modem.info.inited) {
		LOG_SIMCOM(LEVEL_ERROR, "Do not ping. Modem not initialized.\n");
		return;
	}

	LOG_SIMCOM(LEVEL_DEBUG, "Check reg status\n\r");

	SendCmd(huart, "AT+CREG?", 1);

	len = RecvResponce(huart, recv, 1000);

	pch = recv;

	if (len > 16 && strncmp(pch, "\r\n+CREG", 7) == 0) {
		pch += 0;
		pch = strchr(pch + 1, ',');
		modem.info.status = (uint8_t) strtoul(pch + 1, NULL, 10);
		LOG_SIMCOM(LEVEL_DEBUG, "Status is: %d\r\n", modem.info.status);
	} else {
        modem.info.status = 0;
    }

	if (modem.info.status == 1 || modem.info.status == 5) {
		modem.error_cnt = 0;
	} else {
		modem.error_cnt++;
	}
}
