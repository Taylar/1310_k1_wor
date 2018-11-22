#include "../../general.h"

#ifdef SUPPORT_DEEPTEMP_PT100

static const PIN_Config ads1247_pin_config[] = {
    ADS1247_POWER_CTR_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    ADS1247_DRDY_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    //ADS1247_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    ADS1247_SPI_CLK_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    ADS1247_SPI_SIMO_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    ADS1247_SPI_SOMI_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

static PIN_State   ads1247PinState;
static PIN_Handle  ads1247PinHandle = NULL;

uint8_t ads1247PGAGain = 1;

static uint8_t ADS1247_spi_transmit(uint8_t data)
{
    uint8_t r = 0;

    //SPI�ٶ����2MHz,��������͵Ȳ���
    uint8_t i;
    for(i = 0; i < 8; i++) {

      ADS_SPI_CLK_HIGH();
      r <<= 1;

      if (data & 0x80) {
          ADS_SPI_SIMO_HIGH();
      } else {
          ADS_SPI_SIMO_LOW();
      }

      ADS_SPI_CLK_DELAY();
      if (READ_ADS_SPI_SOMI_PIN_LEVEL()) {
          r++;
      }
      ADS_SPI_CLK_LOW();

      ADS_SPI_CLK_DELAY();

      data <<= 1;
    }
    return r;
}

static int32_t ADS1247_ReadData(void)
{
    int32_t r = 0;

    //SPI�ٶ����2MHz,��������͵Ȳ���
    uint8_t i;
    uint32_t data = 0xffffff;

    ENABLE_ADS1247_CS();
    for(i = 0; i < 24; i++) {
        ADS_SPI_CLK_HIGH();
        r <<= 1;

        if (data & 0x80000) {
            ADS_SPI_SIMO_HIGH();
        } else {
            ADS_SPI_SIMO_LOW();
        }

        ADS_SPI_CLK_DELAY();
        ADS_SPI_CLK_LOW();

        if (READ_ADS_SPI_SOMI_PIN_LEVEL()) {
            r++;
        }

        ADS_SPI_CLK_DELAY();

        data <<= 1;
    }
    DISENABLE_ADS1247_CS();

    return r;
}

void ADS1247_IOInit(void)
{
    ads1247PinHandle = PIN_open(&ads1247PinState, ads1247_pin_config);

    ADS_SPI_CLK_LOW();

    DISENABLE_ADS1247_CS();
    ADS1247_PowerOff();
}

// -------------------------------------------
//
//--------------------------------------------
void ADS1247_ConfigInit(void)
{
    // 00 000 001,Bit7-6:Burnout��ʹ�ã�Bit5-3:������ΪAI0��Bit2-0:������ΪAIN1
    ADS1247_SetInputChannel(P_AIN2, N_AIN3);

    // ����IEXC1ΪIDAC1������������Ϊ500uA
    ADS1247_SetOutputCurrentValue(ADS1247_IDAC1_OUTPUT_CHANNEL_AN0, ADS1247_IDAC2_OUTPUT_CHANNEL_AN1,\
                                  ADS1247_DOUT_MODULE_ONLY_DOUT, ADS1247_IEXC_OUTPUT_1000UA);

    // ����1�� ת������20
    ADS1247_SetPGAGainAndDataRate(ADC_GAIN_1, ADC_SPS_20);
    ads1247PGAGain = 1;

    //��ʼ��MUX1�Ĵ���
    ADS1247_SetReference(CLK_INTER, REF_INTER_ALWAYS_ON, SELT_REF0);

//    System_printf("SYS0=0x02:0x%x, MUX1=0x20:0x%x\r\n",ADS1247_ReadReg(ADS1247REG_SYS0), ADS1247_ReadReg(ADS1247REG_MUX1));
//    System_printf("IDAC0=0x96:0x%x, IDAC1=0x01:0x%x\r\n",ADS1247_ReadReg(ADS1247REG_IDAC0), ADS1247_ReadReg(ADS1247REG_IDAC1));
//    System_flush();
}


void ADS1247_PowerOn(void)
{
    READ_ADS1247_SOMI_PIN_INPUT();
    READ_ADS1247_DRDY_PIN_INPUT();
    ADS1247_POWER_ON();
    Task_sleep(30 * CLOCK_UNIT_MS);
}

void ADS1247_PowerOff(void)
{
    ADS1247_POWER_OFF();
    ADS_SPI_SIMO_LOW();
    ENABLE_ADS1247_CS();

    ADS_SPI_SIMO_LOW();
    READ_ADS1247_SOMI_PIN_OUTPUT();
    READ_ADS1247_DRDY_PIN_OUTPUT();
}

// -------------------------------------------
//
//--------------------------------------------
void ADS1247_WriteCmd(uint8_t cmd)
{
    ENABLE_ADS1247_CS();
    ADS1247_spi_transmit(cmd);
    DISENABLE_ADS1247_CS();
}

// -------------------------------------------
//
//--------------------------------------------
void ADS1247_WriteReg(uint8_t regAddr, uint8_t dataByte)
{
    ENABLE_ADS1247_CS();
    // ��Ĵ���д�����ݵ�ַ
    ADS1247_spi_transmit(ADS1247CMD_WREG + (regAddr & 0x0F));
    //д�����ݵĸ���n-1
    ADS1247_spi_transmit(0);
    //��regaddr��ַָ��ļĴ���д������databyte
    ADS1247_spi_transmit(dataByte);
    DISENABLE_ADS1247_CS();
}

// -------------------------------------------
//
//--------------------------------------------
uint8_t ADS1247_ReadReg(uint8_t regAddr)
{
    uint8_t reg = 0;

    ENABLE_ADS1247_CS();
    //��ADS1248�е�ַΪregaddr�ļĴ�������һ���ֽ�
    //д���ַ
    ADS1247_spi_transmit(ADS1247CMD_RREG + (regAddr & 0x0F));
    //д���ȡ���ݵĸ���n-1
    ADS1247_spi_transmit(0);
    //����regaddr��ַָ��ļĴ���������
    reg = ADS1247_spi_transmit(ADS1247CMD_NOP);
    DISENABLE_ADS1247_CS();

    return reg;
}

// ---------------------------------------------------------
// ����ת������
// ---------------------------------------------------------
void ADS1247_SetPGAGainAndDataRate(uint8_t covGain, uint8_t covRate)
{
    uint8_t cmd = 0;
    cmd = covGain | covRate;
    ADS1247_WriteReg(ADS1247REG_SYS0, cmd);      //���ò������������
}

// -------------------------------------------------------------------
// ����ʱ��Դ����׼�ο�
// -------------------------------------------------------------------
void ADS1247_SetReference(uint8_t clkSelected, uint8_t interVrefOnOff, uint8_t refSelected)
{
    ADS1247_WriteReg(ADS1247REG_MUX1, clkSelected | interVrefOnOff | refSelected);
}

// --------------------------------------------------------------------------
// ����ת��ͨ����������͸�����
// --------------------------------------------------------------------------
void ADS1247_SetInputChannel(uint8_t positiveChannel, uint8_t negativeChannel)
{
    ADS1247_WriteReg(ADS1247REG_MUX0, positiveChannel | negativeChannel);
}

// ---------------------------------------------------------------------------
// ����IDAC�����ͨ���͵�����С���ڲ��ο������
// ---------------------------------------------------------------------------
void ADS1247_SetOutputCurrentValue(uint8_t IDAC1Chn, uint8_t IDAC2Chn, uint8_t doutModule, uint8_t IValue)
{
    ADS1247_WriteReg(ADS1247REG_IDAC1, IDAC1Chn | IDAC2Chn);
    // DOUT/DRDY pin functions both as Data Out and Data Ready
    ADS1247_WriteReg(ADS1247REG_IDAC0, IValue | doutModule);
    ADS1247_SetReference(CLK_INTER, REF_INTER_ALWAYS_ON, SELT_REF0);
}

int32_t ADS1247_StartAConversion(void)
{
    int32_t r = 0, i = 0;

    // �ȴ�DRDY��ͣ�ָʾADת������
    while(READ_ADS1247_DRDY_PIN_LEVEL()) {
        Task_sleep(20 * CLOCK_UNIT_MS);
        i++;
        if(i > 10) {
            return 0;
        }
    }

    ADS1247_WriteCmd(ADS1247CMD_RDATA);
    // ��ת�����
    r = ADS1247_ReadData();

    // �����ʹ�õ�����DRDY�ߣ��ڽ���һ��ת������ȡ����ֵ�󣬱���ִ��NOP��WREG�Ȳ�Ӱ������Ĵ�����ָ��ǿ��DOUT/DRDY��Ϊ��
    ADS1247_WriteCmd(ADS1247CMD_NOP);

    if(r & 0x800000) { // ���費����Ϊ����
       return 0;
    }

    return r / ((int32_t) ads1247PGAGain);
}


//-----------------------------------------------------------------//
//  �������� / (2 *��׼����) = ����ֵ / 2^23
//  �������� = (����ֵ / 2^23) * (2 * ��׼����)
//  ��    �ܣ�ADS1248 ������
//  ��    ע: ����һ��ADS1248����,���Ϊ����
//-----------------------------------------------------------------//
float ADS1247_MeasureOneResistance(void)
{
    int32_t S1 = 0, S2 = 0;
    float value = 0.0f;

    ADS1247_SetOutputCurrentValue(ADS1247_IDAC1_OUTPUT_CHANNEL_AN0, ADS1247_IDAC2_OUTPUT_CHANNEL_AN1, \
                                      ADS1247_DOUT_MODULE_ONLY_DOUT, ADS1247_IEXC_OUTPUT_1000UA);
    Task_sleep(1 * CLOCK_UNIT_MS);
    S1 =  ADS1247_StartAConversion();

    // �л�����ͨ��
    ADS1247_SetOutputCurrentValue(ADS1247_IDAC1_OUTPUT_CHANNEL_AN1, ADS1247_IDAC2_OUTPUT_CHANNEL_AN0, \
                                  ADS1247_DOUT_MODULE_ONLY_DOUT, ADS1247_IEXC_OUTPUT_1000UA);
    Task_sleep(1 * CLOCK_UNIT_MS);
    S2 =  ADS1247_StartAConversion();

    value = ((float)S1 + (float)S2) / 2.0 * ADS1247_UNIT_REF0;

    return value;
}
#endif

