/******************************************************************************
 *
 *               Apsolutni enkoder ugla za ILLIG RDM K70
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
 * IDE:             MPLAB X IDE v1.95 (java 1.7.0_25)
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
//#include <usart.h>
#include <timers.h>
//#include <spi.h>
#include "typedefs.h"
#include "defines.h"
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
unsigned char tmr1;
unsigned char print_pos[8];
BYTE sys_flags;
//
//
//
//#pragma udata int_vars
unsigned int actual_position, old_position;
WORD signal_clk;
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
void high_isr(void) {
    //
    //------------------------------------------- check USART RX interrupt
    //
    if (PIE1bits.RCIE && PIR1bits.RCIF) {
        PIE1bits.RCIE = LOW;
        PIR1bits.RCIF = LOW;
    }// End of USART RX INTERRUPT
    //
    //------------------------------------------- check USART TX interrupt
    //
    if (PIE1bits.TXIE && PIR1bits.TXIF) {
        PIE1bits.TXIE = LOW;
         PIR1bits.TXIF = LOW;
    }// End of USART TX INTERRUPT
    //
    //------------------------------------------- check TMR0
    //
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = FALSE; //clear interrupt flag
        TMR0L = 0x06; // timer0 IRQ 1ms
        if(++tmr1 == 10) {
            tmr1 = 0;
            POS_UPDATE = TRUE;
        }
        
        ++signal_clk._word;
    }// End of if(INTCONbits.TMR0IE && .....
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
    //------------------------------------------- check interrupt source
    //
}// End of low_isr
// </editor-fold>
//
//
//
#pragma code
//
// <editor-fold defaultstate="collapsed" desc="init system">

void InitSYS(void) {
    //------------------------------------------- config oscilator source
    //
    OSCCONbits.IRCF2 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.IRCF1 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.IRCF0 = 1; // int.osc. freq. select 4 MHz
    OSCCONbits.SCS1 = 0; //
    OSCCONbits.SCS0 = 0; //

    //OSCTUNEbits.PLLEN = 1; // PLL enabled
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
    OpenTimer0(TIMER_INT_ON &
            T0_8BIT &
            T0_SOURCE_INT &
            T0_PS_1_16);
    //
    //------------------------------------------- open usart 19200bps
    //
    TXSTAbits.TX9 = FALSE;
    TXSTAbits.SYNC = FALSE;
    TXSTAbits.BRGH = LOW;
    TXSTAbits.TXEN = HIGH;
    RCSTAbits.ADDEN = FALSE;
    RCSTAbits.CREN = TRUE;
    RCSTAbits.RX9 = FALSE;
    RCSTAbits.SPEN = TRUE;
    BAUDCONbits.BRG16 = FALSE;
    SPBRGH = 0;
    SPBRG = 12;
    //PIE1bits.RCIE = HIGH;
    //PIE1bits.TXIE = HIGH;
    //
    //------------------------------------------- open spi interface
    //
    //OpenSPI(SPI_FOSC_16, MODE_00, SMPEND);
    //SH1_OE = HIGH;
    //SH1_OE_TRIS = OUTPUT_PIN;
    //
    //------------------------------------------- interrupt pririty config
    //
    INTCON2bits.TMR0IP = LOW;
    INTCONbits.GIEL = TRUE;
    INTCONbits.GIEH = TRUE;
    //
    //------------------------------------------- interrupts enabled
    //
}// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="init RAM">

void InitRAM(void) {

}// </editor-fold>
//
//
//
// <editor-fold defaultstate="collapsed" desc="main">

void main(void) {

    InitSYS();
    InitRAM();

    while (1) {
        ReadPosition();
        SignalService();
    }// End of while(1)
}// </editor-fold>
//
//
// <editor-fold defaultstate="collapsed" desc="read sensor">

void ReadPosition(void) {


    if(!POS_UPDATE) return;
    POS_UPDATE = FALSE;
    actual_position = 0;
    SSI_NCS = LOW;
    SSI_CLK = LOW;
    Delay1TCY();
    Delay1TCY();
    Delay1TCY();
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0200;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0100;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0080;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0040;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0020;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0010;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0008;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0004;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0002;
    SSI_Clock();
    if (SSI_DATA) actual_position += 0x0001;
    SSI_Clock();
    SSI_Clock();
    SSI_Clock();
    SSI_Clock();
    SSI_Clock();
    SSI_Clock();
    SSI_CLK = HIGH;
    Delay10TCY();
    Delay10TCY();
    Delay10TCY();
    Delay10TCY();
    if (actual_position != old_position) {
        old_position = actual_position;
        itoa(actual_position, print_pos);
        TXREG = print_pos[0];
        while (!TXSTAbits.TRMT) continue;
        TXREG = print_pos[1];
        while (!TXSTAbits.TRMT) continue;
        TXREG = print_pos[2];
        while (!TXSTAbits.TRMT) continue;
        TXREG = print_pos[3];
        while (!TXSTAbits.TRMT) continue;


        actual_position = (actual_position * 4) / 10;
        
        if (actual_position > 399) actual_position = 0;

        if (actual_position > 199) {
            actual_position -= 200U;
            ENC_200 = HIGH;
        } else ENC_200 = LOW;

        if (actual_position > 99U) {
            actual_position -= 100U;
            ENC_100 = HIGH;
        } else ENC_100 = LOW;

        if (actual_position > 79U) {
            actual_position -= 80U;
            ENC_80 = HIGH;
        } else ENC_80 = LOW;

        if (actual_position > 39U) {
            actual_position -= 40U;
            ENC_40 = HIGH;
        } else ENC_40 = LOW;

        if (actual_position > 19U) {
            actual_position -= 20U;
            ENC_20 = HIGH;
        } else ENC_20 = LOW;

        if (actual_position > 9U) {
            actual_position -= 10U;
            ENC_10 = HIGH;
        } else ENC_10 = LOW;

        if (actual_position > 7U) {
            actual_position -= 8U;
            ENC_8 = HIGH;
        } else ENC_8 = LOW;

        if (actual_position > 3U) {
            actual_position -= 4U;
            ENC_4 = HIGH;
        } else ENC_4 = LOW;

        if (actual_position > 1U) {
            actual_position -= 2U;
            ENC_2 = HIGH;
        } else ENC_2 = LOW;

        if (actual_position > 0U) {
            ENC_1 = HIGH;
        } else ENC_1 = LOW;
    }


}// </editor-fold>
//
//
//


//
//
//
// <editor-fold defaultstate="collapsed" desc="signal service">

void SignalService(void) {
    if (MAG_HI) {
        if (SIG_128_MS)LED_STATUS = LOW;
        else LED_STATUS = HIGH;
    } else if (MAG_LO) {
        if (SIG_64_MS)LED_STATUS = LOW;
        else LED_STATUS = HIGH;
    } else {
        if (SIG_512_MS)LED_STATUS = LOW;
        else LED_STATUS = HIGH;
    }// End of else...
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
