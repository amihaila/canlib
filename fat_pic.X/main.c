/*
 * File:   main.c
 * Author: alexm
 *
 * Created on July 17, 2018, 7:16 PM
 */

#define _XTAL_FREQ 4000000
#include "mcp_2515.h"
#include "config.h"
#include "plib.h"
#include <stdint.h>

#define LED_ON (TRISD2 = 0)
#define LED_OFF (TRISD2 = 1)

// CNF1 SJW = 0
// CNF1 BRP = 0b11111
static void can_init_spi() {
    TRISD3 = 0;
    LATD3 = 1;
    __delay_ms(100);    // yeah yeah
    
    // set to config mode. top 3 bits are 0b100
    mcp_write_reg(CANCTRL, 0x4 << 5);
    while (!(mcp_read_reg(CANCTRL))) {
        __delay_ms(5);
    }
    
    // hard code timing params for now until shit works
    
    // CNF1
    uint8_t sjw = 0b11;
    uint8_t brp = 0;
    mcp_write_reg(CNF1, sjw << 6 | brp);

    // CNF2
    uint8_t btlmode = 1;
    uint8_t sam = 0;
    uint8_t phseg1 = 0b100;
    uint8_t prseg1 = 0;
    mcp_write_reg(CNF2, btlmode << 7 | sam << 6 | phseg1 << 3 | prseg1);
    
    // CNF3
    uint8_t phseg2 = 0b100;
    mcp_write_reg(CNF3, phseg2);
    
    // set normal mode (top 3 bits = 0, set clock output)
    // set one shot mode - no retransmission
    mcp_write_reg(CANCTRL, 0xc);

    // normal mode: top 3 bits are 0
    while (mcp_read_reg(CANCTRL) & 0xe0 != 0);   // wait for normal mode
}

// quick and dirty and completely garbage
static void can_send(uint16_t sid) {
    mcp_write_reg(CANINTF, 0);     // clear interrupt flag register
    mcp_write_reg(EFLG, 0);
    
    mcp_write_reg(TXB0SIDH, (uint8_t) (sid >> 3));          // set sid
    mcp_write_reg(TXB0SIDL, (sid & 0x7) << 5);              // set sid

    // data message w/ some number of bytes
    mcp_write_reg(TXB0DLC, 0);
    while (mcp_read_reg(TXB0DLC) != 0) {
        LED_ON;
        __delay_ms(100);
        LED_OFF;
        __delay_ms(100);
    }
    
    /*mcp_write_reg(0x37, 0xAA);*/
    /*mcp_write_reg(0x38, 0xAA);*/
    /*mcp_write_reg(0x39, 0xAA);*/
    /*mcp_write_reg(0x3A, 0xAA);*/
    /*mcp_write_reg(0x3b, 0xAA);*/
    /*mcp_write_reg(0x3c, 0xAA);*/
    /*mcp_write_reg(0x3d, 0xAA);*/
    mcp_write_reg(TXB0CTRL, 1 << 3);            // set txreq
    
    mcp_read_reg(TXB0CTRL);
    if (mcp_read_reg(EFLG)) {   // elfg
        LED_ON; // turn on if error
    } else {
        LED_OFF;
    }
}

void main(void) {
    // set up clock
    OSCCONbits.IRCF = 0x6;  // internal osc freq sel: 4 MHz
    OSCCONbits.SCS = 0x2;   // system clock sel
    
    // set an led for testing
    // led is on RD2
    TRISD2 = 0;     // output
    LED_OFF;

    // sync mode, bus mode, phase
    OpenSPI(SPI_FOSC_64, MODE_11, SMPMID);
   
    can_init_spi();
    mcp_read_reg(TXB0CTRL);     // txb0ctrl
    mcp_read_reg(EFLG); // eflg

    while (1) {
        can_send(0x2aa);
        __delay_ms(200);
        
        can_send(0x444);
        __delay_ms(200);
    }
    return;
}
