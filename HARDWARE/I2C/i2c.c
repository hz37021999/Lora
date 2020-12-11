#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "usmart.h"	 
#include "24cxx.h"	 

		u32 altitude,Temperature;
float altif;


u8 i2cset(void)
{
	

AT24CXX_WriteOneByte(0x26,0x04);
delay_ms(100);
AT24CXX_WriteOneByte(0x26,0xB9);//����ģʽ
delay_ms(100);
	AT24CXX_WriteOneByte(0x13,0x07);//�����жϱ�־
	delay_ms(100);
	AT24CXX_WriteOneByte(0x11,0x01);//����
}
u8 i2c(void)
{
	const u8 TEXT_Buffer[]={"Elite STM32 IIC TEST"};
	#define SIZE sizeof(TEXT_Buffer)	
		u8 temp1,temp2, temp3,temp4,temp6,temp7;
		u16 temp5;
		u16 temp8;
		u8 flag;

		u8 final_alti;
		u8 key;

	u16 i=0;
	u8 datatemp[SIZE];
 
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ	  

   
delay_ms(100);



			//�����β�����reg 0x01 ���ֽڣ����λΪ����λ,reg 0x02 ���ֽ�; reg 0x03 ��04~07λ��x0.0625ΪС�����֣���4λ����
			//*******************************************************
			temp1 = AT24CXX_ReadOneByte(0x01);//msb


			temp2 = AT24CXX_ReadOneByte(0x02);//csb


			temp3 = AT24CXX_ReadOneByte(0x03);//lsb
//********************************************************
		
		
					//���¶ȣ�reg 0x04 �����λΪ����λ,reg 0x05 ��04~07λ��ΪС�����֣���4λ����
		
			temp6 = AT24CXX_ReadOneByte(0x04);//lsb

		temp7 = AT24CXX_ReadOneByte(0x05);//lsb
		
		//******************************************************************

			temp5 = ((u16)temp1<<8)+temp2;//���㺣����������
			
			if(temp1&0x80)//����
				{
					temp5 = ~temp5 +1;
					flag=1;  //fu
				}else flag=0;
				
				
				altif=(temp3>>4)*(float)0.0625+(float)temp5;//���㺣��
				if(flag) altif=-altif;
				
			//altitude = temp5 + (float) (temp3>>4)>>4;

			Temperature=temp8;

//printf("altitude=%6.2f M!\r\n",altif);

	

		
	

	return (temp2);
}