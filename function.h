//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: function.h
// Description: various function routine head.
//***********************************************************************************

#ifndef __ZKSIOT_FUNCTION_H__
#define __ZKSIOT_FUNCTION_H__

#ifdef LITTLE_ENDIAN
// little endian
#define	HIBYTE_ZKS(w)		(((uint8_t *)&w)[1])
#define	LOBYTE_ZKS(w)		(((uint8_t *)&w)[0])
#define	HIWORD_ZKS(w)		(((uint16_t *)&w)[1])
#define	LOWORD_ZKS(w)		(((uint16_t *)&w)[0])
#else
// 	big endian
#define	HIBYTE_ZKS(w)		(((uint8_t*)&w)[0])
#define	LOBYTE_ZKS(w)		(((uint8_t*)&w)[1])
#define	HIWORD_ZKS(w)		(((uint16_t*)&w)[0])
#define	LOWORD_ZKS(w)		(((uint16_t*)&w)[1])
#endif

#define sizeofarray(x)  (sizeof(x)/sizeof(x[0]))

#define PROTOCOL_TOKEN              0x7e
#define PROTOCOL_TRANSFER           0x7d


#define CRC_SEED                    0xffff
#define CRC_POLYNOMIAL              0x1021
#define CRC_POLYNOMIAL_REVERS       0x8408

typedef enum {
    ES_SUCCESS = 0,
    ES_ERROR = 1
} ErrorStatus;

typedef struct {
    uint8_t *objData;
    uint16_t objSize;
    uint16_t objNumber;
    uint32_t front;
    uint32_t rear;
} QueueDef;

extern uint8_t Alarm_ffs(uint16_t num);
extern uint16_t Protocol_escape(uint8_t *pObj, uint8_t *pSou, uint16_t length);
extern uint16_t Protocol_recover_escape(uint8_t *pObj, uint8_t *pSou, uint16_t length);
extern uint16_t CRC16(uint8_t *pData,  uint16_t length);
extern uint8_t CheckCode8(uint8_t *pData,  uint16_t length);
extern ErrorStatus EnQueue(QueueDef *queue, uint8_t *obj);
extern ErrorStatus DeQueue(QueueDef *queue, uint8_t *obj);
extern uint32_t htoi(char *str);
extern void SetContinueCRC16(void);
extern uint16_t ContinueCRC16(uint8_t *pData,  uint16_t length);
extern uint8_t* mystrchar(uint8_t *buf, uint8_t c, uint16_t len);
extern uint8_t TransHexToBcd(uint8_t hex);
extern uint8_t TransBcdToHex(uint8_t bcd);
extern uint16_t CrcCheckCode_JSLL(uint8_t *pData, uint16_t length);
extern uint32_t RtcTransUtc(uint8_t *timeTemp);
extern uint16_t docrc16(uint8_t *pSrcData, uint16_t lenght);
extern void TimeTransformUtc(uint8_t *timeTemp, uint32_t *utcTemp);
#endif	/* __ZKSIOT_FUNCTION_H__ */

