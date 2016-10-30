/*******************************************************************************

File_name:       loadfalsh.h
Description:    the header file of  Model_c .

*******************************************************************************/
#ifndef     __LOADFLASH_H__
#define     __LOADFLASH_H__

/*==============================================================================
@ Include files
*/
#include "stm32f10x.h"


/*==============================================================================
@ Typedefs
*/
#define MAX_DATA_SIZE   34 // 32bytes + 2btes CRC16
#pragma pack(push,1)//one byte by one byte

/**define the format of usart data**/
typedef struct PROCESS_MSG_
{
  u16 StartCode;
  u16 SerialNumber;
  u8  Command;
  u8  PayoadLenth;
  u8  Data[MAX_DATA_SIZE];
} PROCESS_MSG,*pPROCESS_MSG;
#pragma pack(pop)

/** define the process message status **/
typedef enum
{
  PROCCESS_SUCCESS,
  PROCCESS_ERRO_CRC = 1,
  PROCCESS_SUNSUPPORT_CMD,
  PROCCESS_ERRO_LENTH
} Process_Status;

/** define the command **/
typedef enum
{
  ReadCmd=0x01,
  ReadRspCmd,
  WriteCmd,
  WriteRspCmd,   
} Procees_Cmd;
/*==============================================================================
@ Constants and defines
*/
FLASH_Status EraseOnePage(u16 PageNumber);
Process_Status ProcessMsg(pPROCESS_MSG Msg, u16 TotalLenth);
extern u16 ModBus_CRC16(u8 *puchMsg,u8 crc_count);
extern void USB_Send_Data(u8 *pBuf,u8 len);
extern u32  BinFileSize;






#endif
/*@*****************************end of file**********************************@*/


