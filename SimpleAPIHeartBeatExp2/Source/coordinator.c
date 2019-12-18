#include "smartHome.h"
#include "sapi.h"
#include "hal_led.h"
#include "osal.h"
#include "hal_uart.h"
#include "stdio.h"
#define NUM_IN_CMD_COORINATOR 2  //�����������������
#define NUM_OUT_CMD_COORINATOR 1 //����������������
void *alloceLedDeviceNode(uint8 lednum);
void uart_receive(uint8 port,uint8 event);//����ص�����
void UartSendNum( uint8 x );//���巢��DHT11���ݵ�esp32����
const cId_t coordinatorInputCommandList[NUM_IN_CMD_COORINATOR]=
                                {LEDJOINNET_CMD_ID,HEART_BEAT_CMD_ID};
                                //����Э���������ն���Ϣ�������
                                //LEDJOINNET_CMD_ID�����յƺŵ�����  0x1
                                //HEART_BEAT_CMD_ID�����ն˵���ʱ���յ������� 0x3
const cId_t coordinatorOutputCommandList[NUM_OUT_CMD_COORINATOR]=
                                {TOGGLE_LED_CMD_ID};//����Э�������ն˷�����Ϣ������� 0x3
                                //0x5������DHT11����  
                                //0x6��������δ���������
                                //0x7�������ն˺�
//TIMER_TIMEOUT_EVT �Լ�����Ķ�ʱ�������¼� 0x01
struct led_device_node //����led�Ľṹ��
{
  struct led_device_node *next; //
  uint8 shortAddr[2];  //led�ĵ�ַ
  uint8 lostHeartCount;//
  uint8 ledNum;//led�Ƶ�����
  uint8 ledId[1];//led��id
};

static struct led_device_node ledDeviceHeader={NULL};//����Ƶ�ͷ�ڵ�

void *alloceLedDeviceNode(uint8 lednum)
{
  return osal_mem_alloc(sizeof(struct led_device_node)-1+lednum);//����ռ�
}
//Ϊsimple API����������� 
const SimpleDescriptionFormat_t zb_SimpleDesc=
{
  ENDPOINT_ID_SMARTHOME,//Endpoint �ն˽ڵ� 0x01
  PROFILE_ID_SMARTHOME,//Profile ID ��������ID 0x401
  DEVICE_ID_COORDINATOR,//Device ID �豸��� 0x01
  DEVIDE_VERSION_ID,//Device Version �豸�汾 0x0
  0,//  Reserved Ԥ����
  NUM_IN_CMD_COORINATOR,//Number of Input Commands �������������
  (cId_t*)coordinatorInputCommandList,//Input Command List ���������б�
  NUM_OUT_CMD_COORINATOR,//  Number of Output Commands �������������
  (cId_t*)coordinatorOutputCommandList  //  Output Command List  ����������б�
};
/***********
ִ��ʱ�������͵����ݰ������շ��յ�ʱ������
handle:���ı�ţ�
status:ZSUCCESS��ʾ�ɹ�����
************/
void zb_SendDataConfirm( uint8 handle, uint8 status )//handle:���ݰ���ţ�status:ZSUCCESS��ʾ���ͳɹ���
{
  
}
//����DHT11��esp32���ڵĺ��� 
void UartSendNum( uint8 x )
{
  uint8 buf[10];
  buf[0] = x/100+'0';
  buf[1] = x%100/10+'0';
  buf[2] = x%10+'0';
  HalUARTWrite(1,buf,3);
  //HalUARTWrite(1,wenshidu,osal_strlen(wenshidu));
}
/***********
ִ��ʱ�������յ����ݰ�ʱ������
************/
void zb_ReceiveDataIndication( uint16 source, uint16 command, 
                              uint16 len, uint8 *pData  )//��Դ ���� ���� ����
{
  if(command==LEDJOINNET_CMD_ID)//���ն˼�������󣬽��յƺŵ�����
  {
    char buf[100];
    int i;
    struct led_device_node *p=ledDeviceHeader.next;
    while(p!=NULL)
    {
      if( osal_memcmp(pData,p->ledId,len)==TRUE)//
        break;
      else
      {
        p=p->next;
      }
    }
    if(p==NULL)//�½ڵ����
    {
      struct led_device_node *np=(struct led_device_node *)alloceLedDeviceNode(len);//����ռ�
      osal_memcpy(np->shortAddr,&source,2);
      np->ledNum=len;
      osal_memcpy(np->ledId,pData,len); 
      np->next=ledDeviceHeader.next;//ͷ��
      ledDeviceHeader.next=np;
      p=np;
    } 
    else
    {
      osal_memcpy(p->shortAddr,&source,2);
    }
    //�򴮿ڷ��͵ƺ�
    sprintf(buf,"Led device come on!,shortAddr=%u,ledId:",(uint16)p->shortAddr);//�򴮿ڷ��͵ƺ�
    HalUARTWrite(1,buf,osal_strlen(buf));
    for(i=0;i<p->ledNum;i++)
    {
      sprintf(buf,"%u  ",p->ledId[i]);
      HalUARTWrite(1,buf,osal_strlen(buf));
    }
    //HalUARTWrite(1,"\r\n",2);
  }
  else if(command==HEART_BEAT_CMD_ID)//�����������
  {
    struct led_device_node *p=ledDeviceHeader.next;
    while(p!=NULL)
    {
      if( osal_memcmp(&source,p->shortAddr,2)==TRUE)
        break;
      else
      {
        p=p->next;
      }
    } 
    if(p!=NULL)
    {
      p->lostHeartCount=HEART_BEAT_MAX_COUNT;//HEART_BEAT_MAX_COUNT��3
    }
  }

else if(command==0x7)//�����ն˺�
{
  char buf[100];
  osal_memcpy(buf,pData,len);
  HalUARTWrite(1,buf,osal_strlen(buf));
}
  else if(command==0x5)//����DHT11����
  {
    char buf[100];
    //osal_memcpy(buf,pData,len);
    buf[0]=pData[0];//�¶�
    buf[1]=pData[1];//ʪ��
    HalUARTWrite(1,"T:",osal_strlen("T:"));
    UartSendNum(buf[0]);//�¶�
    HalUARTWrite(1," H:",osal_strlen(" H:"));
    UartSendNum(buf[1]);//ʪ��
  }
   else if(command==0x6)//������δ���������
  {
    char buf[100];
    unsigned int adc;
    adc=pData[0];
    adc|=((unsigned int)pData[1])<<8;
    adc>>2;
    sprintf(buf,"G:%d",adc);
    HalUARTWrite(1,buf,osal_strlen(buf));
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

//������������������ɹ�ʱ������
void zb_StartConfirm( uint8 status )
{

  if(status==ZSUCCESS)
  {
    //��ʼ������
  halUARTCfg_t uartcfg;//halUARTCfg_t�����ڽṹ��
  uartcfg.baudRate=HAL_UART_BR_115200;//���岨����
  uartcfg.flowControl=FALSE;//�ر�����
  uartcfg.callBackFunc=uart_receive;//����ص�����
  //uartcfg.callBackFunc=NULL;
  HalUARTOpen(1,&uartcfg);//�򿪴���
    char buf[]="Coordinator is created successfully!";//����������ɹ����򴮿ڷ��͹����ɹ�����Ϣ
    HalUARTWrite(1,buf,osal_strlen(buf));
    osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);//��ʱ��
  }  
}
//���������û����Զ����¼�
void zb_HandleOsalEvent( uint16 event )
{
  if(event&TIMER_TIMEOUT_EVT)//������ʱ��
  {
    struct led_device_node *p=ledDeviceHeader.next;//����ṹ�����
    struct led_device_node *pre=ledDeviceHeader.next;
    osal_start_timerEx(sapi_TaskID,TIMER_TIMEOUT_EVT,2000);//���ϴ�����
    while(p!=NULL)
    {
      p->lostHeartCount--;
      if(p->lostHeartCount<=0)//���Ƶ�ͷ�ڵ�<=0ʱ����ѭ��
      {
          char buf[100];
         struct led_device_node *pTmp=p;
         pre->next=p->next;  
         p=p->next;
         sprintf(buf,"endpoint:shortAddr=%u is off-line��",(uint16)pTmp->shortAddr);//��ӡ�����ն˵ĵ�ַ
         HalUARTWrite(1,buf,osal_strlen(buf));
         continue;
      }
      pre=p;//����
      p=p->next;
    }    
  }
}
//�û�����ͨ��zb_FindDeviceRequestͨ���ڵ�������ַ��ȡ�ڵ�������ַʱ�ĵ��� 
void zb_FindDeviceConfirm( uint8 searchType, 
                          uint8 *searchKey, uint8 *result )
{
  
}
//����������Ϣ����
void uart_receive(uint8 port,uint8 event)
{
  uint16 dstAddr;
  if(event& (HAL_UART_RX_FULL|HAL_UART_RX_ABOUT_FULL|HAL_UART_RX_TIMEOUT))
  {
    struct led_device_node *p=ledDeviceHeader.next;
    struct led_device_node *pre=ledDeviceHeader.next;
    char buff[20];
    HalUARTRead(1,buff,2);//�����ڷ���������
    while(p!=NULL)
    {
      char id[1];
      sprintf(id,"%d",(char)p->ledId[0]);//����id�Ÿ��Ƶ��¶�������Ϊid�ı�����
      if(id[0]==buff[0])//���Ƶ�ID������յ������ݵĵ�һλ��ȶԣ�
                        // ���յ�����Ϊ��λ������һλ�ǵƵı�ţ��ڶ�λ�ǿ��ƵƵ�״̬
      {
        //HalLedSet(HAL_LED_1,HAL_LED_MODE_ON);
        uint8 buf[1];//�¶���һ��buf����
        if(buff[1]=='1')//���ǽ��յ�����buff�ĵڶ�λΪ1����buf[0]=1������Ϊ1����buf[0]=0
        {
          buf[0]=1;
        }
        else{buf[0]=0;}
        struct led_device_node *pTmp=p;
        pre->next=p->next;//�����ڵ�
        p=p->next;
        osal_memcpy(&dstAddr,pTmp->shortAddr,2);//Ѱ���ն˵ĵ�ַ
        zb_SendDataRequest(dstAddr,TOGGLE_LED_CMD_ID,
                       1,buf,0,FALSE,AF_DEFAULT_RADIUS);
                       //������ַ �󶨵�����ID ���ݳ��� ���� 
                       //��buf���͵��նˣ�buf[0]=1��������buf[0]��=1������
                       //TOGGLE_LED_CMD_ID��Э�������ն˷�����Ϣ�������
        continue;
      }
      pre=p;
      p=p->next;
    }
  }  
}