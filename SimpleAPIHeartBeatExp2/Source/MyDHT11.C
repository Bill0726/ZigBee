#include "hal_mcu.h"
#include "Mydht11.h"
#include "OnBoard.h"


//��ʪ�ȶ���
uint8 uint8FLAG,uint8temp;
uint8 uint8T_data_H,uint8T_data_L,uint8RH_data_H,uint8RH_data_L,uint8checkdata;
uint8 uint8T_data_H_temp,uint8T_data_L_temp,uint8RH_data_H_temp,uint8RH_data_L_temp,uint8checkdata_temp;
uint8 uint8comdata;
uint8 dht11Temp,dht11Humidity;
uint8 dht11TempDec,dht11HumidityDec;

void Delay_us(void);
void Delay_10us(void);
void Delay_ms(uint16 Time);
void COM(void);
void readDHT11(void);

/****************************
        ��ʱ����
*****************************/
void Delay_us(void) //1 us��ʱ
{
    MicroWait(1);   
}

void Delay_10us(void) //10 us��ʱ
{
  MicroWait(10);   
}

void Delay_ms(uint16 Time)//n ms��ʱ
{
  unsigned char i;
  while(Time--)
  {
    for(i=0;i<100;i++)
     Delay_10us();
  }
}


/***********************
   ��ʪ�ȴ���
***********************/
void COM(void)	// ��ʪд��
{     
  uint8 i;         
  for(i=0;i<8;i++)    
  {
    uint8FLAG=2; 
    while((!DHT_PIN)&&uint8FLAG++);
    Delay_10us();
    Delay_10us();
    Delay_10us();
    uint8temp=0;
    if(DHT_PIN)uint8temp=1;
    uint8FLAG=2;
    while((DHT_PIN)&&uint8FLAG++);   
    if(uint8FLAG==1)break;    
    uint8comdata<<=1;
    uint8comdata|=uint8temp; 
  }    
}

void readDHT11(void)   //��ʪ��������
{
  P0DIR |= 0x40; //IO����Ҫ�������� 
  DHT_PIN=0;
  Delay_ms(19);  //>18MS
  DHT_PIN=1; 
  P0DIR &= ~0x40; //��������IO�ڷ���
  Delay_10us();
  Delay_10us();						
  Delay_10us();
  Delay_10us();  
  
  if(!DHT_PIN) //DHT11���ڲ�Ϊ��
  {
    uint8FLAG=2; 
    while((!DHT_PIN)&&uint8FLAG++);
    uint8FLAG=2;
    while((DHT_PIN)&&uint8FLAG++); 
    COM();
    uint8RH_data_H_temp=uint8comdata;
    COM();
    uint8RH_data_L_temp=uint8comdata;
    COM();
    uint8T_data_H_temp=uint8comdata;
    COM();
    uint8T_data_L_temp=uint8comdata;
    COM();
    uint8checkdata_temp=uint8comdata;
    DHT_PIN=1; 
    uint8temp=(uint8T_data_H_temp+uint8T_data_L_temp+uint8RH_data_H_temp+uint8RH_data_L_temp);
    if(uint8temp==uint8checkdata_temp)
    {
      uint8RH_data_H=uint8RH_data_H_temp;
      uint8RH_data_L=uint8RH_data_L_temp;
      uint8T_data_H=uint8T_data_H_temp;
      uint8T_data_L=uint8T_data_L_temp;
      uint8checkdata=uint8checkdata_temp;
    }
    dht11Temp = uint8T_data_H; 
    dht11TempDec = uint8T_data_L;
    dht11Humidity = uint8RH_data_H;     
    dht11HumidityDec = uint8RH_data_L; 
  } 
  else //û�гɹ���ȡ������0
  {
    dht11Temp = 0; 
    dht11TempDec = 0;
    dht11Humidity = 0;
    dht11HumidityDec = 0;
  } 
}