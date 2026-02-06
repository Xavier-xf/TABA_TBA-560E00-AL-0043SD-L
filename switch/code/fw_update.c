#include <string.h>  

//typedef unsigned char   u08;
//typedef unsigned int    u16;
//ljk 전체 추가 
/*
<업데이트 명령 처리 방법>

1) 부트로더 예제에서 port.h 선업부를 하드웨어 특성에 맞게 재 define 한다.
2) IAP_memory_IAPLB 번지는 IAP 시작 영역을 나타 내며, 
   512byte 이후 EEPROM 시작영역을 사용 해야 한다.
   (즉 IAP시작 부터~512byte는 업데이트 용으로 사용) 
3) 마이컴 Type 에 따라 flash 메모리 삭제 시간이 다른데 Ctype의 경우 약 4초 소요.
4) 펌웨어 어플은 아래 필수 변수, 함수, 사용 예제 처럼 사용 한다. 
*/
#define IAP_memory_IAPLB        0x07200


//1. 필요 변수-------------------------------------------------------------------
uint8_t ModelName[10] = {'S','A','T','_','S','W','I','T','C','H'};
unsigned char    rx0_modelbuf[14];
unsigned char    rx1_modelbuf[14];

//2. 필요 함수------------------------------------------------------------------
//==============================================================================
// 역할 :  Read one byte from IAP address
// 매개 : 
// 반환 : 
// 주의 :
//==============================================================================
unsigned char IapReadByte(uint16_t IAP_Addr)
{  
    ISPCR = ISPCR | 0x80 ;   					 //IAP enable 
    IFADRH = IAP_Addr >> 8 ;                     //IAP Flash Address High Byte
    IFADRL = IAP_Addr & 0xFF ;                   //IAP Flash Address Low Byte
    IFMT = 0x01 ;                                //IAP Flash Mode Table (Read Mode)
    IFD = 0x00 ;
    SCMD = 0x46 ;
	SCMD = 0xB9 ;  								//IAP 
 
	IFMT = 0x00 ;
    ISPCR = ISPCR & 0x7F ;						//IAP disable	
    return(IFD);                              	//Return Flash data     
}
//==============================================================================
// 역할 : Write to IAP address one byte
// 매개 : 
// 반환 : 
// 주의 :
//==============================================================================
void IapWriteByte(uint16_t IAP_Addr, unsigned char Value)
{   
   	ISPCR |= 0x80 ;   					//IAP enable 
   	IFMT = 0x02 ;                               //IAP Flash Mode Table (Write Mode)
   	IFADRH = IAP_Addr >> 8;                     //IAP Flash Address High Byte
   	IFADRL = IAP_Addr & 0xFF;                   //IAP Flash Address Low Byte
   	IFD = Value ;                               //Flash data buffer      
   
   	SCMD = 0x46 ;
   	SCMD = 0xB9 ;								//IAP

    IFMT = 0x00 ;                           //select Standby Mode
   	ISPCR = ISPCR & 0x7F ;						//IAP disable
}

//==============================================================================
// 역할 : Erase to IAP address page
// 매개 : 
// 반환 : 
// 주의 :	512 byte
//==============================================================================
void Iap_Page_Erase(unsigned int IAP_Addr)
{  
   	ISPCR |= 0x80 ;   					          //IAP enable 
   	IFMT  = 0x03 ;                                //IAP Flash page erase
   	IFADRH = IAP_Addr >> 8;                      //IAP Flash Address High Byte
   	IFADRL = IAP_Addr & 0xFF;                    //IAP Flash Address Low Byte

   	SCMD = 0x46 ;
   	SCMD = 0xB9 ;								//IAP

    IFMT = 0x00 ;                                //select Standby Mode
   	ISPCR = ISPCR & 0x7F ;						//IAP disable
}
/******************************************************************************
// 역할 : 1byte수신 할 때 마다 Main 수신처리부에서 호출하여 14byte값 맞는지 본다. 
// 매개 : port:포트 번호uart0=0,uart1=1, dt=수신 데이터, *buf포인터
// 반환 :
// 주의 : 
******************************************************************************/
void ModelName_update_parsing(unsigned char port, unsigned char dt,  unsigned char *buf)
{
    uint8_t i;
    for(i= 0; i < 13; i++)
    {
        buf[i] = buf[i+1];
    }
    buf[13] = dt;

    if(memcmp(&buf[4], ModelName, 10) != 0) return;
     
    if((buf[0] != 0x3E) || (buf[1] != 0x41)) return;

		
    Iap_Page_Erase(IAP_memory_IAPLB);
    IapWriteByte(IAP_memory_IAPLB,   'U');
    IapWriteByte(IAP_memory_IAPLB+1, buf[2]);
    IapWriteByte(IAP_memory_IAPLB+2, port);//통신 수신 하드웨어 포트로 고정함.

    for( i = 0; i < 10; i++){
        IapWriteByte(IAP_memory_IAPLB+3+i, ModelName[i]);
    }

    ISPCR = 0x40;    // reset시 isp영역 부터 시작.
    //bWdtSkip = 1;    //watchdog time out rest
    ISPCR |= 0x20; // software reset 

}


/*
3. 사용 예제---------------------------------------------------------------------------
   uart0에서 1byte수신 할 때 마다 Main 수신처리부에서 호출하여 14byte값 맞는지 본다. 
   프로토콜 구조와 상관 없이 14byte순차 값이 동일 하면 업데이트 진행.
        
   ModelName_update_parsing(0, dat, rx_modelbuf);
*/
