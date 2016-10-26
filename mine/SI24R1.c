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
//----------------------------   ȫ�ֱ���   ----------------------------------
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
//������spi_init()
//����: SPI���ų�ʼ��
//=====================================================================================
static void spi_init(void)
{
    CE_L;        // ����
    SPI_CSN_H;        // SPI��ֹ
    SPI_CLK_L;        // SPIʱ���õ�
    //IRQ ;        // �жϸ�λ
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
//������spi_rw_byte()
//���ܣ�����SPIЭ�飬дһ�ֽ����ݵ�Si24R1��ͬʱ��Si24R1����һ�ֽ�
//=====================================================================================
u8 spi_rw_byte(u8 byte)
{
    u8 i;
    delay_us(1) ;
    for(i = 0; i < 8; i++)          // ѭ��8��
    {
        if(byte & 0x80)SPI_MOSI_H;   	// byte���λ�����MOSI
        else SPI_MOSI_L;
        byte  <<= 1;             	// ��һλ��λ�����λ
        SPI_CLK_H;                // ����SCK��Si24R1��MOSI����1λ���ݣ�ͬʱ��MISO���1λ����
        delay_us(1) ;
        if(MISO)byte |= 0x01;       		// ��MISO��byte���λ
        SPI_CLK_L;            	// SCK�õ�
        delay_us(1) ;
    }
    return(byte);           		// ���ض�����һ�ֽ�
}

//=====================================================================================
//������spi_rw_reg()
//���ܣ�д����value��reg�Ĵ�����ͬʱ����״̬�Ĵ���ֵ
//=====================================================================================
u8 spi_rw_reg(u8 reg, u8 value)
{
    u8 status;

    SPI_CSN_L;                   	// CSN�õͣ���ʼ��������
    status = spi_rw_byte(reg);   	// ѡ��Ĵ�����ͬʱ����״̬��
    spi_rw_byte(value);             // Ȼ��д���ݵ��üĴ���
    SPI_CSN_H;                   	// CSN���ߣ��������ݴ���
    return(status);            		// ����״̬�Ĵ���
}

//=====================================================================================
//������spi_rd_reg()
//���ܣ���reg�Ĵ�����һ���ֽڵ�����
//=====================================================================================
u8 spi_rd_reg(u8 reg)
{
    u8 value;

    SPI_CSN_L;                	// CSN�õͣ���ʼ��������
    spi_rw_byte(reg);              	// ѡ��Ĵ���
    value 	= spi_rw_byte(0);      	// Ȼ��ӸüĴ���������
    SPI_CSN_H;                	// CSN���ߣ��������ݴ���
    return(value);            		// ���ؼĴ�������
}

//=====================================================================================
//������spi_read_buf()
//���ܣ���reg�Ĵ�������bytes���ֽڣ�ͨ��������ȡ����ͨ������ �� ����/���͵�ַ
//=====================================================================================
u8 spi_read_buf(u8 reg, u8 *pBuf, u8 bytes)
{

    u8 status;
    u8 i;

    SPI_CSN_L;                   	 // CSN�õͣ���ʼ��������
    status  = spi_rw_byte(reg);      // ѡ��Ĵ�����ͬʱ����״̬��
    for(i = 0; i < bytes; i++)
    {
        pBuf[i] = spi_rw_byte(0);    // ����ֽڴ�Si24R1����
    }
    SPI_CSN_H;                     // CSN���ߣ��������ݴ���
    return(status);             	 // ����״̬�Ĵ���
}

//=====================================================================================
//������spi_write_buf()
//���ܣ���pBuf�����е�����д�뵽Si24R1��ͨ������д�뷢��ͨ������ �� ����/���͵�ַ
//=====================================================================================
u8 spi_write_buf(u8 reg, u8 *pBuf, u8 bytes)
{
    u8 status, i;
    SPI_CSN_L;                     // CSN�õͣ���ʼ��������
    status 	= spi_rw_byte(reg);      // ѡ��Ĵ�����ͬʱ����״̬��
    for(i = 0; i < bytes; i++)
    {
        spi_rw_byte(pBuf[i]);        // ����ֽ�д��Si24R1
    }
    SPI_CSN_H;                     // CSN���ߣ��������ݴ���
    return(status);             	 // ����״̬�Ĵ���
}
//=====================================================================================
//������Change_WirelessChannel
//���ܣ����ķ��ͽ��յ�Ƶ��
//=====================================================================================
void Change_WirelessChannel(u8  channel)
{

    channel = channel%126;
    spi_rw_reg((W_REGISTER|5),channel);
    //SPI_Write_Reg(RF_CH,18);
    //frequency = SPI_Read_Reg(RF_CH);

}
//=====================================================================================
//������rx_mode()
//���ܣ������������Si24R1Ϊ����ģʽ���ȴ����շ����豸�����ݰ�
//=====================================================================================
void WirteRxAddr(u8 address0,u8 address1,u8 address2,u8 address3,u8 address4,u8 address5)
{

    RX0_Address[0] = address0;
    spi_write_buf((W_REGISTER+RX_ADDR_P0),RX0_Address,5); 		//д�Ĵ���10��ͨ��0��ַ
    RX0_Address[0]  = address1;
    spi_write_buf((W_REGISTER+RX_ADDR_P1),RX0_Address,5); 		//д�Ĵ���11��ͨ��1��ַ��������ͨ����λ��ַ
    RX0_Address[0]  = address2;
    spi_write_buf((W_REGISTER+RX_ADDR_P2),RX0_Address,1); 		//д�Ĵ���11��ͨ��2��ַ��������ͨ����λ��ַ
    RX0_Address[0]  = address3;
    spi_write_buf((W_REGISTER+RX_ADDR_P3),RX0_Address,1); 		//д�Ĵ���11��ͨ��3��ַ��������ͨ����λ��ַ
    RX0_Address[0]  = address4;
    spi_write_buf((W_REGISTER+RX_ADDR_P4),RX0_Address,1); 		//д�Ĵ���11��ͨ��4��ַ��������ͨ����λ��ַ
    RX0_Address[0]  = address5;
    spi_write_buf((W_REGISTER+RX_ADDR_P5),RX0_Address,1); 		//д�Ĵ���11��ͨ��5��ַ��������ͨ����λ��ַ

}

//=====================================================================================
//������rx_mode()
//���ܣ������������Si24R1Ϊ����ģʽ���ȴ����շ����豸�����ݰ�
//=====================================================================================
void rx_mode(void)
{
    CE_L;
    delay_us(10) ;
//*   spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);	// �����豸����ͨ��0ʹ�úͷ����豸��ͬ�ķ��͵�ַ
    spi_rw_reg(W_REGISTER + EN_AA, 0x01);               				// ʹ�ܽ���ͨ��0�Զ�Ӧ��
    spi_rw_reg(W_REGISTER + EN_RXADDR, 0x01);           				// ʹ�ܽ���ͨ��0
    spi_rw_reg(W_REGISTER + DYNPD , 0x01);                      //ʹ��ͨ����̬���س��Ƚ��չ���
    spi_rw_reg(W_REGISTER + SETUP_AW, 0x03);                     //���յ�ַ����5�ֽ�
//*   spi_rw_reg(W_REGISTER + RF_CH, 0x40);                 				// ѡ����Ƶͨ��0x40
    spi_rw_reg(W_REGISTER + RX_PW_P0, TX_PLOAD_WIDTH);  				// ����ͨ��0ѡ��ͷ���ͨ����ͬ��Ч���ݿ���

//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x20);            				// ���ݴ�����250kbps,-18dbm TX power
//  	spi_rw_reg(W_REGISTER + RF_SETUP, 0x00);            				// ���ݴ�����1Mbps,-18dbm TX power
    spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);            				// ���ݴ�����1Mbps,7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0f);              				// Rx IRQ ʹ�ܣ�CRCʹ�ܣ�16λCRCУ�飬�ϵ磬RX mode

    CE_H;                                            					// ����CE���������豸
}

//=====================================================================================
//������tx_mode()
//����: �����������Si24R1Ϊ����ģʽ����CE=1��������10us����130us���������䣬
//	    ���ݷ��ͽ����󣬷���ģ���Զ�ת�����ģʽ�ȴ�Ӧ���źš�
//=====================================================================================
void tx_mode(u8 *buf,u8 tx_len)
{
//    u8  status;
    CE_L;

    TX_ADDRESS[0] = buf[0];  //tx address
    spi_write_buf(W_REGISTER + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);     		// д�뷢�͵�ַ

    //spi_write_buf(W_REGISTER + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);  	// Ϊ��Ӧ������豸������ͨ��0��ַ�ͷ��͵�ַ��ͬ

    spi_rw_reg(W_REGISTER + FEATURE, 0x05);      					     	// ʹ��NO_ACK_TX  ʹ��

    spi_write_buf(W_TX_PAYLOAD_NOACK , buf, tx_len);                    // д���ݰ���TX FIFO ACK

    //  spi_write_buf(W_TX_PAYLOAD_NOACK, buf,tx_len);                // д���ݰ���TX FIFO NO ACK


    spi_rw_reg(W_REGISTER + SETUP_AW, 0x03);  						 		// 5 byte Address width

    //spi_rw_reg(W_REGISTER + EN_AA, 0x00);       						 	// ʹ�ܽ���ͨ��0�Զ�Ӧ��
    spi_rw_reg(W_REGISTER + EN_RXADDR, 0x3f);   						 	// ʹ�ܽ���ͨ��0

    spi_rw_reg(W_REGISTER + SETUP_RETR, 0x12);  						 	// �Զ��ط���     �Զ��ط���ʱ�ȴ�500us+86us���Զ��ط�15��
//*   spi_rw_reg(W_REGISTER + RF_CH, 0x40);         					     	// ѡ����Ƶͨ��0x40

    spi_rw_reg(W_REGISTER + RF_SETUP, 0x07);   						 	    // ���ݴ�����1Mbps(**��24L01��ͬ**),7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0e);      					     	// TX_DS IRQ enable CRCʹ�ܣ�16λCRCУ�飬�ϵ�, PTX mode


    CE_H;

    delay_us(500);
//    status = spi_rd_reg(CONFIG);	  					// ��״̬�Ĵ���
//    status = spi_rd_reg(STATUS);	  					// ��״̬�Ĵ���
//    if(status)
//      return;

}

//=====================================================================================
//������deal_IRQ()
//����: ����IRQ���֮����¼�  ���� ������� ���ǽ������
//	    ����0,�����жϣ� >0 ���յ�����
//=====================================================================================
u8 deal_IRQ(void)
{
//     status_reg = spi_rd_reg(CONFIG);	  					// ��״̬�Ĵ���
    u8 lenth;
    u8 status_reg = spi_rd_reg(STATUS);	  					// ��״̬�Ĵ���
    if(status_reg & STATUS_RX_DR )				  			  // �ж��Ƿ���յ�����
    {
        lenth = spi_rd_reg(R_RX_PL_WID );  //lenth = spi_rd_reg( 0x11 + status_reg&0x0E); //����Ӧͨ�����ݳ���
        spi_read_buf(R_RX_PAYLOAD, RX_BUF, lenth );  // ��RX FIFO�������ݵ�RX_BUF[]��
        spi_rw_reg(W_REGISTER + STATUS, status_reg);  		// ���RX_DS�жϱ�־      
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
    spi_rw_reg(W_REGISTER + RF_CH, channal);         					     	// ѡ����Ƶͨ��0x40

    spi_rw_reg(W_REGISTER + RF_SETUP, 0x87);   						 	    // ���ݴ�����1Mbps(**��24L01��ͬ**),7dbm TX power

    spi_rw_reg(W_REGISTER + CONFIG, 0x0e);      					     	// TX_DS IRQ enable CRCʹ�ܣ�16λCRCУ�飬�ϵ�, PTX mode
    CE_H;
}

/*@*****************************end of file*************************************@*/