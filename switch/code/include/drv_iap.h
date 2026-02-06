/**
 ******************************************************************************
 *
 * @file        drv_iap.H
 *
 * @brief       iap head file.
 *
 * @par         Project
 *              megawin 8051 serial
 * @version     V0.01
 * @date        2018/11/22
 * @author      Megawin Software Center
 * @copyright   Copyright (c) 2017 MegaWin Technology Co., Ltd.
 *              All rights reserved.
 *
 ******************************************************************************
 * @par 		Disclaimer 
 *		The Demo software is provided "AS IS"  without any warranty, either 
 *		expressed or implied, including, but not limited to, the implied warranties 
 *		of merchantability and fitness for a particular purpose.  The author will 
 *		not be liable for any special, incidental, consequential or indirect 
 *		damages due to loss of data or any other reason. 
 *		These statements agree with the world wide and local dictated laws about 
 *		authorship and violence against these laws. 
 ******************************************************************************
 */ 

#ifndef _IAP_H
#define _IAP_H
#include ".\include\Type.h"


void PageP_Write(uint8_t Addr , uint8_t Data);
uint8_t PageP_Read(uint8_t	Addr);
void IAP_Erase(uint8_t AddrH, uint8_t AddrL);
void IAP_Program(uint8_t AddrH, uint8_t AddrL, uint8_t Dta);
uint8_t IAP_Read(uint8_t AddrH, uint8_t AddrL);
#endif
