#include "../general.h"

#ifdef SUPPORT_DEEPTEMP_PT100
#include "pt100.h"

//pt1000 Resistor table.
static const float Pt100ResTab[] = {
    // -200 ~ +199, start -200
    18.52,  //-200
    18.95,  19.38,  19.82,  20.25,  20.68,  21.11,  21.54,  21.97,  22.40,  22.83,  // -190
    23.25,  23.68,  24.11,  24.54,  24.97,  25.39,  25.82,  26.24,  26.67,  27.10,  // -180
    27.52,  27.95,  28.37,  28.80,  29.22,  29.64,  30.07,  30.49,  30.91,  31.34,  // -170
    31.76,  32.18,  32.60,  33.02,  33.44,  33.86,  34.28,  34.70,  35.12,  35.54,  // -160
    35.96,  36.38,  36.80,  37.22,  37.64,  38.05,  38.47,  38.89,  39.31,  39.72,  // -150
    40.14,  40.56,  40.97,  41.39,  41.80,  42.22,  42.63,  43.05,  43.46,  43.88,  // -140
    44.29,  44.70,  45.12,  45.53,  45.94,  46.36,  46.77,  47.18,  47.59,  48.00,  // -130
    48.42,  48.83,  49.24,  49.65,  50.06,  50.47,  50.88,  51.29,  51.70,  52.11,  // -120
    52.52,  52.93,  53.34,  53.75,  54.15,  54.56,  54.97,  55.38,  55.79,  56.19,  // -110
    56.60,  57.01,  57.41,  57.82,  58.23,  58.63,  59.04,  59.44,  59.85,  60.26,  // -100
    60.66,  61.07,  61.47,  61.88,  62.28,  62.68,  63.09,  63.49,  63.90,  64.30,  // -90
    64.70,  65.11,  65.51,  65.91,  66.31,  66.72,  67.12,  67.52,  67.92,  68.33,  // -80
    68.73,  69.13,  69.53,  69.93,  70.33,  70.73,  71.13,  71.53,  71.93,  72.33,  // -70
    72.73,  73.13,  73.53,  73.93,  74.33,  74.73,  75.13,  75.53,  75.93,  76.33,  // -60
    76.73,  77.12,  77.52,  77.92,  78.32,  78.72,  79.11,  79.51,  79.91,  80.31,  // -50
    80.70,  81.10,  81.50,  81.89,  82.29,  82.69,  83.08,  83.48,  83.87,  84.27,  // -40
    84.67,  85.06,  85.46,  85.85,  86.25,  86.64,  87.04,  87.43,  87.83,  88.22,  // -30
    88.62,  89.01,  89.40,  89.80,  90.19,  90.59,  90.98,  91.37,  91.77,  92.16,  // -20
    92.55,  92.95,  93.34,  93.73,  94.12,  94.52,  94.91,  95.30,  95.69,  96.09,  // -10
    96.48,  96.87,  97.26,  97.65,  98.04,  98.44,  98.83,  99.22,  99.61,  100.00,  //0
    100.39,  100.78,  101.17,  101.56,  101.95,  102.34,  102.73,  103.12,  103.51,
    103.90,  104.29,  104.68,  105.07,  105.46,  105.85,  106.24,  106.63,  107.02,  107.40,  //10
    107.79,  108.18,  108.57,  108.96,  109.35,  109.73,  110.12,  110.51,  110.90,  111.29,  //20
    111.67,  112.06,  112.45,  112.83,  113.22,  113.61,  114.00,  114.38,  114.77,  115.15,  //30
    115.54,  115.93,  116.31,  116.70,  117.08,  117.47,  117.86,  118.24,  118.63,  119.01,  //40
    119.40,  119.78,  120.17,  120.55,  120.94,  121.32,  121.71,  122.09,  122.47,  122.86,  //50
    123.24,  123.63,  124.01,  124.39,  124.78,  125.16,  125.54,  125.93,  126.31,  126.69,  //60
    127.08,  127.46,  127.84,  128.22,  128.61,  128.99,  129.37,  129.75,  130.13,  130.52,  //70
    130.90,  131.28,  131.66,  132.04,  132.42,  132.80,  133.18,  133.57,  133.95,  134.33,  //80
    134.71,  135.09,  135.47,  135.85,  136.23,  136.61,  136.99,  137.37,  137.75,  138.13,  //90
    138.51,  138.88,  139.26,  139.64,  140.02,  140.40,  140.78,  141.16,  141.54,  141.91,  //100
    142.29,  142.67,  143.05,  143.43,  143.80,  144.18,  144.56,  144.94,  145.31,  145.69,  //110
    146.07,  146.44,  146.82,  147.20,  147.57,  147.95,  148.33,  148.70,  149.08,  149.46,  //120
    149.83,  150.21,  150.58,  150.96,  151.33,  151.71,  152.08,  152.46,  152.83,  153.21,  //130
    153.58,  153.96,  154.33,  154.71,  155.08,  155.46,  155.83,  156.20,  156.58,  156.95,  //140
    157.33,  157.70,  158.07,  158.45,  158.82,  159.19,  159.56,  159.94,  160.31,  160.68,  //150
    161.05,  161.43,  161.80,  162.17,  162.54,  162.91,  163.29,  163.66,  164.03,  164.40,  //160
    164.77,  165.14,  165.51,  165.89,  166.26,  166.63,  167.00,  167.37,  167.74,  168.11,  //170
    168.48,  168.85,  169.22,  169.59,  169.96,  170.33,  170.70,  171.07,  171.43,  171.80,  //180
    172.17,  172.54,  172.91,  173.28,  173.65,  174.02,  174.38,  174.75,  175.12,  175.49,  //190
};

static float PT100_CalcTemperature(float temperature)
{
    uint16_t start, end, cur;
    uint16_t cnt;
    float temperatureC = 0;

    if (temperature < Pt100ResTab[0] || temperature > Pt100ResTab[sizeofarray(Pt100ResTab) - 1]) {
        return DEEP_TEMP_OVERLOAD;
    }

    start = 0;
    end = sizeofarray(Pt100ResTab) - 1;
    cnt = 0;
    while (cnt++ < sizeofarray(Pt100ResTab)) {
        cur = (start + end) / 2;
        if (temperature == Pt100ResTab[cur])
            break;
        if (temperature < Pt100ResTab[cur] && temperature > Pt100ResTab[cur - 1])
            break;
        if (temperature < Pt100ResTab[cur])
            end = cur;
        else
            start = cur;
    }

    if (temperature == Pt100ResTab[cur])
        temperatureC = (PT100_START_TEMPERATURE + (float)cur);
    else {
        if(PT100_START_TEMPERATURE < 0) //normal order
            temperatureC = (PT100_START_TEMPERATURE + (float)(cur - 1)) + (float)((temperature - Pt100ResTab[cur - 1]))/(Pt100ResTab[cur] - Pt100ResTab[cur - 1]);
        else//not  normal order
            temperatureC = (PT100_START_TEMPERATURE - (float)(cur - 1)) - (float)((temperature - Pt100ResTab[cur] - 1))/(Pt100ResTab[cur] - Pt100ResTab[cur - 1]);
    }
    return temperatureC;
}

static void PT100_init(uint8_t chNum)
{
    ADS1247_IOInit();
}

static void PT100_measure(uint8_t chNum)
{
    float res = 0, resTotal = 0, resMax, resMin;
    float resArry[4] = {0};
    int32_t temp = 0;
    uint8_t sampleNum = 4;
    uint8_t i;

    if(ADS1247_Handle() == NULL)
        return;
    
    ADS1247_PowerOn();

    ADS1247_ConfigInit();

    for (i = 0; i < sampleNum; ++i) {
        resArry[i] = ADS1247_MeasureOneResistance();
    }

    ADS1247_PowerOff();

    resMax = resArry[0];
    resMin = resArry[0];

    for (i = 0; i < sampleNum; ++i) {
       if (resMax < resArry[i]) {
           resMax = resArry[i];
       }

       if (resMin > resArry[i]) {
           resMin = resArry[i];
       }

       resTotal += resArry[i];
    }

    res = (resTotal - resMax - resMin) / (float)(sampleNum - 2);

    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_DEEPTEMP) {
        temp = (int32_t)(PT100_CalcTemperature(res) * 100.0);
        if (temp <  PT100_MIN_T * 100.0 || temp > PT100_MAX_T * 100.0) {
            rSensorData[chNum].tempdeep = 0xfffffff8;
        } else {
            rSensorData[chNum].tempdeep =  (temp + g_rSysConfigInfo.deepTempAdjust * 10) << 4;
        }
    }
}

static int32_t PT100_get_value(uint8_t chNum, SENSOR_FUNCTION function)
{
    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_DEEPTEMP) {
            return rSensorData[chNum].tempdeep;
    }
    return TEMPERATURE_OVERLOAD;
}

const Sensor_FxnTable  DeepTemp_PT100_FxnTable = {
    SENSOR_DEEP_TEMP,
    PT100_init,
    PT100_measure,
    PT100_get_value
};
#endif




