#include "lora_app.h"
#include "lora_ui.h"
#include "usart3.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "lcd.h"
#include "stdio.h"
#include "text.h"
#include "key.h"
#include "adc.h"
#include "24cxx.h"	 


//#define VOL_H  //��ѹ
#define PH_H  
//#define LEVEL_H  //ˮλ





//�豸������ʼ��(�����豸������lora_cfg.h����)
_LoRa_CFG LoRa_CFG=
{
	.addr = LORA_ADDR,       //�豸��ַ
	.power = LORA_POWER,     //���书��
	.chn = LORA_CHN,         //�ŵ�
	.wlrate = LORA_RATE,     //��������
	.wltime = LORA_WLTIME,   //˯��ʱ��
	.mode = LORA_MODE,       //����ģʽ
	.mode_sta = LORA_STA,    //����״̬
	.bps = LORA_TTLBPS ,     //����������
	.parity = LORA_TTLPAR    //У��λ����
};

//ȫ�ֲ���
EXTI_InitTypeDef EXTI_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;

//�豸����ģʽ(���ڼ�¼�豸״̬)
u8 Lora_mode=0;//0:����ģʽ 1:����ģʽ 2:����ģʽ
//��¼�ж�״̬
static u8 Int_mode=0;//0:�ر� 1:������ 2:�½���

//AUX�ж�����
//mode:���õ�ģʽ 0:�ر� 1:������ 2:�½���
void Aux_Int(u8 mode)
{
    if(!mode)
	{
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;//�ر��ж�
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;	
	}else
	{
		if(mode==1)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //������
	    else if(mode==2)
			 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½���
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	}
	Int_mode = mode;//��¼�ж�ģʽ
	EXTI_Init(&EXTI_InitStructure);
	NVIC_Init(&NVIC_InitStructure);  
    
}

//LORA_AUX�жϷ�����
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4))
	{  
	   if(Int_mode==1)//������(����:��ʼ�������� ����:���ݿ�ʼ���)     
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART3_RX_STA=0;//���ݼ�����0
		  }
		  Int_mode=2;//�����½��ش���
		  LED0=0;//DS0��
	   }
       else if(Int_mode==2)//�½���(����:�����ѷ����� ����:�����������)	
	   {
		  if(Lora_mode==1)//����ģʽ
		  {
			 USART3_RX_STA|=1<<15;//���ݼ���������
		  }else if(Lora_mode==2)//����ģʽ(�������ݷ������)
		  {
			 Lora_mode=1;//�������ģʽ
		  }
		  Int_mode=1;//���������ش���
          LED0=1;//DS0��		   
	   }
       Aux_Int(Int_mode);//���������жϱ���
	   EXTI_ClearITPendingBit(EXTI_Line4); //���LINE4�ϵ��жϱ�־λ  
	}	
}

//LoRaģ���ʼ��
//����ֵ: 0,���ɹ�
//        1,���ʧ��
u8 LoRa_Init(void)
{
	 u8 retry=0;
	 u8 temp=1;
	
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PA�˿�ʱ��
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

     GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);//��ֹJTAG,�Ӷ�PA15��������ͨIOʹ��,����PA15��������ͨIO!!!	
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	    		 //LORA_MD0
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LORA_AUX
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		     //��������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.4
	
	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource4);
	
	 EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	 EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	 EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //�����ش���
  	 EXTI_InitStructure.EXTI_LineCmd = DISABLE;              //�ж��߹ر�(�ȹرպ����ٴ�)
  	 EXTI_Init(&EXTI_InitStructure);//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
	
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//LORA_AUX
  	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�2�� 
  	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//�����ȼ�3
  	 NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE; //�ر��ⲿ�ж�ͨ���������ٴ򿪣�
   	 NVIC_Init(&NVIC_InitStructure); 
	 
	 LORA_MD0=0;
	 LORA_AUX=0;
	
	 while(LORA_AUX)//ȷ��LORAģ���ڿ���״̬��(LORA_AUX=0)
	 {
		 Show_Str(40+30,50+20,200,16,"ģ����æ,���Ե�!!",16,0); 	
		 delay_ms(500);
		 Show_Str(40+30,50+20,200,16,"                    ",16,0);
         delay_ms(100);		 
	 }
	 usart3_init(115200);//��ʼ������3
	 
	 LORA_MD0=1;//����ATģʽ
	 delay_ms(40);
	 retry=3;
	 while(retry--)
	 {
		 if(!lora_send_cmd("AT","OK",70))
		 {
			 temp=0;//���ɹ�
			 break;
		 }	
	 }
	 if(retry==0) temp=1;//���ʧ��
	 return temp;
}

//Loraģ���������



void LoRa_Set(void)
{
	u8 sendbuf[20];
	
	u8 lora_addrh=0,lora_addrl=0;
	#ifdef PH_H
		LoRa_CFG.chn=16;
		LoRa_CFG.addr=1;
	#else
	#ifdef VOL_H
		LoRa_CFG.chn=8;
		LoRa_CFG.addr=2;
	#else
	#ifdef LEVEL_H
		LoRa_CFG.chn=1;
		LoRa_CFG.addr=3;
	#endif
	#endif
	#endif
	
	usart3_set(LORA_TTLBPS_115200,LORA_TTLPAR_8N1);//��������ģʽǰ����ͨ�Ų����ʺ�У��λ(115200 8λ���� 1λֹͣ ������У�飩
	usart3_rx(1);//��������3����
	
	while(LORA_AUX);//�ȴ�ģ�����
	LORA_MD0=1; //��������ģʽ
	delay_ms(40);
	Lora_mode=0;//���"����ģʽ"
	

	
	lora_addrh =  (LoRa_CFG.addr>>8)&0xff;
	lora_addrl = LoRa_CFG.addr&0xff;

	
	
	sprintf((char*)sendbuf,"AT+ADDR=%02x,%02x",lora_addrh,lora_addrl);//�����豸��ַ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLRATE=%d,%d",LoRa_CFG.chn,LORA_RATE_19K2);//�����ŵ��Ϳ�������
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TPOWER=%d",LORA_PW_20Bbm);//���÷��书��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+CWMODE=%d",LORA_MODE_GEN);//���ù���ģʽ
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+TMODE=%d",LORA_STA_Tran);//���÷���״̬
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+WLTIME=%d",LORA_WLTIME_1S);//����˯��ʱ��
	lora_send_cmd(sendbuf,"OK",50);
	sprintf((char*)sendbuf,"AT+UART=%d,%d",LORA_TTLBPS_9600,LORA_TTLPAR_8N1);//���ô��ڲ����ʡ�����У��λ
	lora_send_cmd(sendbuf,"OK",50);

	LORA_MD0=0;//�˳�����,����ͨ��
	delay_ms(40);
	while(LORA_AUX);//�ж��Ƿ����(ģ����������ò���)
	USART3_RX_STA=0;
	Lora_mode=1;//���"����ģʽ"
	usart3_set(LORA_TTLBPS_9600,LORA_TTLPAR_8N1);//����ͨ��,����ͨ�Ŵ�������(�����ʡ�����У��λ)
	Aux_Int(1);//����LORA_AUX�������ж�	
	printf("chn=%d,addr=%d",LoRa_CFG.chn,lora_addrl);
	
}

u8 Dire_Date[]={0x11,0x22,0x33,0x44,0x55};//����������
u8 date[30]={0};//��������
u8 Tran_Data[30]={0};//͸������

#define Dire_DateLen sizeof(Dire_Date)/sizeof(Dire_Date[0])
extern u32 obj_addr;//��¼�û�����Ŀ���ַ
extern u8 obj_chn;//��¼�û�����Ŀ���ŵ�
extern 		float altif;

u8 wlcd_buff[10]={0}; //LCD��ʾ�ַ���������
//Loraģ�鷢������
void LoRa_SendData(void)
{      
	//static u8 num=0;
    u16 addr;
	u8 chn;
	u16 i=0; 
	
	  u16 adcx;
	//float temp;
		
	if(LoRa_CFG.mode_sta == LORA_STA_Tran)//͸������
	{
		//if()
		
#ifdef LEVEL_H

				delay_ms(250);	
		sprintf((char*)Tran_Data,"Altimeter=%6.2f\r\n",altif);
		printf((char*)Tran_Data,"Altimeter=%6.2f",altif);
		u3_printf("%s",Tran_Data);
#endif
		#ifdef PH_H
		adcx=Get_Adc_Average(ADC_Channel_1,10);

		delay_ms(250);	

		sprintf((char*)Tran_Data,"PH_adc=%f\r\n",0.05*(adcx>>4)+0.36);  //PH���� 0.05*data+0.36
		printf("PH_adc=%d",adcx>>4);
	//	sprintf((char*)Tran_Data,"%d",PCin(4));
		u3_printf("%s",Tran_Data); //lora
//		u3_printf("%s\r\n",Tran_Data);
		#endif
				#ifdef VOL_H
		adcx=Get_Adc_Average(ADC_Channel_1,10);

		delay_ms(250);	

		sprintf((char*)Tran_Data,"VOL_adc=%d, Hi=%d\r\n",adcx>>4,PCin(4));
		printf("VOL_adc=%d ",adcx>>4);

	//	sprintf((char*)Tran_Data,"%d",PCin(4));
		u3_printf("%s",Tran_Data);
//		u3_printf("%s\r\n",Tran_Data);
		#endif
		LCD_Fill(0,195,240,220,WHITE); //�����ʾ
		Show_Str_Mid(10,195,Tran_Data,16,240);//��ʾ���͵�����	
		
		
		//num++;
		//if(num==255) num=0;
		
	}else if(LoRa_CFG.mode_sta == LORA_STA_Dire)//������
	{
		
		addr = (u16)obj_addr;//Ŀ���ַ
		chn = obj_chn;//Ŀ���ŵ�
		
		date[i++] =(addr>>8)&0xff;//��λ��ַ
		date[i++] = addr&0xff;//��λ��ַ
		date[i] = chn;//�����ŵ�
		
		for(i=0;i<Dire_DateLen;i++)//����д������BUFF
		{
			date[3+i] = Dire_Date[i];
		}	
		for(i=0;i<(Dire_DateLen+3);i++)
		{
			while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);//ѭ������,ֱ���������   
			USART_SendData(USART3,date[i]); 
		}	
		
        //��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
		sprintf((char*)wlcd_buff,"%x %x %x %x %x %x %x %x",
				date[0],date[1],date[2],date[3],date[4],date[5],date[6],date[7]);
		
		LCD_Fill(0,200,240,230,WHITE);//�����ʾ
		Show_Str_Mid(10,200,wlcd_buff,16,240);//��ʾ���͵�����	
		
	    Dire_Date[4]++;//Dire_Date[4]���ݸ���
		
	}
			
}

u8 rlcd_buff[10]={0}; //LCD��ʾ�ַ���������
//Loraģ���������
void LoRa_ReceData(void)
{
    u16 i=0;
    u16 len=0;
   
	//����������
	if(USART3_RX_STA&0x8000)
	{
		len = USART3_RX_STA&0X7FFF;
		USART3_RX_BUF[len]=0;//��ӽ�����
		USART3_RX_STA=0;

		for(i=0;i<len;i++)
		{
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
			USART_SendData(USART1,USART3_RX_BUF[i]); 
		}
		LCD_Fill(10,260,240,320,WHITE);
		if(LoRa_CFG.mode_sta==LORA_STA_Tran)//͸������
		{	
			Show_Str_Mid(10,270,USART3_RX_BUF,16,240);//��ʾ���յ�������

		}else if(LoRa_CFG.mode_sta==LORA_STA_Dire)//������
		{
			//��ʮ�����Ƶ�����ת��Ϊ�ַ�����ӡ��lcd_buff����
			sprintf((char*)rlcd_buff,"%x %x %x %x %x",
			USART3_RX_BUF[0],USART3_RX_BUF[1],USART3_RX_BUF[2],USART3_RX_BUF[3],USART3_RX_BUF[4]);
				
			Show_Str_Mid(10,270,rlcd_buff,16,240);//��ʾ���յ�������	
		}
		memset((char*)USART3_RX_BUF,0x00,len);//���ڽ��ջ�������0

	}

}

//���ͺͽ��մ���
void LoRa_Process(void)
{
	u8 key=0;
	u8 t=0;
		

	Process_ui();//������ʾ
	LoRa_Set();//LoRa����(�������������ô��ڲ�����Ϊ115200) 
	
	i2cset();//��ʼ��i2c��only when haiba mode
	
	while(1)
	{
		if(PCin(2)==0)
		{
				PCout(3) = 0;
				delay_ms(100);
				PCout(3) = 1;		
		}
		//LoRa_ReceData();
		
		t++;
		if(t==20)
		{
			t=0;
			LED1=~LED1;
					if(!LORA_AUX)//������
					{
				  Lora_mode=2;//���"����״̬"
				  LoRa_SendData();//��������    
					}
					
						i2c();//��������
						
		}			
		delay_ms(10);		
   }
	
}

//�����Ժ���
void Lora_Test(void)
{	
	u8 t=0;
	u8 key=0;
	u8 netpro=0;
	
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	Show_Str_Mid(0,30,"ATK-LORA-01 ���Գ���",16,240); 
	
	while(LoRa_Init())//��ʼ��ATK-LORA-01ģ��
	{
		Show_Str(40+30,50+20,200,16,"δ��⵽ģ��!!!",16,0); 	
		delay_ms(300);
		Show_Str(40+30,50+20,200,16,"                ",16,0);
	}
	Show_Str(40+30,50+20,200,16,"��⵽ģ��!!!",16,0);
	printf("Moude is ok!");
    delay_ms(500); 	
	Menu_ui();//��������ʾ
	
	LoRa_Process();//
	
		
	
	
}
