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

