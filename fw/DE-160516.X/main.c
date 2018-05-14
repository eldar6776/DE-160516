/******************************************************************************
 *
 *               Apsolutni enkoder ugla za ILLIG RDM 58/3
 *
 *                          Glavni program
 *
 *******************************************************************************
 * Ime fajla:       main.c
 *
 * Procesor:        PIC18F23K20
 *
 * Kompajler:       MCC18 v3.47
 *
 * IDE:             MPLAB X IDE v3.26 (java 1.8.0_65)
 *
 * Firma:           PROVEN d.o.o. Sarajevo
 *
 * Autor:           <mailto> eldar6776@hotmail.com
 *
 * Prava:           Sva prava na programski kod zadrzava firma
 *                  PROVEN d.o.o. Sarajevo.
 *                  <mailto> proven@bih.net.ba
 *                  Adema Buce 102, 71000 Sarajevo
 *                  Federacija Bosne i Hercegovine
 *                  Bosna i Hercegovina
 *                  tel.:+387 33 651 407
 *                  tel.:+387 33 718 670
 *                  fax.:+387 33 718 671
 *                  www.proven.ba
 *
 ******************************************************************************/
//
//
/** U K L J U C E N I *********************************************************/
//
//
#include <p18cxxx.h>
#include <stdlib.h>
#include <delays.h>
#include <usart.h>
#include <timers.h>
#include "defines.h"
#include "typedefs.h"
#include "io_cfg.h"
#include "main.h"
//
//
/** K O N F I G U R A C I J A  M I K R O K O N T R O L E R A  *****************/
//
//
#pragma config FOSC	= INTIO67 // interni oscilator ukljucen
#pragma config FCMEN	= OFF	// iskljucen fail safe clock monitor
#pragma config IESO     = OFF   // iskljucen preklopnik oscilatora
#pragma config PWRT 	= ON	// iskljucen timer zadrske po ukljucenju
#pragma config BOREN	= ON	// iskljucen monitoring napona napajanja
#pragma config WDTEN	= OFF	// iskljucen watch dog timer
#pragma config CCP2MX   = PORTC // CCP2 on PORTC 1
#pragma config PBADEN   = OFF   // PORTB 4:0 digital io on reset
#pragma config LPT1OSC  = OFF   // TIMER1 config for higher power operation
#pragma config HFOFST   = OFF   // wait until oscilator is stable
#pragma config MCLRE    = ON    // MCLR pin enabled
#pragma config LVP	= OFF	// onemoguceno programiranje ICSP bez povisenog napona
//
//
//
//
/** R A M   V A R I J A B L E *************************************************/
//
//
//#pragma udata char_vars
BYTE sys_flags;
//
#define POS_UPDATE      sys_flags.b0
#define ALIGNMENT_MODE  sys_flags.b1
//
//
//
BYTE enc_out;
//
#define ENC_4_OUT       enc_out.b0
#define ENC_8_OUT       enc_out.b1
#define ENC_10_OUT      enc_out.b2
#define ENC_20_OUT      enc_out.b3
#define ENC_40_OUT      enc_out.b4
#define ENC_80_OUT      enc_out.b5
#define ENC_100_OUT     enc_out.b6
#define ENC_200_OUT     enc_out.b7
//
//
//
//#pragma udata int_vars
volatile unsigned int actual_position;
volatile unsigned int enc_position;
volatile unsigned int temp_pos;
volatile WORD signal_clk;
//
//
//
#pragma code high_vector = 0x08
//
// <editor-fold defaultstate="collapsed" desc="asm high vector">

void interrupt_at_high_vector(void) {
    _asm goto high_isr _endasm
}// End of interrupt_at_high_vector
// </editor-fold>
//
#pragma code low_vector = 0x18
//
// <editor-fold defaultstate="collapsed" desc="asm low vector">

void interrupt_at_low_vector(void) {
    _asm goto low_isr _endasm
}// End of interrupt_at_low_vector
// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="high interrupt">
#pragma interrupt high_isr
//
//
//
void high_isr(void) 
{
    //
    //------------------------------------------- check TMR1
    //
    if (PIE1bits.TMR1IE && PIR1bits.TMR1IF) 
    {
        PIR1bits.TMR1IF = FALSE;
        WriteTimer1(0xf880);
        
        temp_pos = 0;            
        SSI_CLK = LOW;
        Delay10TCYx(10);
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0200;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0100;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0080;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0040;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0020;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0010;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0008;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0004;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0002;
        ClockSSI();
        if (SSI_DATA) temp_pos += 0x0001;
        ClockSSI();
        SSI_CLK = HIGH;
        
        if (!POS_UPDATE) 
        {
            POS_UPDATE = TRUE;
            actual_position = temp_pos;
        }
        
        if(signal_clk._word) --signal_clk._word;
        
    }// End of timer 1
}// End of high_isr
// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="low interrupt">
#pragma interruptlow low_isr
//
//
//

void low_isr(void) {
    //
    //------------------------------------------- check USART RX interrupt
    //
    //if (PIE1bits.RCIE && PIR1bits.RCIF) {
    //    //PIR1bits.RCIF = LOW;
    //    rx_char = getcUSART();
    //    if(rx_char == 'A') ALIGNMENT_MODE = TRUE;
    //    else if(rx_char == 'P') ALIGNMENT_MODE = FALSE;
    //}// End of USART RX INTERRUPT
    //
    //------------------------------------------- check USART TX interrupt
    //
    //if (PIE1bits.TXIE && PIR1bits.TXIF) {
    //    PIR1bits.TXIF = FALSE;
    //   PIE1bits.TXIE = FALSE;
    //}// End of USART TX INTERRUPT
    //
    //------------------------------------------- check TMR0
    //
//    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF)
//    {
//        INTCONbits.TMR0IF = FALSE; //clear interrupt flag
//        WriteTimer0(0xf060);
//        ++signal_clk._word;
//    }
}// End of low_isr
// </editor-fold>
//
//
//
#pragma code
//
// <editor-fold defaultstate="collapsed" desc="init system">

void InitSYS(void) 
{
    //------------------------------------------- config oscilator source
    //
    OSCCONbits.IRCF2 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.IRCF1 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.IRCF0 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.SCS1 = 0; //
    OSCCONbits.SCS0 = 0; //

    OSCTUNEbits.PLLEN = 1; // PLL enabled
    while (!OSCCONbits.IOFS) continue;
    //
    //------------------------------------------- config oscilator frequency
    //
    ANSEL = 0b00000000;
    ANSELH = 0b00000000;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    TRISA = 0x00;
    TRISB = 0x00;
    TRISC = 0b11110011;
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
    //
    //------------------------------------------- open timer 0
    //------------------------------------------- interrupt time base for
    //------------------------------------------- display driver
    //
//    OpenTimer0(TIMER_INT_ON &
//            T0_16BIT &
//            T0_SOURCE_INT &
//            T0_PS_1_1);
//    
//    WriteTimer0(0xf060);
    //
    //------------------------------------------- open timer 1
    //------------------------------------------- interrupt time base for
    //------------------------------------------- display driver
    //
    OpenTimer1(TIMER_INT_ON &
            T1_16BIT_RW &
            T1_SOURCE_INT &
            T1_PS_1_1);
    
    WriteTimer1(0xf880);    // 120us interrupt
    //
    //------------------------------------------- open usart 57600bps
    //
    //TXSTAbits.BRGH = TRUE;
    //TXSTAbits.TXEN = TRUE;
    //RCSTAbits.CREN = TRUE;
    //RCSTAbits.SPEN = TRUE;
    //BAUDCONbits.BRG16 = TRUE;
    //SPBRGH = 0;
    //SPBRG = 34;
    //
    //
    //------------------------------------------- open spi interface
    //
    //OpenSPI(SPI_FOSC_16, MODE_00, SMPEND);
    //SH1_OE = HIGH;
    //SH1_OE_TRIS = OUTPUT_PIN;
    //
    //------------------------------------------- interrupt pririty config
    //
    IPR1bits.TMR1IP = TRUE;
    //IPR1bits.RCIP = FALSE;
    //IPR1bits.TXIP = FALSE;
    INTCON2bits.TMR0IP = FALSE;
    //PIE1bits.RCIE = TRUE;
    //PIE1bits.TXIE = TRUE;
    RCONbits.IPEN = TRUE;
    INTCONbits.GIEL = TRUE;
    INTCONbits.GIEH = TRUE;
    //
    //------------------------------------------- interrupts enabled
    //
    SSI_NCS = LOW;
}// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="init RAM">

void InitRAM(void) 
{
    actual_position = 0;
    enc_position = 0;
    temp_pos = 0;
    ALIGNMENT_MODE = FALSE;
    DRIVER_OE = HIGH;
    
}// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="main">

void main(void) 
{

    InitSYS();
    InitRAM();

    while (1) 
    {
        ReadPosition();
        SignalService();
    }
    
}// </editor-fold>
//
//
// <editor-fold defaultstate="collapsed" desc="read sensor">

void ReadPosition(void) 
{
    
    if(!POS_UPDATE) return;
    
    actual_position = ((dword) actual_position * (dword)391) / (dword) 1000;
    
//    if (actual_position == 0) enc_position = 0;
//    else if (actual_position > enc_position) enc_position = actual_position;
//    
    enc_out._byte = 0;

    if (actual_position > 199) 
    {
        actual_position -= 200;
        ENC_200_OUT = HIGH;
    }
    
    if (actual_position > 99) 
    {
        actual_position -= 100;
        ENC_100_OUT = HIGH;
    } 
    
    if (actual_position > 79) 
    {
        actual_position -= 80;
        ENC_80_OUT = HIGH;
    }
    
    if (actual_position > 39) 
    {
        actual_position -= 40;
        ENC_40_OUT = HIGH;
    }
    
    if (actual_position > 19) 
    {
        actual_position -= 20;
        ENC_20_OUT = HIGH;
    }
    
    if (actual_position > 9) 
    {
        actual_position -= 10;
        ENC_10_OUT = HIGH;
    }
    
    if (actual_position > 7) 
    {
        actual_position -= 8;
        ENC_8_OUT = HIGH;
    }
    
    if (actual_position > 3) 
    {
        actual_position -= 4;
        ENC_4_OUT = HIGH;
    }
    
    if (actual_position > 1) 
    {
        actual_position -= 2;
        ENC_2 = HIGH;
    }
    else ENC_2 = LOW;
    
    if (actual_position > 0) 
    {
        ENC_1 = HIGH;
    }
    else ENC_1 = LOW;

    LATB = enc_out._byte;
    POS_UPDATE = FALSE;
    
}// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="signal service">

void SignalService(void) 
{
    if(temp_pos == 0) 
    {
        LED_ZERO = HIGH;
        SIG_1_S = HIGH;
    }
    else if(!SIG_1_S)
    {
        LED_ZERO = LOW;
    }
    
    if (MAG_HI) 
    {
        LED_ZERO = HIGH;
        if (SIG_128_MS) LED_STATUS = HIGH;
        else LED_STATUS = LOW;
    } 
    else if (MAG_LO)
    {
        LED_ZERO = HIGH;
        if (SIG_512_MS) LED_STATUS = HIGH;
        else LED_STATUS = LOW;
    } 
    else 
    {   
        LED_STATUS = HIGH;
    }
}// End of SignalService
// </editor-fold>
//
//
// <editor-fold defaultstate="collapsed" desc="eeprom read byte">

unsigned char EEPROM_ReadByte(unsigned char address) {
    EECON1 = 0;
    EEADR = address;
    EECON1bits.RD = 1;
    return (EEDATA);
}// End of EEPROM_ReadByte
// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="eeprom read int">

unsigned int EEPROM_ReadInt(unsigned char address) {
    volatile unsigned int data;
    data = EEPROM_ReadByte(address);
    data = (data << 8);
    data += EEPROM_ReadByte(address + 0x01);
    return data;
}// End of eeprom read int
// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="eeprom write byte">

void EEPROM_WriteByte(unsigned char address, unsigned char data) {
    volatile unsigned char tmp_int;
    //    SEGMENT_WR = 0x00;
    tmp_int = INTCON;
    INTCONbits.GIEH = FALSE;
    INTCONbits.GIEL = FALSE;
    EECON1 = 0; //ensure CFGS=0 and EEPGD=0
    EECON1bits.WREN = 1; //enable write to EEPROM
    EEADR = address; //setup Address
    EEDATA = data; //and data
    EECON2 = 0x55; //required sequence #1
    EECON2 = 0xaa; //#2
    EECON1bits.WR = 1; //#3 = actual write
    while (!PIR2bits.EEIF); //wait until finished
    EECON1bits.WREN = 0; //disable write to EEPROM
    INTCON = tmp_int;
    //Delay1KTCYx(50);
}// End of EEPROM_WriteByte
// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="eeprom write int">

void EEPROM_WriteInt(unsigned char address, unsigned int data) {
    EEPROM_WriteByte(address, (data >> 8));
    EEPROM_WriteByte((address + 0x01), data);
}// </editor-fold>
