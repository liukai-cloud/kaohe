#ifndef __SERIAL_H
#define __SERIAL_H

extern char Serial_TxPacket[4];
extern char Serial_RxPacket[100];

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array,uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t number,uint8_t L);
void Serial_SendPacket(void);
uint8_t Serial_GetRxFlag(void);

#endif