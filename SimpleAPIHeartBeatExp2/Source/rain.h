#include <ioCC2530.h>

#define LED1 P1_0    //����P10��Ϊdemo1���ƶ�
#define RAIN_PIN P2_0    

extern void ledInit(void);
extern void rainInit( void );
void ledInit(void)
{
  P1DIR |= 0x01;     //P1_0����Ϊ���
  P1INP |= 0X01;     //������
}

void rainInit( void )
{
  P2SEL &= ~0X01;     //����P20Ϊ��ͨIO��  
  P2DIR &= ~0X01;    // ��P20�ڣ�����Ϊ����ģʽ 
  P2INP &=  ~0x01;   //��P20��������,��Ӱ��
}