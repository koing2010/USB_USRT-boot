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
Function: ErasePageRange
Description:
    
Input:        uint32_t TotalBytes: the size of Bin file
Output:       None
Return:       FLASH_Status
Editor:	      Mr.kon
Others:	      2016-8-1
*******************************************************************************/

/*@*****************************end of file**********************************@*/
