/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  SimplexTransfer.c - performs a simplex transfer using the SimpleTransfer()
 *  API call. For each message sent, a packet sequence number is incremented.
 *  
 *
 *  @version    1.0.00
 *  @date       04 Feb 2013
 *  @author     BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - this is being compiled exclusively with End Point node(s).
 *  
 *  file dependency
 *  ===============
 *  API.h : defines the protocol API.
 *
 *  revision history
 *  ================
 *  ver 1.0.00 : 04 Feb 2013
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

#include "API.h"

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

#define ST(X) do { X } while (0)

/**
 *  Abstract hardware based on the supported example platform being used.
 * 
 *  Currently supported platforms:
 *    - TI MSP430G2553 + AIR A110x2500 Booster Pack (BPEXP430G2x53.c)
 */
#if defined( __MSP430G2553__ )
#define HardwareInit()\
  ST\
  (\
    WDTCTL = WDTPW | WDTHOLD;\
    BCSCTL1 = CALBC1_8MHZ;\
    DCOCTL = CALDCO_8MHZ;\
  )
#define McuSleep()    _BIS_SR(LPM4_bits | GIE)  // Go to low power mode 4
#define McuWakeup()   _BIC_SR(LPM4_EXIT);       // Wake up from low power mode 4
#define GDO0_VECTOR   PORT2_VECTOR
#define GDO0_EVENT    P2IFG
#endif

/**
 *  sPacket - an example packet. The sequence number is used to demonstrate
 *  communication by sending the same message (payload) and incrementing the
 *  sequence number on each transmission.
 */
struct sPacket
{
  unsigned char seqNum;     // Packet sequence number
  unsigned char payload[7]; // Packet payload
};

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

/**
 *  The following instance of sProtocolSetupInfo is used to initialize the
 *  protocol with required parameters. The End Point node role has the following
 *  parameters [optional parameters are marked with an asterix (*)].
 *        
 *  { channel_list, pan_id, local_address, Backup(*), TransferComplete(*) }
 *
 *  Note: Parameters marked with the asterix (*) may be assigned "NULL" if they
 *  are not needed.
 */
static const struct sProtocolSetupInfo gProtocolSetupInfo = {
  { PROTOCOL_CHANNEL_LIST },// Physical channel list
  { 0x01 },                 // Physical address PAN identifier
  { 0x02 },                 // Physical address
  NULL,                     // Protocol Backup callback (not used)
  NULL                      // Protocol Data Transfer Complete callback (not used)
};

static struct sPacket gPacket = {
  0x00,                     // Set the initial sequence number value to 0
  "Hello"                   // Set the initial payload to a "Hello" string
};

// -----------------------------------------------------------------------------

/**
 *  PlatformInit - sets up platform and protocol hardware. Also configures the
 *  protocol using the setup structure data.
 *
 *    @return   Success of the operation. If true, the protocol has been setup
 *              successfully. If false, an error has occurred during protocol
 *              setup.
 */
bool PlatformInit(void)
{
  // Disable global interrupts during hardware initialization to prevent any
  // unwanted interrupts from occurring.
  MCU_DISABLE_INTERRUPT();
  
  // Setup basic platform hardware (e.g. watchdog, clocks).
  HardwareInit();
  
  // Attempt to initialize protocol hardware and information using the provided
  // setup structure data.
  if (!ProtocolInit(&gProtocolSetupInfo))
  {
    return false;
  }
  
  // Re-enable global interrupts for normal operation.
  MCU_ENABLE_INTERRUPT();
  
  return true;
}

/**
 *  main - main application loop. Sets up platform and then performs simple
 *  transfers (simplex) while incrementing the sequence number for the lifetime 
 *  of execution.
 *
 *    @return   Exit code of the main application, however, this application
 *              should never exit.
 */
int main(void)
{
  // Setup hardware and protocol.
  PlatformInit();
  
  while (true)
  {
    // Perform a simple transfer of the packet.
    if (!ProtocolSimpleTransfer((unsigned char*)&gPacket, sizeof(struct sPacket)))
    {
      // Put the microcontroller into a low power state (sleep). Remain here
      // until the ISR wakes up the processor.
      McuSleep();
    }
    
    /**
     *  Check if the protocol is busy. If it is, a new transfer cannot occur
     *  until it becomes ready for the next instruction. Do not increment the
     *  sequence number until the protocol is ready. This prevents incrementing
     *  the sequence number more than once between transmissions.
     */
    if (!ProtocolBusy())
    {
      // Increment the sequence number for the next transmission.
      gPacket.seqNum++;
    }
  }
}

/**
 *  GDO0Isr - GDO0 interrupt service routine. This service routine will always
 *  be a I/O interrupt service routine. Therefore, it is important to pass the
 *  port flag to the ProtocolEngine so that the protocol can determine if a GDO0
 *  event has occurred.
 */
#pragma vector=GDO0_VECTOR
__interrupt void GDO0Isr(void)
{
  /**
   *  Store the port interrupt flag register so that it may be used to determine
   *  what caused the interrupt (e.g. did a GDO0 interrupt occur? The protocol 
   *  needs this information to determine what to do next...).
   */
  register volatile unsigned char event = GDO0_EVENT;
  
  /**
   *  Notify the protocol of a port event. The protocol will determine if it is
   *  associated with the GDO0 pin and act accordingly (e.g. if GDO0 has not
   *  triggered an interrupt then the protocol will not run, otherwise it will
   *  continue where it left off).
   *
   *  Note: Clearing of the GDO0 event (interrupt flag bit) is handled 
   *  internally. It is important that the application does not clear the GDO0
   *  event in this ISR.
   */
  ProtocolEngine(event);

  // Wake up the microcontroller to continue normal operation upon exiting the
  // ISR.
  McuWakeup();
}

// Note: No hardware timer interrupt required for this example because the 
// End Point node does not perform half duplex transfers.
