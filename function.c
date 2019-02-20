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

#ifdef  JSLL_PROJECT

const uint16_t crc16_ccitt_table[] = { 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a,
    0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528,
    0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719,
    0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e,
    0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf,
    0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec,
    0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd,
    0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2,
    0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3,
    0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691,
    0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806,
    0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37,
    0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64,
    0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55,
    0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

uint16_t docrc16(uint8_t *pSrcData, uint16_t length)
{
    uint16_t crc_reg = 0x0000, i;
    for (i = 0; i < length; i++) {
        crc_reg =  crc16_ccitt_table[(crc_reg ^ pSrcData[i]) & 0xFF] ^ (crc_reg >> 8);
    }
    return crc_reg;
}



// 2000年-2099年的闰年标记与第一天的星期
static const uint8_t Table_YearFirstDay[] =
{
    0x0e,0x01,0x02,0x03,0x0c,0x06,0x00,0x01,0x0a,0x04,
    0x05,0x06,0x08,0x02,0x03,0x04,0x0d,0x00,0x01,0x02,
    0x0b,0x05,0x06,0x00,0x09,0x03,0x04,0x05,0x0e,0x01,
    0x02,0x03,0x0c,0x06,0x00,0x01,0x0a,0x04,0x05,0x06,
    0x08,0x02,0x03,0x04,0x0d,0x00,0x01,0x02,0x0b,0x05,
    0x06,0x00,0x09,0x03,0x04,0x05,0x0e,0x01,0x02,0x03,
    0x0c,0x06,0x00,0x01,0x0a,0x04,0x05,0x06,0x08,0x02,
    0x03,0x04,0x0d,0x00,0x01,0x02,0x0b,0x05,0x06,0x00,
    0x09,0x03,0x04,0x05,0x0e,0x01,0x02,0x03,0x0c,0x06,
    0x00,0x01,0x0a,0x04,0x05,0x06,0x08,0x02,0x03,0x04,
};
////////////////////////////////////////////////////////
static const uint16_t Table_Monthdyas[] =
{
//  31,28,31,30,31,30,31,31,30,31,30,31
    0,0,31,59,90,120,151,181,212,243,273,304,334,365
};


////////////////////////////////////////////////////////
static const uint32_t Table_YearFirstDayUTC[101] =
{
    946684800,978307200,1009843200,1041379200,1072915200,1104537600,1136073600,1167609600,1199145600,1230768000,
    1262304000,1293840000,1325376000,1356998400,1388534400,1420070400,1451606400,1483228800,1514764800,1546300800,
    1577836800,1609459200,1640995200,1672531200,1704067200,1735689600,1767225600,1798761600,1830297600,1861920000,
    1893456000,1924992000,1956528000,1988150400,2019686400,2051222400,2082758400,2114380800,2145916800,2177452800,
    2208988800,2240611200,2272147200,2303683200,2335219200,2366841600,2398377600,2429913600,2461449600,2493072000,
    2524608000,2556144000,2587680000,2619302400,2650838400,2682374400,2713910400,2745532800,2777068800,2808604800,
    2840140800,2871763200,2903299200,2934835200,2966371200,2997993600,3029529600,3061065600,3092601600,3124224000,
    3155760000,3187296000,3218832000,3250454400,3281990400,3313526400,3345062400,3376684800,3408220800,3439756800,
    3471292800,3502915200,3534451200,3565987200,3597523200,3629145600,3660681600,3692217600,3723753600,3755376000,
    3786912000,3818448000,3849984000,3881606400,3913142400,3944678400,3976214400,4007836800,4039372800,4070908800,
    4102444800,
};

void TimeTransformUtc(uint8_t *timeTemp, uint32_t *utcTemp)
{
    uint32_t    CurdayCnt;
    uint32_t    utcTemp2;
    utcTemp2        = Table_YearFirstDayUTC[TransBcdToHex(timeTemp[0])]; //
    
    CurdayCnt       = Table_Monthdyas[TransBcdToHex(timeTemp[1])];

    // 闰年且大于2月多加一天
    if((TransBcdToHex(timeTemp[1]) > 2) && (Table_YearFirstDay[TransBcdToHex(timeTemp[0])] & 0x08))
        CurdayCnt++;
    CurdayCnt       += TransBcdToHex(timeTemp[2]) - 1;

// 当前时区的UTC时间
    utcTemp2            = utcTemp2 + (uint32_t)CurdayCnt * 86400 
                        + ((uint32_t)TransBcdToHex(timeTemp[3])*60 + (uint32_t)TransBcdToHex(timeTemp[4]))*60 + TransBcdToHex(timeTemp[5]);

    *utcTemp        = utcTemp2 - 8 * 3600;
}



#endif //JSLL_PROJECT
