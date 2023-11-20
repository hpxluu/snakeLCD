#include <msp430.h>

unsigned int memloc2[16][2] = {{9, 0x80}, {5, 0x80}, {3, 0x80}, {18, 0x80},
                               {14, 0x80}, {7, 0x80}, {7, 0x40}, {7, 0x20},
                               {7, 0x10}, {14, 0x10}, {18, 0x10}, {3, 0x10},
                               {5, 0x10}, {9, 0x10}, {9, 0x08}, {9, 0x04}};

void onboard_seg_display_init(void)
{
    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    LCDCPCTL0 = 0x8880;     // Init. LCD segments 4, 6-15
    LCDCPCTL1 = 0x200C;     // Init. LCD segments 16-21, 27-31
    LCDCPCTL2 = 0x0020;     // Init. LCD segments 35-39

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    LCDCCTL0 |= LCDON;
}

void delay(long delayTime) {
    volatile unsigned loops = delayTime; // Start the delay counter at 25,000
    while (--loops > 0);             // Count down until the delay counter reaches 0
}

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    onboard_seg_display_init();     // Init the LCD

    // init. external buttons
    P2DIR &= ~BIT1;
    P2REN |=  BIT1;
    P2OUT |=  BIT1;

    P2DIR &= ~BIT2;
    P2REN |=  BIT2;
    P2OUT |=  BIT2;

    P2DIR &= ~BIT3;
    P2REN |=  BIT3;
    P2OUT |=  BIT3;

    P2DIR &= ~BIT4;
    P2REN |=  BIT4;
    P2OUT |=  BIT4;

    P1DIR &= ~BIT1;
    P1REN |=  BIT1;
    P1OUT |=  BIT1;

    unsigned int length = 1;
    unsigned int i = 0;
    unsigned int j = i - length;
    long delayTime = 50000;
    int enable = 0;
    while(1) {

        i = i & 0x0F; // i % 16
        j = j & 0x0F;
        LCDMEM[memloc2[i][0]] |= memloc2[i++][1];
        LCDMEM[memloc2[j][0]] &= ~memloc2[j++][1];
        delay(delayTime);

        if(!(P2IN & BIT3)) {
            delayTime = delayTime + 2000;
        }

        if(!(P2IN & BIT4)) {
            delayTime = delayTime - 2000;
        }

        if (!(P2IN & BIT1)) { //increase
            length++;
            j = i - length;
        }

        if (!(P2IN & BIT2)) {
            LCDMEM[memloc2[j][0]] &= ~memloc2[j++][1];
            j = i - --length;
        }

        if (!(P1IN & BIT1)) {
            enable = 1;
            j = i + length;
            delay(25000);
        }

        while (enable == 1) {

           i = i & 0x0F;
           j = j & 0x0F;
           LCDMEM[memloc2[i][0]] |= memloc2[i--][1];
           LCDMEM[memloc2[j][0]] &= ~memloc2[j--][1];
           delay(delayTime);

           if (!(P2IN & BIT1)) { //increase
               length++;
               j = i + length;
           }

           if (!(P2IN & BIT2)) {
               LCDMEM[memloc2[j][0]] &= ~memloc2[j--][1];
               j = i + --length;
           }


           if(!(P2IN & BIT3)) {
               delayTime = delayTime + 2000;
           }

           if(!(P2IN & BIT4)) {
               delayTime = delayTime - 2000;
           }

           if (!(P1IN & BIT1)) {
               enable = 0;
               j = i - length;
               delay(25000);
           }

        }
    }
}
