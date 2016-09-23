

#include "SPI2.h"
#include "config.h"
#include "socket.h"
#include "w5500.h"
#include <string.h>
#include <stdlib.h>
//#include <stdio.h>




void WIZ_SPI_Init(void)
{
//	SPI_InitTypeDef   SPI_InitStructure;
//
//	  /* SPI Config -------------------------------------------------------------*/
//	  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
//	  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
//	  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
//	  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
//	  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
//	  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
//	  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
//	  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//	  SPI_InitStructure.SPI_CRCPolynomial = 7;
//
//	  SPI_Init(SPI2, &SPI_InitStructure);
//
//	  /* Enable SPI */
//	  SPI_Cmd(SPI2, ENABLE);

}


// Connected to Data Flash
void WIZ_CS(uint_8 val)
{
	if (val == 0) {
		gpio_set(WIZ_SCS, 0);
	}else if (val == 1){
		gpio_set(WIZ_SCS, 1);
	}
}

uint_8 SPI_I2S_ReceiveData(uint_8 spix)
{
	uint_8 temp;
	SPI_MemMapPtr BaseAdd = spi_get_base_address(spix);
	temp = (uint_8)SPI_POPR_REG(BaseAdd);
	return temp;
}

uint_8 SPI2_SendByte(uint_8 byte)
{
	uint_16 rec;
	SPI_MemMapPtr BaseAdd = spi_get_base_address(SPI_2);

	SPI_send1(SPI_2, byte);

	while(!(SPI_SR_REG(BaseAdd) & SPI_SR_RFDF_MASK));

	spi_recieve1(SPI_2,&rec);
	return rec;

}
/*
void SPI1_TXByte(uint_8_t byte)
{
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);       

	  SPI_I2S_SendData(SPI1, byte);	
}
*/
