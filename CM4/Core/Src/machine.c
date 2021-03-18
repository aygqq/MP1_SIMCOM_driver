#include "debug.h"
#include "utils_crc.h"

#include "sms_modem.h"
#include "machine.h"

pc_t pc;

extern sms_modem_t modem;

extern VIRT_UART_HandleTypeDef huart0;
extern VIRT_UART_HandleTypeDef huart1;

/*
 Some functions to transmitt data to PC
 */
void pc_send_short(uint8_t type, uint8_t data) {
	uint16_t crc16;
	uint8_t ptr = 0;

	pc.send_buf.data[ptr++] = type;
	pc.send_buf.data[ptr++] = 1;
	pc.send_buf.data[ptr++] = 0;
	pc.send_buf.data[ptr++] = data;

	crc16 = calcCrc16((uint8_t*) pc.send_buf.data, ptr);
	pc.send_buf.data[ptr++] = crc16 & 0x00ff;
	pc.send_buf.data[ptr++] = (crc16 & 0xff00) >> 8;
	pc.send_buf.data[ptr++] = 0xFE;
	pc.send_buf.ptr = ptr;

	if (pc.send_buf.data[ptr - 2] == 0xFE) {
		pc.send_buf.data[ptr - 2] = 0xFD;
	}
	if (pc.send_buf.data[ptr - 3] == 0xFE) {
		pc.send_buf.data[ptr - 3] = 0xFD;
	}

	LOG_PC(LEVEL_INFO, "Send short\r\n");
	VIRT_UART_Transmit(&huart0, (uint8_t*) pc.send_buf.data, pc.send_buf.ptr);
}

void pc_send_data(uint8_t type, uint8_t *buf_in, uint16_t length) {
	uint16_t crc16;
	uint8_t ptr = 0;

	pc.send_buf.data[ptr++] = type;
	pc.send_buf.data[ptr++] = length;
	pc.send_buf.data[ptr++] = length >> 8;
	memcpy(&pc.send_buf.data[ptr], buf_in, length);
	ptr += length;

	crc16 = calcCrc16((uint8_t*) pc.send_buf.data, ptr);
	pc.send_buf.data[ptr++] = crc16 & 0x00ff;
	pc.send_buf.data[ptr++] = (crc16 & 0xff00) >> 8;
	pc.send_buf.data[ptr++] = 0xFE;
	pc.send_buf.ptr = ptr;

	if (pc.send_buf.data[ptr - 2] == 0xFE) {
		pc.send_buf.data[ptr - 2] = 0xFD;
	}
	if (pc.send_buf.data[ptr - 3] == 0xFE) {
		pc.send_buf.data[ptr - 3] = 0xFD;
	}

	LOG_PC(LEVEL_INFO, "Send data\r\n");
	VIRT_UART_Transmit(&huart0, (uint8_t*) pc.send_buf.data, pc.send_buf.ptr);
}

void pc_send_sms(uint8_t type, phone_number_t *phone, char *data) {
	LOG_PC(LEVEL_MAIN, "Send SMS\r\n");
	uint8_t buf[512];
	uint16_t len, ptr = 0;
	len = strlen(data);

	memcpy(&buf[ptr], phone, PHONE_SIZE);
	ptr += PHONE_SIZE;
	memcpy(&buf[ptr], data, len);

	pc_send_data(CMD_OUT_SMS, buf, 2 + len + PHONE_SIZE);
}

void pc_recv_pc_ready(char *data, uint16_t length) {
	LOG_PC(LEVEL_MAIN, "Recv PC ready\r\n");

	pc_send_short(CMD_PC_READY, 1);
}

void pc_recv_send_sms(char *data, uint16_t length) {
//    uint8_t type;
	phone_number_t phone;
	int ptr = 0;
	uint8_t res = 0;

	LOG_PC(LEVEL_MAIN, "Recv SMS send\r\n");

	ptr++;
	ptr++;
//    type = data[ptr++];
	memcpy(&phone, &data[ptr], PHONE_SIZE);
	ptr += PHONE_SIZE;

	res = sms_send(&phone, &data[ptr]);

	pc_send_short(CMD_SEND_SMS, res);
}

void pc_recv_req_modem_info(char *data, uint16_t length) {
	uint8_t buf[128];

	LOG_PC(LEVEL_MAIN, "Recv req modem info\r\n");

	memcpy(buf, &modem.info, sizeof(modem_info_t));
	pc_send_data(CMD_REQ_MODEM_INFO, buf, sizeof(modem_info_t));
}

void pc_recv_req_conn_info(char *data, uint16_t length) {

}

int pc_recv_handler(char *buf, uint32_t length) {
	cmd_header_t *hdr = (cmd_header_t*) buf;
	char *data;
	uint16_t crc_calc;

	crc_calc = calcCrc16((uint8_t*) buf, (hdr->length + HDR_SIZE + CRC_SIZE));
	if (crc_calc != 0) {
		LOG_PC(LEVEL_ERROR, "Bad crc\r\n");
		return 1;
	}

	data = buf + HDR_SIZE;
	data[hdr->length] = 0;

	switch (hdr->cmd_type) {
	case CMD_PC_READY:
		pc_recv_pc_ready(data, hdr->length);
		break;
	case CMD_REQ_MODEM_INFO:
		pc_recv_req_modem_info(data, hdr->length);
		break;
	case CMD_REQ_CONN_INFO:
		pc_recv_req_conn_info(data, hdr->length);
		break;
	case CMD_SEND_SMS:
		pc_recv_send_sms(data, hdr->length);
		break;
	default:
		LOG_PC(LEVEL_ERROR, "Wrong packet type\r\n");
		break;
	}

	return 0;
}
