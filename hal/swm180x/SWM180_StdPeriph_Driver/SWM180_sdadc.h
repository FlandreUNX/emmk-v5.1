#ifndef __SWM180_SDADC_H__
#define __SWM180_SDADC_H__


typedef struct {
	uint8_t clk_src;		//ADCת��ʱ��Դ��SDADC_CLKSRC_HRC_DIV8��ADC_CLKSRC_XTAL
	uint16_t channels;		//ADCת��ͨ��ѡ�У�SDADC_CH0��SDADC_CH1��... ... ��SDADC_CH5������ϣ�������λ�����㣩
	uint8_t out_cali;		//SDADC_OUT_RAW ADC�����У׼��ת�����    SDADC_OUT_CALIED ADC���У׼���ת�����
	uint8_t refp_sel;		//SDADC_REFP_AVDD ʹ��AVDD��ΪREFP    SDADC_REFP_VREFIN ʹ���ⲿVREFIN��������ΪREFP
	uint8_t trig_src;		//ADC������ʽ��ADC_TRIGSRC_SW��ADC_TRIGSRC_TIMR3
	uint8_t Continue;		//����������ģʽ�£�1 ����ת��ģʽ��������һֱ������ת����ֱ���������STARTλ
							//                  0 ����ת��ģʽ��ת����ɺ�STARTλ�Զ����ֹͣת��
	uint8_t EOC_IEn;		//EOC�ж�ʹ�ܣ�1 ʹ���ж�    0 ��ֹ�ж�
	uint8_t OVF_IEn;		//OVF�ж�ʹ�ܣ�1 ʹ���ж�    0 ��ֹ�ж�
	uint8_t HFULL_IEn;		//FIFO�����ж�ʹ�ܣ�1 ʹ���ж�    0 ��ֹ�ж�
	uint8_t FULL_IEn;		//FIFO���ж�ʹ�ܣ�1 ʹ���ж�    0 ��ֹ�ж�
} SDADC_InitStructure;


#define SDADC_CH0		0x001
#define SDADC_CH1		0x002
#define SDADC_CH2		0x004
#define SDADC_CH3		0x008
#define SDADC_CH4		0x010
#define SDADC_CH5		0x020


#define SDADC_CLKSRC_HRC_DIV8	0
#define SDADC_CLKSRC_XTAL_DIV8	1	//���ܳ���6MHz

#define SDADC_OUT_RAW			0
#define SDADC_OUT_CALIED		1

#define SDADC_REFP_AVDD			0
#define SDADC_REFP_REFP			1

#define SDADC_TRIGSRC_SW		0	//������������SDADC->STARTд1����ת��
#define SDADC_TRIGSRC_TIMR3		1	//TIMR3�������ת��

#define SDADC_CFG_A				0
#define SDADC_CFG_B				1
#define SDADC_CFG_C				2

#define SDADC_CFG_GAIN_1		0
#define SDADC_CFG_GAIN_2		1
#define SDADC_CFG_GAIN_4		2
#define SDADC_CFG_GAIN_8		3
#define SDADC_CFG_GAIN_16		4
#define SDADC_CFG_GAIN_32		5
#define SDADC_CFG_GAIN_1DIV2	7


#define SDADC_CALI_COM_GND			0	//У׼ͨ��������ΪGND
#define SDADC_CALI_COM_VDD_1DIV2	1	//У׼ͨ��������ΪVDD/2
#define SDADC_CALI_COM_VDD			2	//У׼ͨ��������ΪVDD


void SDADC_Init(SDADC_TypeDef * SDADCx, SDADC_InitStructure * initStruct);

void SDADC_Config_Set(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t gian, uint32_t sigle_end, uint32_t refm_as_inn);
void SDADC_Config_Cali(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t cali_com, uint32_t cali_fast);
void SDADC_Config_Sel(SDADC_TypeDef * SDADCx, uint32_t CFGx, uint32_t channels);

void SDADC_Open(SDADC_TypeDef * SDADCx);
void SDADC_Close(SDADC_TypeDef * SDADCx);
void SDADC_Start(SDADC_TypeDef * SDADCx);
void SDADC_Stop(SDADC_TypeDef * SDADCx);

uint32_t SDADC_IsEOC(SDADC_TypeDef * SDADCx);
int16_t SDADC_Read(SDADC_TypeDef * SDADCx, uint32_t *chn);

void SDADC_ChnOpen(SDADC_TypeDef * SDADCx, uint32_t chns);
void SDADC_ChnClose(SDADC_TypeDef * SDADCx, uint32_t chns);

void SDADC_IntEOCEn(SDADC_TypeDef * SDADCx);
void SDADC_IntEOCDis(SDADC_TypeDef * SDADCx);
void SDADC_IntEOCClr(SDADC_TypeDef * SDADCx);
uint32_t SDADC_IntEOCStat(SDADC_TypeDef * SDADCx);
void SDADC_IntOVFEn(SDADC_TypeDef * SDADCx);
void SDADC_IntOVFDis(SDADC_TypeDef * SDADCx);
void SDADC_IntOVFClr(SDADC_TypeDef * SDADCx);
uint32_t SDADC_IntOVFStat(SDADC_TypeDef * SDADCx);


#endif //__SWM180_SDADC_H__
