#define RF_LED_H    GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define RF_LED_L    GPIO_ResetBits(GPIOB,GPIO_Pin_12)



#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "loadfalsh.h"

extern u32 count_out;
extern u8 buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
extern u32 count_in;
extern u8 buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];

/* */
u32  BinFileSize ;

u8 LED_Status;
void LED_IO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
int main(void)
{
//	  JumpToApplication(0x8005000);
    Set_System();
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
	  LED_IO_Config();
    while (1)
    {

        if ((count_out > 0) && (bDeviceState == CONFIGURED))
        {
					 Process_Status status = PROCCESS_SUCCESS;
           status =  ProcessMsg((pPROCESS_MSG)buffer_out, (u16)count_out);
           count_out = 0;
            
					if(LED_Status)
					{
				   	LED_Status = 0;
						 RF_LED_H;
					}
					else
					{
			    	LED_Status = 1;
					  RF_LED_L;
					}
        }
    }

}


/*********************************************************************************
Function:  ModBus_CRC16
Description:
Calculate  ModBus_CRC16

Input:       uchar *puchMsg :   the buffer address
           uchar crc_count :   length of data buffer

Output:       None
Return:       CRC
Others:	      don't care
*********************************************************************************/
u16 ModBus_CRC16(u8 *puchMsg,u8 crc_count)
{
    u8 i ,j;
    u8 XORResult;
    u16 xorCRC = 0xA001; //??CRC????
    u16 CRCdata = 0xFFFF;
    for ( i = 0; i <crc_count; i++)
    {
        CRCdata ^= puchMsg[i];
        for (j = 0; j < 8; j++)
        {
            XORResult = CRCdata & 1;
            CRCdata >>= 1;
            if (XORResult)
                CRCdata ^= xorCRC;
        }
    }

    return  CRCdata;
}


void USB_Send_Data(u8 *pBuf,u8 len)
{

// buffer_in[count_in]
    do {
        buffer_in[count_in] = *pBuf;
        len--;
        count_in++;
        pBuf++;
    } while(len);

    UserToPMABufferCopy(buffer_in, ENDP1_TXADDR, count_in);
    SetEPTxCount(ENDP1, count_in);
    SetEPTxValid(ENDP1);
}


/*********************************************************************************
Function:  USB_To_JF24D_DataProcessing
Description:
Calculate  processing the data  transimitted to deveice.

Input:        data_count  the number of data to be transimitted to.

Output:       None
Return:       CRC
Others:	      don't care
*********************************************************************************/


