/**
 ******************************************************************************
 *
 * @file        drv_crc.c
 *
 * @brief       The driver code of CRC.
 *
 * @par         project
 *              8051 serial
 * @version     V0.01
 * @date        2018/11/22
 * @author      Megawin Software Center
 * @copyright   Copyright (c) 2017 MegaWin Technology Co., Ltd.
 *              All rights reserved.
 * 
 ******************************************************************************* 
 * @par Disclaimer
 * The Demo software is provided "AS IS" without any warranty, either
 * expressed or implied, including, but not limited to, the implied warranties
 * of merchantability and fitness for a particular purpose. The author will
 * not be liable for any special, incidental, consequential or indirect
 * damages due to loss of data or any other reason.
 * These statements agree with the world wide and local dictated laws about
 * authorship and violence against these laws.
 *******************************************************************************
 */

#include ".\include\drv_iap.h"
#include "REG_MG82G5Exx.H"

#define    ISP_ENABLE          0x80              // Set ISP Enable
#define    ISP_DISABLE         0x00              // Set ISP Disable



/**
 *******************************************************************************
 * @brief	set Page P control Process
 * @details	This function is used to set PageP Write & Read Process.
 * @param[in]  Addr
 * @param[in]  Data
 * @param[in]   
 * @return	None
 * @note        
 * @par         Example
 * @code
 * @endcode
 *******************************************************************************
 */
void PageP_Write(uint8_t Addr , uint8_t Data)
{
	IFADRH = 0x00;
	IFADRL = Addr;
	IFD = Data;
	IFMT = 0x04;
	ISPCR = ISP_ENABLE;
	SCMD = 0x46;
	SCMD = 0xB9;
	ISPCR = ISP_DISABLE;
}

uint8_t PageP_Read(uint8_t	Addr)
{
	uint8_t	Data;

	IFADRH = 0x00;
	IFADRL = Addr;
	IFMT = 0x05;
	ISPCR = ISP_ENABLE;
	SCMD = 0x46;
	SCMD = 0xB9;
	ISPCR = ISP_DISABLE;
	Data = IFD;
	
	return Data;
}


/**
 *******************************************************************************
 * @brief	set IAP control Process
 * @details	This function is used to set IAP erase and write and read Process.
 * @param[in]   AddrH
 * @param[in]   AddrL
 * @param[in		Write Dta
 * @return	None
 * @note        
 * @par         Example
 * @code
 * @endcode
 *******************************************************************************
 */

void IAP_Erase(uint8_t AddrH, uint8_t AddrL)
{
	IFADRH = AddrH;
	IFADRL = AddrL;
	IFMT = 0x03;								//erase
	ISPCR = ISP_ENABLE;
    SCMD = 0x46;
    SCMD = 0xB9;
    ISPCR = ISP_DISABLE;
}

void IAP_Program(uint8_t AddrH, uint8_t AddrL, uint8_t Dta)
{
	IFADRH = AddrH;
	IFADRL = AddrL;
	IFD = Dta;
	IFMT = 0x02;								//program
	ISPCR = ISP_ENABLE;
    SCMD = 0x46;
    SCMD = 0xB9;
    ISPCR = ISP_DISABLE;
}

uint8_t IAP_Read(uint8_t AddrH, uint8_t AddrL)
{
	uint8_t Dta;

	IFADRH = AddrH;
	IFADRL = AddrL;
	IFMT = 0x01;								//read
	ISPCR = ISP_ENABLE;
    SCMD = 0x46;
    SCMD = 0xB9;
    ISPCR = ISP_DISABLE;

	Dta = IFD;
	return Dta;
}
