//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by min.zeng, 2017.10.24
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: max31855.c
// Description: Display API function process routine.
//***********************************************************************************
#include "../general.h"
#ifdef SUPPORT_DEEPTEMP
#include "max31855.h"

//Use software spi
#define MAX31855_POWER_PIN               IOID_21
#define MAX31855_CS_PIN                  IOID_20

#define MAX31855_CLK_PIN                  IOID_28
#define MAX31855_MISO_PIN                 IOID_29




#define Max31855_spiClkCtrl(on)          PIN_setOutputValue(max31855PinHandle, MAX31855_CLK_PIN, on)
#define Max31855_spiCsCtrl(on)           PIN_setOutputValue(max31855PinHandle, MAX31855_CS_PIN, on)
#define Max31855_spiVccCtrl(on)          PIN_setOutputValue(max31855PinHandle, MAX31855_POWER_PIN, !on)



const PIN_Config max31855PinTable[] = {
    MAX31855_POWER_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    MAX31855_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    MAX31855_CLK_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    MAX31855_MISO_PIN | PIN_INPUT_EN | PIN_GPIO_LOW | PIN_PULLDOWN | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};

static PIN_State   max31855PinState;
static PIN_Handle  max31855PinHandle;


typedef struct{
    int32_t tempdeep;
} SensorData_t;



static SensorData_t rSensorData[MAX31855_SPI_MAX];


/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * @return:
 ******************************************************************************/
signed char Max31855_poweron(void) {

    Max31855_spiVccCtrl(1);
    Task_sleep(300 * CLOCK_UNIT_MS);
    Max31855_spiCsCtrl(0);
    Max31855_delay(20);
    Max31855_spiClkCtrl(0);
    Max31855_delay(20);
    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * @return:
 ******************************************************************************/
signed char Max31855_powerdown(void) {

    Max31855_spiCsCtrl(1) ;
    Max31855_delay(20);
    Max31855_spiClkCtrl(0);
    Max31855_delay(20);
    Max31855_spiVccCtrl(0);
    Max31855_delay(20);
    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * @return:
 ******************************************************************************/
signed char Max31855_spi_read_init(void) {

    Max31855_spiCsCtrl(1) ;
    Max31855_delay(20);
    Max31855_spiCsCtrl(1) ;
    Max31855_delay(20);
    Max31855_spiClkCtrl(0);


    Max31855_delay(60);
    Max31855_spiCsCtrl(0);
    Max31855_delay(20);
    Max31855_spiCsCtrl(0);
    Max31855_delay(20);
    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * @return:
 ******************************************************************************/
signed char Max31855_delay(uint8_t cnt) {
    uint8_t i = 0;
   for(i = 0; i < cnt;i++)
   {

   }

    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @param:
 * @return:
 ******************************************************************************/
signed char Max31855_readdata(uint16_t *junction,uint16_t *thermocouple ) {
    uint8_t i = 0;
    *thermocouple = 0;
    *junction = 0;
    for (i=0; i<=31; i++) {
        Max31855_spiClkCtrl(1);
        Max31855_delay(20);

        if ((PIN_getInputValue(MAX31855_MISO_PIN))!=0) {
          if (i<16) {
              (*thermocouple) = (*thermocouple)|(1<<(15-i));
            }else {
              (*junction) = (*junction)|(1<<(31-i));
            }
        }

        Max31855_spiClkCtrl(0);
        Max31855_delay(20);

    }

    System_flush();
    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
signed char MAX31855_DataCorrect(uint8_t *Datastate,uint16_t junction,uint16_t thermocouple ) {
    if ((junction&0x01)!=0) {//no connections
        *Datastate|=0x01;
        return -1;
    }
    if ((junction&0x02)!=0) {//GND erro
        *Datastate|=0x02;
        return -2;
    }
    if ((junction&0x04)!=0) {//VCC erro
        *Datastate|=0x04;
        return -3;
    }
    if ((junction&0x06)!=0) {//data erro
        return -4;
    }
    if ((thermocouple&0x01)!=0) {//thermocouple hard erro
        *Datastate|=0x08;
        return -5;
    }
    if ((thermocouple&0x02)!=0) {//thermocouple data erro
        return -6;
    }
    return 0;
}
/*******************************************************************************
 * @fn:
 * @brief:      热电偶温度，T = 0.25*x - 2048
 * @para:
 * @return:
 ******************************************************************************/
unsigned int MAX31855_MapRawThermocoupleTemp(unsigned int input) {

    unsigned int thermocouple = input >> 2;
    thermocouple &= 0x3FFF;

    if (thermocouple >= 0x2000) {
        thermocouple -= 0x2000;
    } else {
        thermocouple += 0x2000;
    }

    thermocouple |= (input & 0x0003) << 14;

    return thermocouple;
}

/*******************************************************************************
 * @fn:
 * @brief:      芯片温度，T = 0.0625*x - 128
 * @para:
 * @return:
 ******************************************************************************/
unsigned int MAX31855_MapColdJunctionTemp(unsigned int input) {

    unsigned int junction = input >> 4;
    junction &= 0x0FFF;

    if (junction >= 0x0800) {
        junction -= 0x0800;
    } else {
        junction += 0x0800;
    }

    junction |= (input & 0x000F) << 12;

    return junction;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
void MAX31855_qsort(MAX31855_MeasureData md[], uint8_t nbrOfSample) {

    uint8_t cnt = 0;
    MAX31855_MeasureData tmp;               // 用于交换时的缓存
    for (cnt = 1; cnt < nbrOfSample; cnt++) {

        // 按照温度升序进行排列
        if (md[0].temp > md[cnt].temp) {
            memcpy(&tmp, md + cnt, sizeof(MAX31855_MeasureData));
            memcpy(md + cnt, md + 0, sizeof(MAX31855_MeasureData));
            memcpy(md + 0, &tmp, sizeof(MAX31855_MeasureData));
        }
    }

    for (cnt = 2; cnt < nbrOfSample; cnt++) {

        // 按照温度升序进行排列
        if (md[1].temp > md[cnt].temp) {
            memcpy(&tmp, md + cnt, sizeof(MAX31855_MeasureData));
            memcpy(md + cnt, md + 1, sizeof(MAX31855_MeasureData));
            memcpy(md + 1, &tmp, sizeof(MAX31855_MeasureData));
        }
    }

    for (cnt = 3; cnt < nbrOfSample; cnt++) {

        // 按照温度升序进行排列
        if (md[2].temp > md[cnt].temp) {
            memcpy(&tmp, md + cnt, sizeof(MAX31855_MeasureData));
            memcpy(md + cnt, md + 2, sizeof(MAX31855_MeasureData));
            memcpy(md + 2, &tmp, sizeof(MAX31855_MeasureData));
        }
    }

    for (cnt = 4; cnt < nbrOfSample; cnt++) {

        // 按照温度升序进行排列
        if (md[3].temp > md[cnt].temp) {
            memcpy(&tmp, md + cnt, sizeof(MAX31855_MeasureData));
            memcpy(md + cnt, md + 3, sizeof(MAX31855_MeasureData));
            memcpy(md + 3, &tmp, sizeof(MAX31855_MeasureData));
        }
    }

}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
double MAX31855_CalcTemp(double thermocouple, double junction) {

    double linTemp = 0;

//inverse coefficients      //coefficients
//volt to temp              //temp to volts
    double a0 = 0;
    double b0 = 0;
    double a1 = 0;
    double b1 = 0;
    double a2 = 0;
    double b2 = 0;
    double a3 = 0;
    double b3 = 0;
    double a4 = 0;
    double b4 = 0;
    double a5 = 0;
    double b5 = 0;
    double a6 = 0;
    double b6 = 0;
    double a7 = 0;
    double b7 = 0;
    double a8 = 0;
    double b8 = 0;
    double a9 = 0;
    double b9 = 0;
    double a10 = 0;
    double b10 = 0;
    double b11 = 0;
    double b12 = 0;
    double b13 = 0;
    double b14 = 0;

//exponential
    double c0 = 0;
    double c1 = 0;
    double c2 = 0;

    double mVoltsPerC = 0;
    double mVolts = 0;
    double coldmVolts = 0;
    double value = 0;
    bool outOfRange = false;

    mVoltsPerC = 0.05218;

    if (junction >= -270 && junction < 0) {
        b0 = 0;
        b1 = 0.038748106364;
        b2 = 0.000044194434347;
        b3 = 0.00000011844323105;
        b4 = 0.000000020032973554;
        b5 = 9.0138019559E-10;
        b6 = 2.2651156593E-11;
        b7 = 3.6071154205E-13;
        b8 = 3.8493939883E-15;
        b9 = 2.8213521925E-17;
        b10 = 1.4251594779E-19;
        b11 = 4.8768662286E-22;
        b12 = 1.079553927E-24;
        b13 = 1.3945027062E-27;
        b14 = 7.9795153927E-31;
    } else if (junction >= 0 && junction < 400) {
        b0 = 0;
        b1 = 0.038748106364;
        b2 = 0.00003329222788;
        b3 = 0.00000020618243404;
        b4 = -0.0000000021882256846;
        b5 = 1.0996880928E-11;
        b6 = -3.0815758772E-14;
        b7 = 4.547913529E-17;
        b8 = -2.7512901673E-20;
    } else {
        outOfRange = true;
    }

    if (outOfRange == false) {
        //1. Subtract cold junction temperature value from MAX31855 temperature data.
        value = thermocouple - junction;
        //2. Calculate the thermocouple voltage based on the MAX31855抯 uV/癈 for that thermocouple type.
        mVolts = value * mVoltsPerC;
        //3. Calculate the cold junction equivalent thermocouple voltage using the NIST temperature-to-voltage coefficients.
        coldmVolts = b0 + b1 * junction + b2 * pow(junction, 2)
                + b3 * pow(junction, 3) + b4 * pow(junction, 4)
                + b5 * pow(junction, 5) + b6 * pow(junction, 6)
                + b7 * pow(junction, 7) + b8 * pow(junction, 8)
                + b9 * pow(junction, 9) + b10 * pow(junction, 10)
                + b11 * pow(junction, 11) + b12 * pow(junction, 12)
                + b13 * pow(junction, 13) + b14 * pow(junction, 14)
                + c0 * exp(c1 * pow(junction - c2, 2));
        //4. Add the cold junction equivalent thermocouple voltage calculated in step 3 to the thermocouple voltage calculated in step 2.
        mVolts = mVolts + coldmVolts;

        //get inverse coeddicients to convert new mVolt value to temperature

        //T Type
        mVoltsPerC = 0.05218;

        if (mVolts >= -5.603 && mVolts < 0) {
            a0 = 0;
            a1 = 2.5949192E1;
            a2 = -2.1316967E-1;
            a3 = 7.9018692E-1;
            a4 = 4.2527777E-1;
            a5 = 1.3304473E-1;
            a6 = 2.0241446E-2;
            a7 = 1.2668171E-3;
        } else if (mVolts >= 0 && mVolts <= 20.872) {
            a0 = 0;
            a1 = 2.592800E1;
            a2 = -7.602961E-1;
            a3 = 4.637791E-2;
            a4 = -2.165394E-3;
            a5 = 6.048144E-5;
            a6 = -7.293422E-7;
        } else {
            outOfRange = true;
        }

        if (outOfRange == false) {
            //5. Use the result of step 4 and the NIST voltage-to-temperature coefficients (the 搃nverse?coefficients) to calculate the cold-junction-compensated, linearized temperature value.
            linTemp = a0 + a1 * mVolts + a2 * pow(mVolts, 2)
                    + a3 * pow(mVolts, 3) + a4 * pow(mVolts, 4)
                    + a5 * pow(mVolts, 5) + a6 * pow(mVolts, 6)
                    + a7 * pow(mVolts, 7) + a8 * pow(mVolts, 8)
                    + a9 * pow(mVolts, 9) + a10 * pow(mVolts, 10);
        } else {
            linTemp = 0;
        }

    } else
        linTemp = 0;

    return linTemp;
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
static void Max31855_init(uint8_t chNum)
{
    max31855PinHandle = PIN_open(&max31855PinState, max31855PinTable);
}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
static void Max31855_measure(uint8_t chNum)
{

    uint16_t value_thermocouple  = 0;
    uint16_t value_junction = 0;
    uint8_t cntOfSample = 0;            // 计数采样的次数
    uint8_t scsOfSample = 0;            // 计数采样成功的次数
    uint8_t cnt = 0;                   // 循环计数单元
    uint8_t Datastate = 0;
    MAX31855_MeasureData md[5];
    Max31855_poweron();

    for(cntOfSample=0;cntOfSample<10;cntOfSample++){
        Max31855_spi_read_init();
        Max31855_readdata(&value_junction,&value_thermocouple);
        if(MAX31855_DataCorrect(&Datastate,value_junction,value_thermocouple)<0) {
            continue;                       // 读取温度数据失败
        }
        md[scsOfSample].index = scsOfSample;
        md[scsOfSample].sd.thermocouple = MAX31855_MapRawThermocoupleTemp(value_thermocouple);
        md[scsOfSample].sd.junction = MAX31855_MapColdJunctionTemp(value_junction);

        if (++scsOfSample >= 5) {       // 成功采集了5次，就退出
            break;
        }
    }
    Max31855_powerdown();
    if (scsOfSample < 5) {
        rSensorData[chNum].tempdeep = 0x000000ff&Datastate;
        return ; // 10次采集中，采集成功的次数小于5次
    }
    // 如果5次采样值都是一样的，就不需要进行大量的计算得到温度值，再进行排序了。
    uint32_t orMask0 = (uint32_t) md[0].sd.thermocouple << 16
            | (uint32_t) md[0].sd.junction;
    uint32_t orMask = orMask0, andMask0 = orMask0, andMask = orMask0;
    for (cnt = 1; cnt < 5; cnt++) {
        orMask |= (uint32_t) md[cnt].sd.thermocouple << 16
                | (uint32_t) md[cnt].sd.junction;
        andMask &= (uint32_t) md[cnt].sd.thermocouple << 16
                | (uint32_t) md[cnt].sd.junction;
    }


    double thmpTemp = 0, jtnTemp = 0;
    if (orMask == orMask0 && andMask == andMask0) {
        // 5个采样值是一样的，所以随意取一个赋值给snData
        thmpTemp = (double) md[2].sd.thermocouple * 0.25 - 2048.0f;
        jtnTemp = (double) md[2].sd.junction * 0.0625 - 128.0f;
        md[2].temp = ((uint32_t)(MAX31855_CalcTemp(thmpTemp, jtnTemp)*100));
        rSensorData[chNum].tempdeep = ((uint32_t)(MAX31855_CalcTemp(thmpTemp, jtnTemp)*100))<<8;
        return ;
    }
    // 计算温度值
    for (cnt = 0; cnt < 5; cnt++) {
           thmpTemp = (double) md[cnt].sd.thermocouple * 0.25 - 2048.0f;
           jtnTemp = (double) md[cnt].sd.junction * 0.0625 - 128.0f;
           md[cnt].temp = MAX31855_CalcTemp(thmpTemp, jtnTemp);
    }

    // 排序
    MAX31855_qsort(md, 5);

    // 取中间值
    rSensorData[chNum].tempdeep = ((uint32_t)(md[2].temp*100))<<8;

}
/*******************************************************************************
 * @fn:
 * @brief:
 * @para:
 * @return:
 ******************************************************************************/
static int32_t Max31855_get_value(uint8_t chNum, MAX31855_FUNCTION function)
{
    if (chNum < MAX31855_SPI_MAX) {
         if(function & SENSOR_DEEP_TEMP){
             return rSensorData[chNum].tempdeep;
         }
     }else{
          System_printf("Max31855_get_value2\n");
          System_flush();
     }
    return DEEP_TEMP_OVERLOAD;
}
const Max31855_FxnTable  DeepTemp_FxnTable = {
    Max31855_init,
    Max31855_measure,
    Max31855_get_value,
};
#endif
