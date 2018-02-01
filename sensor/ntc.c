//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: ntc.c
// Description: ntc process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_NTC
#include "ntc.h"

/***** Defines *****/
#define         NTC_PARTIAL_TEMP1_PIN       IOID_11
#define         NTC_PARTIAL_TEMP2_PIN       IOID_12
#define         NTC_PARTIAL_TEMP3_PIN       IOID_19



const PIN_Config partialPinTable[] = {
    NTC_PARTIAL_TEMP1_PIN | PIN_INPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*           */
    NTC_PARTIAL_TEMP2_PIN | PIN_INPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*           */
    NTC_PARTIAL_TEMP3_PIN | PIN_INPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*           */
    PIN_TERMINATE
};


//Adc Resistor gpio, value and level table.
static const ResHWAtts_t ResHWAtts[][CTRL_RES_NUM] = {
#ifdef NTC_KEMIT_PT1000
#else
    {
    #ifdef NTC_KEMIT_100K
        // {GPIO_PORT_P7, GPIO_PIN4, 15.80},
        {NTC_PARTIAL_TEMP3_PIN, 35.70},
        {NTC_PARTIAL_TEMP2_PIN, 187.00},
        {NTC_PARTIAL_TEMP1_PIN, 1000.0}
    #elif  defined(NTC_XINXIANG_10K) || defined(NTC_TIANYOU_10K)
        // {GPIO_PORT_P7, GPIO_PIN4, 9.09},
        {NTC_PARTIAL_TEMP3_PIN, 35.70}, //{NTC_PARTIAL_TEMP3_PIN, 15.80},
        {NTC_PARTIAL_TEMP2_PIN, 35.70},
        {NTC_PARTIAL_TEMP1_PIN, 88.70}

	#else
        // {GPIO_PORT_P7, GPIO_PIN4, 0.909},
        {NTC_PARTIAL_TEMP3_PIN, 1.580},
        {NTC_PARTIAL_TEMP2_PIN, 3.570},
        {NTC_PARTIAL_TEMP1_PIN, 8.870}
	#endif
    },
#endif
};



//NTC Resistor table.
static const float NtcResTab[] = {
#ifdef NTC_ELIWELL_10K
    //-35 ~ +35, start -35
    14.400, 13.660, 12.970, 12.320, 11.710,
    11.130, 10.570, 10.040,  9.547,  9.080, 8.630, 8.222, 7.829, 7.458, 7.107,
    6.774,   6.454, 6.152,   5.866,  5.601, 5.339, 5.096, 4.865, 4.648, 4.441,
    4.225,   4.056, 3.876,   3.705,  3.543, 3.389, 3.243, 3.104, 2.972, 2.847,
    2.728,
    2.613,   2.503, 2.399,   2.299,  2.205, 2.115, 2.029, 1.940, 1.870, 1.796,
    1.724,   1.655, 1.590,   1.528,  1.468, 1.412, 1.357, 1.306, 1.256, 1.209,
    1.163,   1.120, 1.078,   1.038,  1.000, 0.963, 0.928, 0.894, 0.862, 0.831,
    0.801,   0.772, 0.745,   0.719,  0.694,
#elif defined(NTC_KEMIT_100K)
	//-50- ~ +80 start -50
	6571, 		6117.97, 	5699.32, 	5312.22, 	4954.08, 	4622.56, 	4315.5, 4030.95, 	3767.11, 	3522.35, 
	3295.16,	3084.17,	2888.12,	2705.86,	2536.34,	2378.59,	2231.7,	2094.88, 	1967.36, 	1848.46,
	1737.54,	1634.02,	1537.35,	1447.04,	1362.63,	1283.71,	1209.88,1140.78,	1076.08,	1015.47,
	958.68,		905.43,		855.48,		808.62,		764.62,		723.31,		684.49,	648.01,		613.7,		581.43,
	551.07,		522.48,		495.56,		470.2,		446.29,		423.76,		402.5,	382.44,		363.51,		345.63,
	328.75,		
	312.34,		296.84,		282.2,		268.37,		255.3,		242.93,		231.24,		220.17,		209.7,	199.79,
	190.4,		181.51,		173.08,		165.09,		157.52,		150.33,		143.52,		137.05,		130.91,	125.08,
	119.54,		114.28,		109.28,		104.52,		100,		95.7,		91.61,		87.71,		84.01,	80.48,	
	77.12,		73.91,		70.86,		67.95,		65.18,		62.53,		60.01,		57.6,		55.3,	53.11,
	51.02,		49.01,		47.1,		45.28,		43.53,		41.86,		40.27,		38.74,		37.28,	35.88,
	34.52,		33.22,		31.98,		30.78,		29.64,		28.55,		27.5,		26.5,		25.54,	24.61,	
	23.73,		22.88,		22.07,		21.29,		20.54,		19.82,		19.13,		18.47,		17.84,	17.22,	
	16.64,		16.07,		15.53,		15.01,		14.51,		14.03,		13.57,		13.12,		12.69,	12.28,
#elif  defined(NTC_XINXIANG_10K)
    //-40- ~ +125 start -40
    205.3,  	193.9,    183.2,    173.2,    163.7,    154.9,    146.6,    138.8,    131.4,    124.5,    
    118,    	111.9,    106.1,    100.7,    95.55,    90.72,    86.16,    81.87,    77.81,    73.98,    
    70.36,  	66.93,    63.69,    60.63,    57.74,    55,    	  52.41,    49.96,    47.64,    45.44,    
    43.35,  	41.37,    39.5,     37.72,    36.03,    34.43,    32.91,    31.46,    30.09,    28.79,    
    27.55,  	
    26.37,      25.25,    24.19,    23.18,    22.21,    21.29,    20.42,    19.58,    18.79,    18.03,    
    17.3,    	16.61,    15.95,    15.33,    14.72,    14.15,    13.6,    	13.08,    12.58,    12.1,    	
    11.64,      11.2,     10.78,    10.38,    10,    	9.632,    9.279,    8.941,    8.617,    8.307,    
    8.01,    	7.724,    7.451,    7.188,    6.936,    6.695,    6.463,    6.24,     6.026,    5.82,    
    5.623,      5.433,    5.251,    5.076,    4.907,    4.745,    4.589,    4.439,    4.295,    4.156,    
    4.023,      3.895,    3.772,    3.653,    3.538,    3.428,    3.321,    3.219,    3.119,    3.024,    
    2.932,      2.843,    2.757,    2.674,    2.594,    2.516,    2.442,    2.37,     2.3,    	2.233,   
    2.167,      2.105,    2.044,    1.985,    1.928,    1.873,    1.82,    	1.769,    1.719,    1.671,    
    1.624,      1.579,    1.535,    1.492,    1.451,    1.411,    1.373,    1.335,    1.299,    1.264,    
    1.23,    	1.197,    1.165,    1.134,    1.104,    1.075,    1.046,    1.019,    0.9924,   0.9666,    
    0.9416,     0.9173,   0.8938,   0.8709,   0.8487,   0.8272,   0.8063,   0.786,    0.7663,  	0.7471,    
    0.7286,     0.7105,   0.693,    0.6759,   0.6594,   0.6433,   0.6276,   0.6125,   0.5977,   0.5833,    
    0.5694,     0.5558,   0.5426,   0.5298,   0.5173,    
#elif defined(NTC_TIANYOU_10K)
    //-40 ~ +82
    336.479,    314.902,  294.849,   	276.194,  258.837, 242.682,  227.633,   213.61,  200.537,   188.348,   
    176.976,    166.354,  156.443,   	147.176,  138.517, 130.422,  122.846,   115.759, 109.121,   102.904,   
    97.083,     91.621,   86.501,   	81.696,   77.189,  72.957,   68.983,   	65.244,  61.736,   	58.433,   	
    55.329,     52.407,   49.658,   	47.065,   44.627,  42.327,   40.16,   	38.113,  36.186,   	34.367,   
    32.65,   
    31.03,   	29.498,   28.051,   	26.683,   25.391,  24.17,   23.015,   	21.918,  20.884,   	19.902,   
    18.97,   	18.091,   17.256,   	16.461,   17.71,   15,   	14.325,   	13.681,  13.073,   	12.491,   
    11.94,   	11.421,   10.924,   	10.448,   10,      9.574,   9.165,   	8.779,   8.406,   	8.055,   
    7.722,   	7.402,    7.096,   		6.807,    6.532,   6.266,   6.017,   	5.777,   5.546,   	5.324,   
    5.115,   	4.916,    4.725,   		4.543,    4.368,   4.201,   4.041,   	3.888,   3.742,   	3.602,   
    3.468,   	3.34,     3.217,   		3.099,    2.986,   2.878,   2.774,   	2.675,   2.579,   	2.488,   
    2.4,   		2.316,    2.235,   		2.157,    2.083,   2.011,   1.942,   	1.876,   1.813,   	1.752,   
    1.693,   	1.637,    1.584,   		1.532,    1.482,   1.434,   1.387,   	1.342,   1.299,   	1.256,   
    1.216,   	1.179,      

#elif defined(NTC_KEMIT_10K)
    //-40 ~ +40, start -40
    30.757, 28.879, 27.127, 25.491, 23.963, 22.536, 21.202, 19.955, 18.788, 17.696,
    16.673, 15.718, 14.823, 13.984, 13.196, 12.458, 11.764, 11.113, 10.502, 9.927,
    9.387,   8.884,  8.411,  7.966,  7.546, 7.150,   6.777, 6.424, 6.092, 5.779,
    5.483,   5.211,  4.954,  4.710,  4.478, 4.259,   4.051, 3.853, 3.666, 3.488,
    3.320,
    3.151,   2.993,  2.843,  2.702,  2.569, 2.443, 2.324, 2.212, 2.105, 2.005,
    1.910,   1.820,  1.735,  1.654,  1.578, 1.505, 1.437, 1.372, 1.310, 1.251,
    1.196,   1.143,  1.093,  1.045,  1.000, 0.957, 0.916, 0.877, 0.840, 0.804,
    0.771,   0.739,  0.708,  0.680,  0.651, 0.625, 0.600, 0.576, 0.553, 0.531,
#elif defined(NTC_KEMIT_PT1000)
    //-50 ~ +70, start +50
    12.708, 12.669, 12.631, 12.593, 12.554, 12.516, 12.478, 12.439, 12.401, 12.363,
    12.324, 12.286, 12.247, 12.209, 12.171, 12.132, 12.094, 12.055, 12.017, 11.978,
    11.940, 11.901, 11.863, 11.824, 11.786, 11.747, 11.708, 11.670, 11.631, 11.593,
    11.554, 11.515, 11.477, 11.438, 11.400, 11.361, 11.322, 11.283, 11.245, 11.206,
    11.167, 11.129, 11.090, 11.051, 11.012, 10.973, 10.935, 10.896, 10.857, 10.818,
    10.779, 10.740, 10.702, 10.663, 10.624, 10.585, 10.546, 10.507, 10.468, 10.429,
    10.390, 10.351, 10.312, 10.273, 10.234, 10.195, 10.156, 10.117, 10.078, 10.039,
    10.000,
    9.961, 9.922, 9.883, 9.844, 9.804, 9.765, 9.726, 9.687, 9.648, 9.609,
    9.569, 9.530, 9.491, 9.452, 9.412, 9.373, 9.334, 9.295, 9.255, 9.216,
    9.177, 9.137, 9.098, 9.059, 9.019, 8.980, 8.940, 8.901, 8.862, 8.822,
    8.783, 8.743, 8.704, 8.664, 8.625, 8.585, 8.546, 8.506, 8.467, 8.427,
    8.387, 8.348, 8.308, 8.269, 8.229, 8.189, 8.150, 8.110, 8.070, 8.031,
#endif
};



/***** Type declarations *****/
typedef struct{
    int16_t temp;
} SensorData_t;



/***** Variable declarations *****/
static SensorData_t rSensorData[NTC_MAX];

// ntc adc handle
ADC_Handle   ntcHandle;

// NTC partial voltage temp handle
PIN_State   partialPinState;
PIN_Handle  partialPinHandle;

//***********************************************************************************
//
// Ntc get value.
//
//***********************************************************************************
static float  Ntc_get_adc_value(uint8_t chNum)
{
    uint8_t i, resCh = 0, ch;
    uint16_t value[CTRL_RES_NUM];
    float res;
	uint16_t diff2048 =0xffff;

    ch = 0;

    //Set control gpio without pull-up/down resistor.
    for (i = 0; i < CTRL_RES_NUM; i++) {
        if (ResHWAtts[ch][i].res == NULL)
            continue;
        PIN_setConfig(partialPinHandle, PIN_BM_INPUT_MODE | PIN_BM_OUTPUT_MODE,
                        ResHWAtts[ch][i].pin | PIN_INPUT_EN | PIN_NOPULL);
    }

    for (i = 0; i < CTRL_RES_NUM; i++) {
        if (ResHWAtts[ch][i].res == NULL)
            continue;
        //Enable Ntc resistor.
        PIN_setConfig(partialPinHandle, PIN_BM_INPUT_MODE | PIN_BM_OUTPUT_MODE,
                        ResHWAtts[ch][i].pin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW);

        //Set Ntc value.
        ADC_convert(ntcHandle, (value + i));
		value[i] -= ADC_COMPENSATION;
        
		#if 0
        if (value[0] < ResHWAtts[ch][i].level)
            resCh = i;
		#else
		if (abs(value[i]-2048) < diff2048)
		{
		    diff2048 = abs(value[i]-2048);
	        resCh = i;
		}
		#endif
        //Disable Ntc resistor.

        PIN_setConfig(partialPinHandle, PIN_BM_INPUT_MODE | PIN_BM_OUTPUT_MODE,
                        ResHWAtts[ch][i].pin | PIN_INPUT_EN | PIN_NOPULL);
    }

    //Set control gpio with pull-up resistor for save power.
    for (i = 0; i < CTRL_RES_NUM; i++) {
        if (ResHWAtts[ch][i].res == NULL)
            continue;
        PIN_setConfig(partialPinHandle, PIN_BM_INPUT_MODE | PIN_BM_OUTPUT_MODE,
                        ResHWAtts[ch][i].pin | PIN_INPUT_EN | PIN_PULLUP);
    }

    if (value[resCh] != 0)
        res = ResHWAtts[ch][resCh].res * ((float)4095 / (float)value[resCh] - 1);
    else
        res = 0;

    return (float)res;
}

//***********************************************************************************
//
// Ntc calculate temperature degree Celsius.
//
//***********************************************************************************
static int16_t Ntc_calc_temperatureC(float temperature)
{
    uint16_t start, end, cur;
    uint16_t cnt;
    int16_t temperatureC;

    if (temperature > NtcResTab[0] || temperature < NtcResTab[sizeofarray(NtcResTab) - 1]) {
        return TEMPERATURE_OVERLOAD;
    }

    start = 0;
    end = sizeofarray(NtcResTab) - 1;
    cnt = 0;
    while (cnt++ < sizeofarray(NtcResTab)) {
        cur = (start + end) / 2;
        if (temperature == NtcResTab[cur])
            break;
        if (temperature < NtcResTab[cur] && temperature > NtcResTab[cur + 1])
            break;
        if (temperature > NtcResTab[cur])
            end = cur;
        else
            start = cur;
    }

#if 0
    float diff;

    if (temperature == NtcResTab[cur] || cur == sizeofarray(NtcResTab) - 1) {
        cnt = 0;
    } else {
        diff = (NtcResTab[cur] - NtcResTab[cur + 1]) / 100.0;
        for (cnt = 0; cnt < 100; cnt++) {
            if ((temperature <= NtcResTab[cur] - diff * cnt) && (temperature > NtcResTab[cur] - diff * (cnt + 1))) {
                break;
            }
        }
    }

    if (START_TEMPERATURE < 0)
        temperatureC = (START_TEMPERATURE + (int16_t)cur) * 100 + (int16_t)cnt;
    else
        temperatureC = (START_TEMPERATURE - (int16_t)cur) * 100 - (int16_t)cnt;
#else

	if (temperature == NtcResTab[cur])
		temperatureC = (START_TEMPERATURE + (int16_t)cur) * 100;
	else {
		if(START_TEMPERATURE < 0) //normal order
			temperatureC = (START_TEMPERATURE + (int16_t)cur) * 100 + (int16_t)(100*(NtcResTab[cur]-temperature))/(NtcResTab[cur] - NtcResTab[cur + 1]);
		else//not  normal order
			temperatureC = (START_TEMPERATURE - (int16_t)cur) * 100 - (int16_t)(100*(NtcResTab[cur]-temperature))/(NtcResTab[cur] - NtcResTab[cur + 1]);	
	}
#endif
    return temperatureC;
}

//***********************************************************************************
//
// NTC init.
//
//***********************************************************************************
static void Ntc_init(uint8_t chNum)
{
    ADC_Params   params;

    ADC_Params_init(&params);

    ntcHandle = ADC_open(ZKS_NTC_ADC, &params);

    partialPinHandle = PIN_open(&partialPinState, partialPinTable);
}

//***********************************************************************************
//
// Ntc measure start.
//
//***********************************************************************************
void Ntc_measure(uint8_t chNum)
{
    float  temp;

    // if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_NTC) {
       
        temp = Ntc_get_adc_value(chNum);

        //convert rawdata to temperature
        rSensorData[chNum].temp = Ntc_calc_temperatureC(temp);
    // }
}

//***********************************************************************************
//
// Ntc get temperature degree Celsius.
//
//***********************************************************************************
static int32_t Ntc_get_value(uint8_t chNum, SENSOR_FUNCTION function)
{
    // if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_NTC) {

        if (function & SENSOR_TEMP) {
            return rSensorData[chNum].temp;
        }
    // }

    return TEMPERATURE_OVERLOAD;
}

const Ntc_FxnTable NTC_FxnTable = {
    Ntc_init,
    Ntc_measure,
    Ntc_get_value,
};

#endif  /* SUPPORT_NTC */
