#include "smartHome.h"
#include "sapi.h"
#include "hal_led.h"
#include "rain.h"
#include "Mydht11.h"
#include "adc.h"
//����LED�Ƶı�ţ����ǲ�ͬ���ն˹̶���ͬ��LED���
/*#if 1
  #define NUM_LED_1 1
  #define NUM_LED_2 2
#elif 0*/
#define NUM_LED_1 1
#define NUM_LED_2 2
/*#else
  #define NUM_LED_1 5
  #define NUM_LED_2 6
#endif*/

#define LEDNUM  2 //����led����
uint8 ledIdList[LEDNUM]={NUM_LED_1,NUM_LED_2};//������б�

#define NUM_IN_CMD_LEDDEVICE 1 //�����������������
#define NUM_OUT_CMD_LEDDEVICE 2 //����������������
const cId_t ledDeviceInputCommandList[NUM_IN_CMD_LEDDEVICE]=
                                {TOGGLE_LED_CMD_ID}; //������������� 
                                //TOGGLE_LED_CMD_ID������Э�����������Ƶ�״̬����Ϣ
const cId_t ledDeviceOutputCommandList[NUM_OUT_CMD_LEDDEVICE]=
                                {LEDJOINNET_CMD_ID,HEART_BEAT_CMD_ID};//������������
                                //LEDJOINNET_CMD_ID�����͵ƺŵ����� 
                                //HEART_BEAT_CMD_ID�����ʹ��������ݵ�����

//Ϊsimple API�����������                                
const SimpleDescriptionFormat_t zb_SimpleDesc=
{
  ENDPOINT_ID_SMARTHOME,//Endpoint �ն˽ڵ�
  PROFILE_ID_SMARTHOME,//Profile ID ��������ID
  DEVICE_ID_LEDDEVICE,//Device ID �豸���
  DEVIDE_VERSION_ID,//Device Version �豸�汾
  0,//  Reserved Ԥ����
  NUM_IN_CMD_LEDDEVICE,//Number of Input Commands �������������
  (cId_t*)ledDeviceInputCommandList,//  Input Command List ����������б�
  NUM_OUT_CMD_LEDDEVICE,//  Number of Output Commands �������������
  (cId_t*)ledDeviceOutputCommandList  //  Output Command List ����������б�
};
/***********
ִ��ʱ�������͵����ݰ������շ��յ�ʱ������
handle:���ı�ţ�
status:ZSUCCESS��ʾ�ɹ�����
************/
void zb_SendDataConfirm( uint8 handle, uint8 status )//handle:���ݰ���ţ�status:ZSUCCESS��ʾ���ͳɹ���
{
  
}
/***********
ִ��ʱ�������յ������ݰ�������
************/
//������Ϣ���ı�Ƶ�״̬
void zb_ReceiveDataIndication( uint16 source, uint16 command, 
                              uint16 len, uint8 *pData  )//��Դ ���� ���� ���� 
{
  if(command==TOGGLE_LED_CMD_ID)
  {
    uint8 led=pData[0];
    if(pData[0]==1)
    {
      HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);//�����յ�����Ϊ1����������Ϊ0������
    }
    if(pData[0]==0)
    {
      HalLedSet(HAL_LED_1,HAL_LED_MODE_OFF);
    }
  }
}

//�����ڵ�󶨵��ýڵ�ʱ���� source:���Ͱ������豸�������ַ
void zb_AllowBindConfirm( uint16 source )
{
}
//������������Ϣ
void zb_HandleKeys( uint8 shift, uint8 keys )
{
  
}
//�û�����İ󶨳ɹ��󱻵��� commandId :�󶨵�����ţ�status:ZSUCCESS��ʾ�ɹ�
void zb_BindConfirm( uint16 commandId, uint8 status )
{
}

//void zb_SendDataRequest ( uint16 destination, uint16 commandId, uint8 len,
//  uint8 *pData, uint8 handle, uint8 ack, uint8 radius );
//������������������ɹ�ʱ������
//��������󣬽�led�ƺŷ���Э����
void zb_StartConfirm( uint8 status )
{
  if(status==ZSUCCESS)
  {
    //�ɰѽڵ���������led�Ƶ�ID�ŷ��͹�ȥ
    zb_SendDataRequest(0X0,LEDJOINNET_CMD_ID,
                       LEDNUM,ledIdList,0,FALSE,AF_DEFAULT_RADIUS);
                       //���� �󶨵�����ID ���ݳ��� ����
    //zb_SendDataRequest���ͺ���
    osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);//��ʱ��
  }
}
//���������û����Զ����¼�
//��������¼������͵ƺţ�����DHT11���ݣ�������δ��������ݣ�
void zb_HandleOsalEvent( uint16 event )
{
  if(event&TIMER_TIMEOUT_EVT)//������ʱ��
  {
    osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);//��ʱ��
    zb_SendDataRequest(0X0,HEART_BEAT_CMD_ID, 0,NULL,0,FALSE,AF_DEFAULT_RADIUS);
    //���� �󶨵�����ID(�����������Э�����ն˵���)��
    //�����ն˺�
    char buff[]="1:";
    zb_SendDataRequest(0X0,0x7,osal_strlen(buff),buff,0,FALSE,AF_DEFAULT_RADIUS);
    //P0SEL &= 0xbf;//��
    //����DHT11����
    uint8 buf[2];
    readDHT11();//��DHT11���ݵĺ���
    buf[0] = dht11Temp;
    buf[1] = dht11Humidity;
    zb_SendDataRequest(0X0,0x5,2,buf,0,FALSE,AF_DEFAULT_RADIUS);//����DHT11����
    //0x0������ 0x5������DHT11���ݵ�����ID ���ݳ��� ���� 
    //������δ���������
    uint8 adc[10];
    readAdc(&adc[0],&adc[1],5);//adc[0]:��λ adc[1]:��λ
    zb_SendDataRequest(0X0,0x6,osal_strlen(adc),adc,0,FALSE,AF_DEFAULT_RADIUS);
    //0x0������ 0x6��������δ��������� ���ݳ��� ���� 
  }
}
//�û�����ͨ��zb_FindDeviceRequestͨ���ڵ�������ַ��ȡ�ڵ�������ַʱ�ĵ��� 
void zb_FindDeviceConfirm( uint8 searchType, 
                          uint8 *searchKey, uint8 *result )
{
  
}