#ifndef A110X2500_PHY_BRIDGE_H
#define A110X2500_PHY_BRIDGE_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  A110x2500PhyBridge.h - physical bridge implementation using A110x2500-based 
 *		modules.
 *
 *  @version  1.0.01
 *  @date     16 Oct 2012
 *  @author   BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  none
 *
 *  file dependency
 *  ===============
 *  PhyBridge : provides interface function prototypes and global definitions.
 *  A1101R04 : provides Anaren A1101R04 driver definitions.
 *  A1101R08 : provides Anaren A1101R08 driver definitions.
 *  A1101R09 : provides Anaren A1101R09 driver definitions.
 *  A110LR09 : provides Anaren A110LR09 driver definitions.
 *  A2500R24 : provides Anaren A2500R24 driver definitions.
 *
 *  revision history
 *  ================
 *  ver 1.0.01 : 16 Oct 2012
 *  - updated internal documentation; comments revised
 *  ver 1.0.00 : 24 Jul 2012 
 *  - initial release
 */
#define A110X2500_PHY_BRIDGE_INFO  "A110X2500_PHY_BRIDGE 1.0.01"
   
#include "PhyBridge.h" 

#if defined( A1101R04_MODULE )
#include "A1101R04.h"
#elif defined( A1101R08_MODULE )
#include "A1101R08.h"
#elif defined( A1101R09_MODULE )
#include "A1101R09.h"
#elif defined( A110LR09_MODULE )
#include "A110LR09.h"
#elif defined( A2500R24_MODULE )
#include "A2500R24.h"
#else
#error "A110x2500 Physical Error 0100: RF module selected is not supported"
#endif

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

/**
 *  A110x2500SpiInit - initialize the SPI interface. Setup peripheral necessary for
 *  SPI communication and I/O port.
 */
void A110x2500SpiInit(void);

/**
 *  A110x2500SpiRead - read from a CC1101-based module.
 *
 *    @param  address The register address to start reading from.
 *    @param  buffer  A buffer used for storing values read from the CC1101. It 
 *                    is dependent on count.
 *    @param  count   Number of bytes to read from the hardware registers.
 */
void A110x2500SpiRead(unsigned char address,
																				  unsigned char *buffer,
																						unsigned char count);

/**
 *  A110x2500SpiWrite - write to a CC1101-based module.
 *
 *    @param  address The register address to start writing to.
 *    @param  buffer  A buffer that contains values to be written to the CC1101. 
 *                    It is dependent on count.
 *    @param  count   Number of bytes to write to the hardware registers.
 */
void A110x2500SpiWrite(unsigned char address,
																						 const unsigned char *buffer,
																							unsigned char count);

/**
 *  A110x2500Gdo0Init - initialize the GDO0 port.
 */
void A110x2500Gdo0Init(void);

/**
 *  A110x2500Gdo0Event - determine if the GDO0 port has caused an interrupt.
 *
 *    @param  event Bit field (or register) with current interrupt flags values.
 *
 *    @return True if a GDO0 event has occurred (interrupt), otherwise false
 *            (the interrupt was due to another source).
 */
bool A110x2500Gdo0Event(unsigned char event);

/**
 *  A110x2500Gdo0WaitForAssert - change the polarity of the GDO0 interrupt to wait
 *  for an assertion.
 */
void A110x2500Gdo0WaitForAssert(void);

/**
 *  A110x2500Gdo0WaitForDeassert - change the polarity of the GDO0 interrupt to wait
 *  for a deassertion.
 */
void A110x2500Gdo0WaitForDeassert(void);

/**
 *  A110x2500Gdo0GetState - get the current GDO0 interrupt polarity.
 *
 *    @return Current polarity state (waiting for assert or deassert).
 */
enum eCC1101GdoState A110x2500Gdo0GetState(void);

/**
 *  A110x2500Gdo0Enable - enable/disable the GDO0 interrupt.
 *
 *    @param  en  Enable flag.
 */
void A110x2500Gdo0Enable(bool en);

/**
 *  A110x2500HardwareTimerInit - initialize the hardware timer.
 *
 *  Note: The Physical Bridge requires a 16-bit hardware timer with at least a 
 *  1ms tick rate. The tick rate should be calculated as 1ms + crystal error %.
 */
void A110x2500HwTimerInit(void);

/**
 *  A110x2500HardwareTimerStart - start the hardware timer.
 */
void A110x2500HwTimerStart(void);

/**
 *  A110x2500HardwareTimerStop - stop the hardware timer.
 */
void A110x2500HwTimerStop(void);

#endif  /* A110X2500_PHY_BRIDGE_H */
