//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: function.c
// Description: various function routine.
//***********************************************************************************
#include "general.h"
#include <time.h>

uint8_t Alarm_ffs(uint16_t num) 
{
    uint8_t  i;

    for( i = 8; i > 0 ; --i){//Alarm flag Max = 0x01ff
        if(num & (1 << i))
            return i;
    }
    return 0;
}


//***********************************************************************************
//
// Protocol transferred meaning.
//
//***********************************************************************************
uint16_t Protocol_escape(uint8_t *pObj, uint8_t *pSou, uint16_t length)
{
    uint16_t i;
    uint16_t len = 0;

    for (i = 0; i < length; i++) {
        if (*pSou == PROTOCOL_TOKEN) {
            *pObj = PROTOCOL_TRANSFER;
            pObj++;
            len++;
            *pObj = 0x02;
        } else if (*pSou == PROTOCOL_TRANSFER) {
            *pObj = PROTOCOL_TRANSFER;
            pObj++;
            len++;
            *pObj = 0x01;
        } else {
            *pObj = *pSou;
        }
        pSou++;
        pObj++;
        len++;
    }

    return len;
}

//***********************************************************************************
//
// Protocol recover transferred meaning.
//
//***********************************************************************************
uint16_t Protocol_recover_escape(uint8_t *pObj, uint8_t *pSou, uint16_t length)
{
    uint16_t i;
    uint16_t len = 0;

    for (i = 0; i < length; i++) {
        if (*pSou == PROTOCOL_TRANSFER) {
            pSou++;
            i++;
            if (*pSou == 0x01) {
                *pObj = PROTOCOL_TRANSFER;
            } else if (*pSou == 0x02) {
                *pObj = PROTOCOL_TOKEN;
            }
        } else {
            *pObj = *pSou;
        }
        pSou++;
        pObj++;
        len++;
    }

    return len;
}

#ifdef SUPPORT_CRC16
//***********************************************************************************
//
// CRC16-CCITT function. Here defined two way to achieve.
//   1, Software, include MSB and LSB algorithm.
//   2, Hardware, use MSP430 CRC module, it use LSB bit order
//
//***********************************************************************************
uint16_t CRC16(uint8_t *pData,  uint16_t length)
{
#ifdef CRC16_HW_MODULE
    uint16_t i;

    CRC_setSeed(CRC_BASE, CRC_SEED);
    for (i = 0; i < length; i++) {
        //Add all of the values into the CRC signature
        CRC_set8BitData(CRC_BASE, pData[i]);
    } 

    //To in accordance with software method result, we use CRC_getResultBitsReversed(). 
    return CRC_getResultBitsReversed(CRC_BASE);

#else   /* Software method */

    uint16_t i, j, crc = CRC_SEED;

#ifdef CRC16_MSB
    for (i = 0; i < length; i++) {
        crc ^= pData[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
            else
                crc <<= 1;
        } 
    } 
#elif defined(CRC16_LSB)
    for (i = 0; i < length; i++) {
        crc ^= pData[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ CRC_POLYNOMIAL_REVERS;
            else
                crc >>= 1;
        }
    }
#endif

    return crc;

#endif  /* CRC16_HW_MODULE */
}

/* Notice:
 * Just for Upgrade CRC16
 * void SetContinueCRC16(void)
 * uint16_t ContinueCRC16(uint8_t *pData,  uint16_t length)
 * */
static  uint16_t continueCrc;



//***********************************************************************************
// set the continueCrc seed
//***********************************************************************************
void SetContinueCRC16(void)
{
#if 0 //def CRC16_HW_MODULE
    CRC_setSeed(CRC_BASE, CRC_SEED);
#else   /* Software method */
    continueCrc = CRC_SEED;
#endif
}

//***********************************************************************************
//
// CRC16-CCITT function. Here defined two way to achieve.
//   1, Software, include MSB and LSB algorithm.
//   2, Hardware, use MSP430 CRC module, it use LSB bit order
//
//***********************************************************************************
uint16_t ContinueCRC16(uint8_t *pData,  uint16_t length)
{
#if 0 //def CRC16_HW_MODULE
    uint16_t i;

    for (i = 0; i < length; i++) {
        //Add all of the values into the CRC signature
        CRC_set8BitData(CRC_BASE, pData[i]);
    } 

    //To in accordance with software method result, we use CRC_getResultBitsReversed(). 
    return CRC_getResultBitsReversed(CRC_BASE);

#else   /* Software method */

    uint16_t i, j;

#if 0 //def CRC16_MSB
    for (i = 0; i < length; i++) {
        continueCrc ^= pData[i] << 8;
        for (j = 0; j < 8; j++) {
            if (continueCrc & 0x8000)
                continueCrc = (continueCrc << 1) ^ CRC_POLYNOMIAL;
            else
                continueCrc <<= 1;
        } 
    } 
#else //elif defined(CRC16_LSB)
    for (i = 0; i < length; i++) {
        continueCrc ^= pData[i];
        for (j = 0; j < 8; j++) {
            if (continueCrc & 0x0001)
                continueCrc = (continueCrc >> 1) ^ CRC_POLYNOMIAL_REVERS;
            else
                continueCrc >>= 1;
        }
    }
#endif

    return continueCrc;

#endif  /* CRC16_HW_MODULE */
}

#endif  /* SUPPORT_CRC16 */

//***********************************************************************************
//
// check code 8 bit function.
//
//***********************************************************************************
uint8_t CheckCode8(uint8_t *pData,  uint16_t length)
{
    uint8_t cc = 0;

    while (length) {
        cc ^= *pData;
        pData++;
        length--;
    }

    return cc;
}

//***********************************************************************************
//
// Circular queue.
//
//***********************************************************************************
static ErrorStatus Queue_is_full(QueueDef *queue)
{
    if (((queue->rear + 1) % queue->objNumber) == queue->front)
        return ES_SUCCESS; 	// Queue full.
    else
        return ES_ERROR; 	// Queue not full.
}

static ErrorStatus Queue_is_empty(QueueDef *queue)
{
    if (queue->rear == queue->front)
        return ES_SUCCESS; 	// Queue empty.
    else
        return ES_ERROR; 	// Queue not empty.
}

ErrorStatus EnQueue(QueueDef *queue, uint8_t *obj)
{
    uint16_t i;

    if (Queue_is_full(queue) == ES_SUCCESS) {
        // Queue full.
        return ES_ERROR;
    }

    for (i = 0; i < queue->objSize; i++) {
        queue->objData[queue->rear * queue->objSize + i] = obj[i];
    }
    queue->rear = (queue->rear + 1) % queue->objNumber;

    return ES_SUCCESS;
}

ErrorStatus DeQueue(QueueDef *queue, uint8_t *obj)
{
    uint16_t i;

    if (Queue_is_empty(queue) == ES_SUCCESS) {
        // Queue empty.
        return ES_ERROR;
    }

    for (i = 0; i < queue->objSize; i++) {
        obj[i] = queue->objData[queue->front * queue->objSize + i];
    }
    queue->front = (queue->front + 1) % queue->objNumber;

    return ES_SUCCESS;
}

ErrorStatus Queue_recover_one_obj(QueueDef *queue)
{
    queue->front = (queue->front - 1) % queue->objNumber;
    return ES_SUCCESS;
}

ErrorStatus Queue_drop_one_obj(QueueDef *queue)
{
    queue->front = (queue->front + 1) % queue->objNumber;
    return ES_SUCCESS;
}

//***********************************************************************************
//
// Hex string to int.
// For example: "08ef" to 2287, "0x8ef" to 2287, "8ef," to 2287
//
//***********************************************************************************
uint32_t htoi(char *str)
{
    uint8_t i;
    uint32_t num;

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        i = 2;
    else
        i = 0;

    for (num = 0; (str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'f') || (str[i] >='A' && str[i] <= 'F'); i++)
    {
        if (str[i] <= '9')
            num = 16 * num + (str[i] - '0');
        else if (str[i] <= 'F')
            num = 16 * num + (10 + str[i] - 'A');
        else
            num = 16 * num + (10 + str[i] - 'a');
    }

    return num;
}

uint8_t* mystrchar(uint8_t *buf, uint8_t c, uint16_t len)
{
    uint16_t i;

    for ( i = 0; i < len; i++) {
        if(buf[i] == c)
            return &buf[i];
    }
    return NULL;
}

//***********************************************************************************
// 
// brief: transform hex to bcd
// 
//***********************************************************************************
uint8_t TransHexToBcd(uint8_t hex)
{
    uint8_t temp;
    temp = ((hex / 10)<<4) +(hex % 10);
    return temp;
}

//***********************************************************************************
// 
// brief: transform bcd to hex
// 
//***********************************************************************************
uint8_t TransBcdToHex(uint8_t bcd)
{
    uint8_t temp;
    temp = (bcd & 0x0f) + (bcd >> 4) * 10;
    return temp;
}
