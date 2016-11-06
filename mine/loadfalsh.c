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
#include "hw_config.h"

#define PAGE_SIZE                       1024
#define FALSH_BASE_ADDRESS             (uint32_t)0x8000000
#define APP_ADDRES_OFFSET_PAGE          20//offset address = APP_ADDRES_OFFSET_PAGE*PAGE_SIZE
/*==============================================================================
@ Global variable
*/




/*==============================================================================
@ All functions  as follow
*/
static Process_Status ProcessWriteCmd(pPROCESS_MSG Msg);




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

      u16 crc = ModBus_CRC16((u8*)&Msg->SerialNumber, Msg->PayoadLenth+3);// sizeof(PROCESS_MSG)- MAX_DATA_SIZE-2bytesCRC16 = 3

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
              status = ProcessWriteCmd(Msg);
              Responded_Msg.Command = WriteRspCmd;
              break;
            case WriteRspCmd :
              break;
            case JumpCmd:
							JumpToApplication(FALSH_BASE_ADDRESS+APP_ADDRES_OFFSET_PAGE*PAGE_SIZE);
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
      Responded_Msg.Attribute = Msg->Attribute;
      Responded_Msg.Data[Offset ++] = status;
      Responded_Msg.PayoadLenth = Offset + sizeof(u16);//+2bytes CRC16
      *(u16*)&Responded_Msg.Data[Offset] = ModBus_CRC16((u8*)&Responded_Msg.SerialNumber,Responded_Msg.PayoadLenth + 3);
      USB_Send_Data((u8*)&Responded_Msg, Responded_Msg.PayoadLenth + sizeof(PROCESS_MSG)- MAX_DATA_SIZE);
    }
  return status;
}
static Process_Status ProcessWriteCmd(pPROCESS_MSG Msg)
{
  static u32 offset = 1;
  Process_Status status = PROCCESS_SUCCESS;
  switch(Msg->Attribute)
    {
    case SystemDescription:
      break;
    case Manufacture:
      break;
    case ModelID:
      break;
    case FileSize:
      if(Msg ->PayoadLenth -2 == sizeof(u32))
        {
          BinFileSize = *(u32*)&Msg ->Data[0];
          if(FLASH_COMPLETE != ErasePageRange(BinFileSize))
            {
              status = PROCCESS_ERRO_ERASE;
            }else
						{
						offset = 0;
						}
        }
      else
        {
          status = PROCCESS_ERRO_ATTR_LENTH;
        }
      break;
    case FileData:
    {
      if(offset == *(u32*)&Msg ->Data[0])
        {
          u32* buf = (u32*)&Msg ->Data[4];
//		if(offset % PAGE_SIZE == 0)
//		{
//		  EraseOnePage(offset / PAGE_SIZE);//relactive page number
//		}
          u8 circle = (Msg->PayoadLenth - sizeof(u32) )/sizeof(u32);
					u8 i = 0;
					FLASH_Unlock();
          for(i = 0; i < circle; i++)
            {
					
             if(FLASH_COMPLETE != FLASH_ProgramWord(offset+ FALSH_BASE_ADDRESS+APP_ADDRES_OFFSET_PAGE*PAGE_SIZE , buf[i]))
						 {
							 FLASH_Lock();
						   status = PROCCESS_ERRO_PROGRAME;
							 break;
						 }
              offset += sizeof(u32);
            }
						FLASH_Lock();
        }
      else
        {
          status = PROCCESS_ERRO_ATTR_LENTH;
        }
    }
    break;
    }
  return status;
}

void JumpToApplication(uint32_t Addr)
{
	pFunction Jump_To_Application;
	__IO uint32_t JumpAddress; 
	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)Addr) & 0x2FFE0000 ) == 0x20000000)
	{ 
	  /* Jump to user application */
	  JumpAddress = *(__IO uint32_t*) (Addr + 4);
	  Jump_To_Application = (pFunction) JumpAddress;
		//__set_PRIMASK(1);//关闭所有中断
		//USB_Interrupts_Disable();
		CAN_ITConfig(CAN1,USB_LP_CAN1_RX0_IRQn, DISABLE);
	  /* Initialize user application's Stack Pointer */
	  __set_MSP(*(__IO uint32_t*) Addr);
	  Jump_To_Application();
	}
	
}

/*@*****************************end of file**********************************@*/
