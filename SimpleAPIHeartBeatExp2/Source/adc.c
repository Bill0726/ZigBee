#include "adc.h"
#include "OnBoard.h"
#include "ioCC2530.h"
#include <stdio.h>
#define ADC_CHNN  0x05  //ѡ��ͨ��5 P0_5
#define uchar unsigned char 
#define uint unsigned int
void Get_ADCValue(unsigned char* AvgValue);
/********************************************************
 * @brief   ��ȡADC��ֵ
********************************************************/
void readAdc(uint8 *D,uint8 *G,uint8 channal)
{
          
         
          APCFG |= 1 << channal ; // ģ���ź�����
          ADCIF = 0 ;
         
          ADCCON3 = channal;          
          while ( !ADCIF ) ;  //�ȴ�ת�����
          
          *D = ADCL ;
          *D |= ADCH ;
          
          //return value; 
}
