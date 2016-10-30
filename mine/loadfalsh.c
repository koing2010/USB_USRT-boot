/*******************************************************************************
Copyright:
File_name:       loadfalsh.c
Version:	       0.0
Revised:        $Date:2016-10-26; $
Description:    the main file of this project.
Notes:          This version targets the stm32
Editor:		      Mr.kon

*******************************************************************************/


/*==============================================================================
@ Include files
*/
#include "loadfalsh.h"

#define PAGE_SIZE                       1024
#define FALSH_BASE_ADDRESS             (uint32_t)0x8000000
#define APP_ADDRES_OFFSET_PAGE          20//offset address = APP_ADDRES_OFFSET_PAGE*PAGE_SIZE
/*==============================================================================
@ Global variable
*/




/*==============================================================================
@ All functions  as follow
*/



/*******************************************************************************
Function: Init_System_Peripher
Description:
     Every peripher used in program could  got initialized in this module .

Input:        None
Output:       None
Return:       None
Editor:	      Mr.kon
Others:	      2016-10-26
*******************************************************************************/
FLASH_Status EraseOnePage(u16 PageNumb)
{
  return FLASH_ErasePage(FALSH_BASE_ADDRESS + 1024 * PAGE_SIZE );
}
/*******************************************************************************
Function: ErasePageRange
Description:

Input:        uint32_t TotalBytes: the size of Bin file
Output:       None
Return:       FLASH_Status
Editor:	      Mr.kon
Others:	      2016-8-1
*******************************************************************************/
FLASH_Status ErasePageRange(uint32_t TotalBytes)
{
  FLASH_Status status = FLASH_COMPLETE;
  u8 i,j;
  /* get the number of pages to erased */
  u8 NumberToErased = TotalBytes / PAGE_SIZE;
  for( i = 0; i < NumberToErased; i ++)
    {
      status = EraseOnePage(NumberToErased + APP_ADDRES_OFFSET_PAGE);
      if(status != FLASH_COMPLETE)
        {
          /* try erase thress times again*/
          for( j = 0; j < 3; j ++)
            {
              status = EraseOnePage(NumberToErased + APP_ADDRES_OFFSET_PAGE);
              if(status == FLASH_COMPLETE)
                {
                  break;
                }
              else
                {
                  continue;
                }
            }
          if(status != FLASH_COMPLETE)
            {
              return status;//erase erro
            }
        }
    }

  return status;
}
/*******************************************************************************
Function: ProcessMsg
Description: ack

Input:        pPROCESS_MSG Msg: usart message;u16 TotalLenth: bytes size of msg
Output:       None
Return:       Process_Status
Editor:	      Mr.kon
Others:	      2016-10-27
*******************************************************************************/
Process_Status ProcessMsg(pPROCESS_MSG Msg, u16 TotalLenth)
{
  Process_Status status = PROCCESS_SUCCESS;
  PROCESS_MSG Responded_Msg = {0};
	u8 Offset = 0;
  
  Responded_Msg.StartCode = 0xaaaa;
  
  /* ensure that the size entered is in the range allowed */
  if(TotalLenth != Msg->PayoadLenth + sizeof(PROCESS_MSG)- MAX_DATA_SIZE)
    {
      status = PROCCESS_ERRO_LENTH;
    }
  else
    {

      u16 crc = ModBus_CRC16((u8*)&Msg->SerialNumber, Msg->PayoadLenth+2);

      //Two bytes at the end of payload data are CRC16 in low endian
      if(crc == *((u16*)&Msg->Data[Msg->PayoadLenth-2]))
        {
          switch(Msg->Command)
            {
            case ReadCmd :
              break;
            case ReadRspCmd :
              break;
            case WriteCmd :
				       BinFileSize = *(u32*)&Msg->Data[0];
						   Responded_Msg.Command = WriteRspCmd;
              break;
            case WriteRspCmd :
              break;

            default:
              status = PROCCESS_SUNSUPPORT_CMD;
            }
          Responded_Msg.SerialNumber = Msg->SerialNumber +1;
        }
      else
        {
          status = PROCCESS_ERRO_CRC;
        }
				
				Responded_Msg.Data[Offset ++] = status;
			  Responded_Msg.PayoadLenth = Offset + sizeof(u16);//+2bytes CRC16
				*(u16*)&Responded_Msg.Data[Offset] = ModBus_CRC16((u8*)&Responded_Msg.SerialNumber,Responded_Msg.PayoadLenth + 2);
			  USB_Send_Data((u8*)&Responded_Msg, Responded_Msg.PayoadLenth + sizeof(PROCESS_MSG)- MAX_DATA_SIZE);
    }
  return status;
}
/*@*****************************end of file**********************************@*/
