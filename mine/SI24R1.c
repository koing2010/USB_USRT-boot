/**********************************************************************************

File_name:       SI24R1.c
Version:	     1.0
Revised:        $Date:2015-9-24 to   ; $
Description:    the driver of SI24R1.
Notes:          This version targets the stm32
Editor:		      Mr.Kong

**********************************************************************************/

/*================================================================================
@ Include files
*/
#include "SI24R1.h"

/*================================================================================
@ Global variable
*/
//----------------------------   全局变量   ----------------------------------
u8 TX_ADDRESS[TX_ADR_WIDTH] = {0x12,0x34,0x56,0x78,0x01};
u8 RX0_Address[TX_ADR_WIDTH] = {0x12,0x34,0x56,0x78,0x01};
u8 TX_BUF[TX_PLOAD_WIDTH];
u8 RX_BUF[TX_PLOAD_WIDTH];
/*================================================================================
@ All functions  as follow
*/
static void spi_init(void);
/*********************************************************************************
Function:    SI24R1_IO_Config
Description:
MCU           SI24R1               TYPE
PC6             IRQ                I
PC7             CSN
PC8             MOSI
PC9             SCK
PC10            CE
PC11            MISO               I

Input:        None
Output:       None
Return:       local address
Others:	      don't care
*********************************************************************************/
void SI24R1_IO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);
    /* Configure USB pull-up pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

#if(WITH_PA)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif

    spi_init();
}

//=====================================================================================
//函数：spi_init()
//功能: SPI引脚初始化
//=====================================================================================
static void spi_init(void)
{
    CE_L;        // 待机
    SPI_CSN_H;        // SPI禁止
    SPI_CLK_L;        // SPI时钟置低
    //IRQ ;        // 中断复位
}

void delay_us(u16 Nus)
{
    u8 i ;
    while(Nus --)
    {
        i = 18;
        while(i --);
    }

}
//=====================================================================================
//函数：spi_rw_byte()
//功能：根据SPI协议，写一字节数据到Si24R1，同时从Si24R1读出一字节
//=====================================================================================
u8 spi_rw_byte(u8 byte)
{
    u8 i;
    delay_us(1) ;
    for(i = 0; i < 8; i++)          // 循环8次
    {
        if(byte & 0x80)SPI_MOSI_H;   	// byte最高位输出到MOSI
        else SPI_MOSI_L;
        byte  <<= 1;             	// 低一位移位到最高位
        SPI_CLK_H;                // 拉高SCK，Si24R1从MOSI读入1位数据，同时从MISO输出1位数据
        delay_us(1) ;
        if(MISO)byte |= 0x01;       		// 读MISO到byte最低位
        SPI_CLK_L;            	// SCK置低
        delay_us(1) ;
    }
    return(byte);           		// 返回读出的一字节
}

//=====================================================================================
//函数：spi_rw_reg()
//功能：写数据value到reg寄存器，同时返回状态寄存器值
//=====================================================================================
u8 spi_rw_reg(u8 reg, u8 value)
{
    u8 status;

    SPI_CSN_L;                   	// CSN置低，开始传输数据
    status = spi_rw_byte(reg);   	// 选择寄存器，同时返回状态字
    spi_rw_byte(value);             // 然后写数据到该寄存器
    SPI_CSN_H;                   	// CSN拉高，结束数据传输
    return(status);            		// 返回状态寄存器
}

//=====================================================================================
//函数：spi_rd_reg()
//功能：从reg寄存器读一个字节的数据
//=====================================================================================
u8 spi_rd_reg(u8 reg)
{
    u8 value;

    SPI_CSN_L;                	// CSN置低，开始传输数据
    spi_rw_byte(reg);              	// 选择寄存器
    value 	= spi_rw_byte(0);      	// 然后从该寄存器读数据
    SPI_CSN_H;                	// CSN拉高，结束数据传输
    return(value);            		// 返回寄存器数据
}

//=====================================================================================
//函数：spi_read_buf()
//功能：从reg寄存器读出bytes个字节，通常用来读取接收通道数据 或 接收/发送地址
//=====================================================================================
u8 spi_read_buf(u8 reg, u8 *pBuf, u8 bytes)
{

    u8 status;
    u8 i;

    SPI_CSN_L;                   	 // CSN置低，开始传输数据
    status  = spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
    for(i = 0; i < bytes; i++)
    {
        pBuf[i] = spi_rw_byte(0);    // 逐个字节从Si24R1读出
    }
    SPI_CSN_H;                     // CSN拉高，结束数据传输
    return(status);             	 // 返回状态寄存器
}

//=====================================================================================
//函数：spi_write_buf()
//功能：把pBuf缓存中的数据写入到Si24R1，通常用来写入发射通道数据 或 接收/发送地址
//=====================================================================================
u8 spi_write_buf(u8 reg, u8 *pBuf, u8 bytes)
{
    u8 status, i;
    SPI_CSN_L;                     // CSN置低，开始传输数据
    status 	= spi_rw_byte(reg);      // 选择寄存器，同时返回状态字
    for(i = 0; i < bytes; i++)
    {
        spi_rw_byte(pBuf[i]);        // 逐个字节写入Si24R1
    }
    SPI_CSN_H;                     // CSN拉高，结束数据传输
    return(status);             	 // 返回状态寄存器
}
//=====================================================================================
//函数：Change_WirelessChannel
//功能：更改发送接收的频道
//=====================================================================================
void Change_WirelessChannel(u8  channel)
{

    channel = channel%126;
    spi_rw_reg((W_REGISTER|5),channel);
    //SPI_Write_Reg(RF_CH,18);
    //frequency = SPI_Read_Reg(RF_CH);

}
//=====================================================================================
//函数：rx_mode()
//功能：这个函数设置Si24R1为接收模式，等待接收发送设备的数据包
//=====================================================================================
void WirteRxAddr(u8 address0,u8 address1,u8 address2,u8 address3,u8 address4,u8 address5)
{

    RX0_Address[0] = address0;
    spi_write_buf((W_REGISTER+RX_ADDR_P0),RX0_Address,5); 		//写寄存器10，通道0地址
    RX0_Address[0]  = address1;
    spi_write_buf((W_REGISTER+RX_ADDR_P1),RX0_Address,5); 		//写寄存器11，通道1地址，及其余通道高位地址
    RX0_Address[0]  = address2;
    spi_write_buf((W_REGISTER+RX_ADDR_P2),RX0_Address,1); 		//写寄存器11，通道2地址，及其余通道高位地址
    RX0_Address[0]  = address3;
    spi_write_buf((W_REGISTER+RX_ADDR_P3),RX0_Address,1); 		//写寄存器11，通道3地址，及其余通道高位地址
    RX0_Address[0]  = address4;
    spi_write_buf((W_REGISTER+RX_ADDR_P4),RX0_Address,1); 		//写寄存器11，通道4地址，及其余通道高位地址
    RX0_Address[0]  = address5;
    spi_write_buf((W_REGISTER+RX_ADDR_P5),RX0_Address,1); 		//写寄存器11，通道5地址，及其余通道高位地址

}

//=====================================================================================
//函数：rx_mode()
//功能：这个函数设置Si24R1为接收模式，等待接收发送设备的数据包
//=====================================================================================
void rx_mode(void)
{
    CE_L;
    delay_us(10) ;
//*   spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);	// 接收设备接收通道0使用和发送设备相同的发送地址
    spi_rw_reg(W_REGISTER + EN_AA, 0x01);               				// 使能接收通道0自动应答
    spi_rw_reg(W_REGISTER + EN_RXADDR, 0x01);           				// 使能接收通道0
    spi_rw_reg(W_REGISTER + DYNPD , 0x01);                      //使能通道动态负载长度接收功能
    spi_rw_reg(W_REGISTER + SETUP_AW, 0x03);                     //接收地址长度5字节
//*   spi_rw_reg(W_REGISTER + RF_CH, 0x40);                 				// 选择射频通道0x40
    spi_rw_reg(W_REGISTER + RX_PW_P0, TX_PLOAD_WIDTH);  				// 接收通道0选择和发送通道相同有效数据宽度

//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x20);            				// 数据传输率250kbps,-18dbm TX power
//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x00);            				// 数据传输率1Mbps,-18dbm TX power
    spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);            				// 数据传输率1Mbps,7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0f);              				// Rx IRQ 使能，CRC使能，16位CRC校验，上电，RX mode

    CE_H;                                            					// 拉高CE启动接收设备
}

//=====================================================================================
//函数：tx_mode()
//功能: 这个函数设置Si24R1为发送模式，（CE=1持续至少10us），130us后启动发射，
//	    数据发送结束后，发送模块自动转入接收模式等待应答信号。
//=====================================================================================
void tx_mode(u8 *buf,u8 tx_len)
{
//    u8  status;
    CE_L;

    TX_ADDRESS[0] = buf[0];  //tx address
    spi_write_buf(W_REGISTER + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     		// 写入发送地址

    //spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  	// 为了应答接收设备，接收通道0地址和发送地址相同

    spi_rw_reg(W_REGISTER + FEATURE, 0x05);      					     	// 使能NO_ACK_TX  使能

    spi_write_buf(W_TX_PAYLOAD_NOACK , buf, tx_len);                    // 写数据包到TX FIFO ACK

    //  spi_write_buf(W_TX_PAYLOAD_NOACK, buf,tx_len);                // 写数据包到TX FIFO NO ACK


    spi_rw_reg(W_REGISTER + SETUP_AW, 0x03);  						 		// 5 byte Address width

    //spi_rw_reg(W_REGISTER + EN_AA, 0x00);       						 	// 使能接收通道0自动应答
    spi_rw_reg(W_REGISTER + EN_RXADDR, 0x3f);   						 	// 使能接收通道0

    spi_rw_reg(W_REGISTER + SETUP_RETR, 0x12);  						 	// 自动重发。     自动重发延时等待500us+86us，自动重发15次
//*   spi_rw_reg(W_REGISTER + RF_CH, 0x40);         					     	// 选择射频通道0x40

    spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);   						 	    // 数据传输率1Mbps(**与24L01不同**),7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0e);      					     	// TX_DS IRQ enable CRC使能，16位CRC校验，上电, PTX mode


    CE_H;

    delay_us(500);
//    status = spi_rd_reg(CONFIG);	  					// 读状态寄存器
//    status = spi_rd_reg(STATUS);	  					// 读状态寄存器
//    if(status)
//      return;

}

//=====================================================================================
//函数：deal_IRQ()
//功能: 处理IRQ变低之后的事件  或是 发送完毕 或是接收完毕
//	    返回0,其它中断， >0 接收到数据
//=====================================================================================
u8 deal_IRQ(void)
{
//     status_reg = spi_rd_reg(CONFIG);	  					// 读状态寄存器
    u8 lenth;
    u8 status_reg = spi_rd_reg(STATUS);	  					// 读状态寄存器
    if(status_reg & STATUS_RX_DR )				  			  // 判断是否接收到数据
    {
        lenth = spi_rd_reg(R_RX_PL_WID );  //lenth = spi_rd_reg( 0x11 + status_reg&0x0E); //度相应通道数据长度
        spi_read_buf(R_RX_PAYLOAD, RX_BUF, lenth );  // 从RX FIFO读出数据到RX_BUF[]中
        spi_rw_reg(W_REGISTER + STATUS, status_reg);  		// 清除RX_DS中断标志      
        return lenth;
    }
    else if(status_reg & STATUS_TX_DS)
    {
        spi_rw_reg(W_REGISTER + STATUS, status_reg);

        return  0;
    }
    else
    {
        spi_rw_reg(W_REGISTER + STATUS, status_reg);
    }
    return 0;
}
void test_mode(u8 channal)
{
    CE_L;
    spi_rw_reg(W_REGISTER + RF_CH, channal);         					     	// 选择射频通道0x40

    spi_rw_reg(W_REGISTER + RF_SETUP, 0x87);   						 	    // 数据传输率1Mbps(**与24L01不同**),7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0e);      					     	// TX_DS IRQ enable CRC使能，16位CRC校验，上电, PTX mode
    CE_H;
}

/*@*****************************end of file*************************************@*/
