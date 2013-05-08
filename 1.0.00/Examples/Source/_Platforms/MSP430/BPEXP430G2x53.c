/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  EXP430G2x53.c - physical bridge implementation using A110x2500-based module
 *		in conjunction with an MSP430G2x53 microcontroller. Hardware pins are mapped
 *		based on the EXP430G2 development platform.
 *
 *  @version    1.0.00
 *  @date       08 Oct 2012
 *  @author     BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - used with a A1101R04, A1101R08, A1101R09, A110LR09, or A2500R24 Booster
 *  Pack platform
 *  
 *  file dependency
 *  ===============
 *  A110x2500PhyBridge.h : defines the interface for porting the protocol.
 *		msp430g2553.h : defines MCU specific registers.
 *
 *  revision history
 *  ================
 *  ver 1.0.00 : 08 Oct 2012
 *  - initial release
 */
#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#include "A110x2500PhyBridge.h"

// Supported microcontrollers
#if defined( __MSP430G2553__ )
#include "msp430g2553.h"
#else
#error "Board Error 0100: Selected microcontroller is not supported"
#endif

// -----------------------------------------------------------------------------
/**
 *  Definitions, enumerations, and structures
 */

/**
 *  RF SPI bus interface
 */

// SPI SCLK
#define RF_SPI_SCLK       (0x0020u)

// SPI MOSI
// TODO: Determine if there are multiple positions for MOSI using USCIB0.
#define RF_SPI_MOSI       (0x0080u)

// SPI MISO 
// TODO: Determine if there are multiple positions for MISO using USCIB0.
#define RF_SPI_MISO       (0x0040u)
#define RF_SPI_MISO_IN    P1IN

// SPI CSn
#if defined( RF_SPI_CSN_1 )
// Chip Select position 1 (default) - P2.7
#define RF_SPI_CSN        (0x0080u)
#define RF_SPI_CSN_OUT    P2OUT
#define RF_SPI_CSN_DIR    P2DIR
#define RF_SPI_CSN_SEL    P2SEL
#define RF_SPI_CSN_SEL2   P2SEL2
#elif defined( RF_SPI_CSN_2 )
// Chip Select position 2 - P1.2
#define RF_SPI_CSN        (0x0004u)
#define RF_SPI_CSN_OUT    P1OUT
#define RF_SPI_CSN_DIR    P1DIR
#define RF_SPI_CSN_SEL    P1SEL
#define RF_SPI_CSN_SEL2   P1SEL2
#elif defined( RF_SPI_CSN_3 )
// Chip Select position 3 - P1.4
#define RF_SPI_CSN        (0x0010u)
#define RF_SPI_CSN_OUT    P1OUT
#define RF_SPI_CSN_DIR    P1DIR
#define RF_SPI_CSN_SEL    P1SEL
#define RF_SPI_CSN_SEL2   P1SEL2
#else
#error "Board Error 0101: CSn selection invalid. Please select CSn position 1, 2, or 3."
#endif

/**
 *  RF GDOx interface
 */

// GDO0
#if defined( RF_GDO0_1 )
// GDO0 selection position 1 (default) - P2.6
#define RF_GDO0           (0x0040u)
#define RF_GDO0_IN        P2IN
#define RF_GDO0_DIR       P2DIR
#define RF_GDO0_IE        P2IE
#define RF_GDO0_IES       P2IES
#define RF_GDO0_IFG       P2IFG
#define RF_GDO0_SEL       P2SEL
#define RF_GDO0_SEL2      P2SEL2
#elif defined( RF_GDO0_2 )
// GDO0 selection position 2 - P1.3
#define RF_GDO0           (0x0008u)
#define RF_GDO0_IN        P1IN
#define RF_GDO0_DIR       P1DIR
#define RF_GDO0_IE        P1IE
#define RF_GDO0_IES       P1IES
#define RF_GDO0_IFG       P1IFG
#define RF_GDO0_SEL       P1SEL
#define RF_GDO0_SEL2      P1SEL2
#elif defined( RF_GDO0_3 )
// GDO0 selection position 3 - P1.1
#define RF_GDO0           (0x0002u)
#define RF_GDO0_IN        P1IN
#define RF_GDO0_DIR       P1DIR
#define RF_GDO0_IE        P1IE
#define RF_GDO0_IES       P1IES
#define RF_GDO0_IFG       P1IFG
#define RF_GDO0_SEL       P1SEL
#define RF_GDO0_SEL2      P1SEL2
#else
#error "Board Error 0102: GDO0 selection invalid. Please select GDO0 position 1, 2, or 3."
#endif

/**
 *  Timer control
 */
#if defined( TIMER0_A )
#define TIMER_INIT()\
   ST(TA0CTL = TASSEL_2 | ID_3 | MC_0;\
   TA0CCTL0 |= CCIE;\
   TA0CCR0 = 1000;)
#define TIMER_START()       ST(TA0CTL &= ~MC_3; TA0CTL |= MC_1;)
#define TIMER_STOP()        ST(TA0CCTL0 &= ~CCIFG; TA0CTL &= ~MC_3;)
#elif defined( TIMER1_A )
#define TIMER_INIT()\
   ST(TA1CTL = TASSEL_2 | ID_3 | MC_0;\
   TA1CCTL0 |= CCIE;\
   TA1CCR0 = 1000;)
#define TIMER_START()       ST(TA1CTL &= ~MC_3; TA1CTL |= MC_1;)
#define TIMER_STOP()        ST(TA1CCTL0 &= ~CCIFG; TA1CTL &= ~MC_3;)
#else
#error "Board Error 0103: Timer selection invalid. Please select TIMER0_A or TIMER1_A."
#endif

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

// -----------------------------------------------------------------------------
/**
 *  Private interface
 */

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

// -----------------------------------------------------------------------------
// A110x2500 RF serial peripheral interface (SPI)

void A110x2500SpiInit()
{
  // Setup CSn line.
  RF_SPI_CSN_DIR |= RF_SPI_CSN;
  RF_SPI_CSN_OUT |= RF_SPI_CSN;
  RF_SPI_CSN_SEL &= ~RF_SPI_CSN;
  RF_SPI_CSN_SEL2 &= ~RF_SPI_CSN;
  
  // Setup the USCIB0 peripheral for SPI operation.
  // Note: The current implementation assumes the SMCLK doesn't exceed 10MHz.
  UCB0CTL1 |= UCSWRST;
  UCB0CTL0 |= (UCMODE_0 | UCCKPH | UCMSB | UCMST | UCSYNC);
  UCB0CTL1 |= UCSSEL_2;
  UCB0BR1 = 0;
  UCB0BR0 = 2;
  
  // Setup SCLK, MOSI, and MISO lines.
  P1SEL |= RF_SPI_SCLK | RF_SPI_MOSI | RF_SPI_MISO;
  P1SEL2 |= RF_SPI_SCLK | RF_SPI_MOSI | RF_SPI_MISO;
  
  UCB0CTL1 &= ~UCSWRST;
}

void A110x2500SpiRead(unsigned char address,
											unsigned char *buffer,
											unsigned char count)
{
  register volatile unsigned char i;    // Buffer iterator
  /**
   *  Note: The buffer iterator should not be used as an offset. If the iterator 
   *  were used, some compilers may issue a warning similar to the following: 
   * 
   *    "...undefined behavior: the order of volatile accesses is undefined in 
   *    this statement".
   *
   *  To prevent this, a new variable is introduced to handle acting as an 
   *  offset.
   */
  unsigned char j;                      // Buffer offset
  
  // Change MISO pin to SPI.
  P1SEL |= RF_SPI_MISO;
  P1SEL2 |= RF_SPI_MISO;
  
  RF_SPI_CSN_OUT &= ~RF_SPI_CSN;
  // Look for CHIP_RDYn from radio.
  while (RF_SPI_MISO_IN & RF_SPI_MISO);

	// Write the address/command byte.
	IFG2 &= ~UCB0RXIFG;
  UCB0TXBUF = address;
  while (!(IFG2 & UCB0RXIFG));
	IFG2 &= ~UCB0RXIFG;

	// Write dummy byte(s) and read response(s).
  for (i = 0, j = 0; i < count; i++, j++)
  {
		while (!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = 0xFF;
    while (!(IFG2 & UCB0RXIFG));
    *(buffer+j) = UCB0RXBUF;
  }

	// Wait for operation to complete.
  while(UCB0STAT & UCBUSY);
  RF_SPI_CSN_OUT |= RF_SPI_CSN;
  
  // Change MISO pin to general purpose output (LED use if available).
  P1SEL &= ~RF_SPI_MISO;
  P1SEL2 &= ~RF_SPI_MISO;
}

void A110x2500SpiWrite(unsigned char address,
											 const unsigned char *buffer,
											 unsigned char count)
{
  register volatile unsigned char i;    // Buffer iterator
  
  // Change MISO pin to SPI.
  P1SEL |= RF_SPI_MISO;
  P1SEL2 |= RF_SPI_MISO;
  
  RF_SPI_CSN_OUT &= ~RF_SPI_CSN;
  // Look for CHIP_RDYn from radio.
  while (RF_SPI_MISO_IN & RF_SPI_MISO);

	// Write the address/command byte.
  UCB0TXBUF = address;

	// Write data byte(s).
  for (i = 0; i < count; i++)
  {
    while (!(IFG2 & UCB0TXIFG));
    UCB0TXBUF = *(buffer+i);
  }

	// Wait for operation to complete.
  while(UCB0STAT & UCBUSY);
  RF_SPI_CSN_OUT |= RF_SPI_CSN;
  
  // Change MISO pin to general purpose output (LED use if available).
  P1SEL &= ~RF_SPI_MISO;
  P1SEL2 &= ~RF_SPI_MISO;
}

// -----------------------------------------------------------------------------
// A110x2500 RF general digital output (GDO)

void A110x2500Gdo0Init()
{
  RF_GDO0_DIR &= ~RF_GDO0;
  RF_GDO0_IES &= ~RF_GDO0;
  RF_GDO0_IFG &= ~RF_GDO0;
  RF_GDO0_SEL &= ~RF_GDO0;
  RF_GDO0_SEL2 &= ~RF_GDO0;
}

bool A110x2500Gdo0Event(unsigned char event)
{
  if (RF_GDO0 & event)
  {
    // Clear GDO0 event.
    RF_GDO0_IFG &= ~RF_GDO0;
    return true;
  }
  return false;
}

void A110x2500Gdo0WaitForAssert()
{
  RF_GDO0_IES &= ~RF_GDO0;
}

void A110x2500Gdo0WaitForDeassert()
{
  RF_GDO0_IES |= RF_GDO0;
}

enum eCC1101GdoState A110x2500Gdo0GetState()
{
  return (RF_GDO0_IES & RF_GDO0) 
    ? eCC1101GdoStateWaitForDeassert
      : eCC1101GdoStateWaitForAssert;
}

void A110x2500Gdo0Enable(bool en)
{
  RF_GDO0_IFG &= ~RF_GDO0;
  if (en)
  {
    RF_GDO0_IE |= RF_GDO0;
  }
  else
  {
    RF_GDO0_IE &= ~RF_GDO0;
  }
}

// -----------------------------------------------------------------------------
// Hardware Timer

void A110x2500HwTimerInit()
{
  TIMER_INIT();
}

void A110x2500HwTimerStart()
{
  TIMER_START();
}

void A110x2500HwTimerStop()
{
  TIMER_STOP();
}
