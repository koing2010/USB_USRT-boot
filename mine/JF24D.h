#ifndef  __JF24D_H__
#define  __JF24D_H__
#include "stm32f10x.h"

//	GPIO_SetBits(GPIOC,GPIO_Pin_2);
//	GPIO_ResetBits(GPIOC,GPIO_Pin_2);
#define  WITH_PA               0    /*有功率放大器的无线模块*/
#define  JF24D_VSERSION04      0    /*无线模块版本选择*/

#define FALSE 0
#define SPI_CLK_H   GPIO_SetBits(GPIOA,GPIO_Pin_5)    
#define SPI_CLK_L   GPIO_ResetBits(GPIOA,GPIO_Pin_5)      

#define SPI_MOSI_H  GPIO_SetBits(GPIOA,GPIO_Pin_7)      
#define SPI_MOSI_L  GPIO_ResetBits(GPIOA,GPIO_Pin_7)      

#define SPI_CSN_H   GPIO_SetBits(GPIOA,GPIO_Pin_4)   
#define SPI_CSN_L   GPIO_ResetBits(GPIOA,GPIO_Pin_4)    

#define CE_H        GPIO_SetBits(GPIOA,GPIO_Pin_3)
#define CE_L        GPIO_ResetBits(GPIOA,GPIO_Pin_3)  

#define IRQ         GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)
#define SPI_MISO    GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)

#define RF_LED_H    GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define RF_LED_L    GPIO_ResetBits(GPIOB,GPIO_Pin_12) 

#if(WITH_PA==1)  

#define PA_EN       GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define PA_DISEN    GPIO_ResetBits(GPIOB,GPIO_Pin_1)

#define PA_Tx       GPIO_SetBits(GPIOB,GPIO_Pin_11)
#define PA_Rx       GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#endif
#define 	NOP()     "nop"
/***********SPI??***********************************/
#define MAX_PACKET_LEN  32// ???????,???255

/***********SPI??***********************************/
#define JF24READ_REG        0x00        // ????
#define JF24WRITE_REG       0x20        // ????
#define RD_RX_PLOAD     0x61        // ???FIFO
#define WR_TX_PLOAD     0xA0        // ???FIFO
#define FLUSH_TX        0xE1        // ????FIFO
#define FLUSH_RX        0xE2        // ????FIFO
#define REUSE_TX_PL     0xE3        // ???????
#define W_TX_PAYLOAD_NOACK_CMD	0xb0	 //???FIFO,???
#define W_ACK_PAYLOAD_CMD	0xa8	 //???FIFO
#define ACTIVATE_CMD		0x50	 //ACTIVATE??
#define R_RX_PL_WID_CMD		0x60	 //??????
//#define NOP             0xFF  // ???

/*********????0??????***********************/
#define CONFIG          0x00  // 'Config' register address
#define EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR       0x02  // 'Enabled RX addresses' register address
#define SETUP_AW        0x03  // 'Setup address width' register address
#define SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
#define RF_CH           0x05  // 'RF channel' register address
#define RF_SETUP        0x06  // 'RF setup' register address
#define STATUS          0x07  // 'Status' register address
#define OBSERVE_TX      0x08  // 'Observe TX' register address
#define CD              0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
#define TX_ADDR         0x10  // 'TX address' register address
#define RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS     0x17  // 'FIFO Status Register' register address
#define PAYLOAD_WIDTH   0x1f  // 'payload length of 256 bytes modes register address

/************STATUS?????********************/
#define STATUS_RX_DR 0x40	  
#define STATUS_TX_DS 0x20
#define STATUS_MAX_RT 0x10

#define STATUS_TX_FULL 0x01

/***********FIFO_STATUS?????**********************/
#define FIFO_STATUS_TX_REUSE 0x40
#define FIFO_STATUS_TX_FULL 0x20
#define FIFO_STATUS_TX_EMPTY 0x10

#define FIFO_STATUS_RX_FULL 0x02
#define FIFO_STATUS_RX_EMPTY 0x01

#define ENTER_TX_RX() CE_H	   //???????
#define EXIT_TX_RX()  CE_L	   //???????


void Wireless_Init(u8 add_set);
u8  IsInitWirelessModuleSuccess(void);
void  Wireless_TXData(u8 Addr,u8 FunCode,u8 OrderData);
u8 SPI_RW(u8 byte); 
void SPI_Write_Reg(u8 reg, u8 value);
u8 SPI_Read_Reg(u8 reg);
void SPI_Read_Buf(u8 reg, u8 *pBuf, u8 bytes);
void SPI_Write_Buf(u8 reg,const u8 *pBuf, u8 bytes);
void Enter_PRX(void);
void Enter_PTX(void);
void SwitchCFG(u8 cfg);
void JF24D_Init(void);
void Send_Packet(u8 type,u8* pbuf,u8 len);
u8 Receive_Packet(u8* rx_buf);
void WirteRxAddrToJf24D(u8 address0,u8 address1,u8 address2,u8 address3,u8 address4,u8 address5);
void Wireless_SendData(u8 *OrderData,u8 count);
void Change_WirelessChannel(u8  channel);//修改无线频率通道
void Wireless_RXData(void);
void JF24D_IO_Config(void);
void SimpleDelayMs(u16 N_Ms );
#endif
