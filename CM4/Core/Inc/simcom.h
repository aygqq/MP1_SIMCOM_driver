#ifndef _SIMCOM_H
#define _SIMCOM_H

#define SIM_STATE_READ      1
#define SIM_STATE_RESET     2
#define SIM_STATE_INIT      3
#define SIM_STATE_DELAY     4

typedef struct SMS_STRUCT_T {
    char number[16];
    char content[400];
} SMS_STRUCT;

uint8_t SIMCOM_Init();

int  SIMCOM_ReadSMS(UART_HandleTypeDef *huart, SMS_STRUCT sms[3]);
uint8_t SIMCOM_SendSMS(UART_HandleTypeDef *huart, char *number, char *content);
void SIMCOM_ReadStatus(UART_HandleTypeDef *huart);

void SIMCOM_Test(char c);
#endif
