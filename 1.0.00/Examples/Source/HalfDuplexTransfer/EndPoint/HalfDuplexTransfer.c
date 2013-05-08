/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  HalfDuplexTransfer.c - performs a half duplex (two-way) transfer using the 
 *  Transfer() API call. Begins by connecting to a Gateway in the PAN. For each 
 *  message sent, a packet sequence number is incremented.
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
 *  string.h : defines memcpy which is used to copy one buffer to another
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

#include <string.h>     // memcpy
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
#define McuSleep()    _BIS_SR(LPM0_bits | GIE)  // Go to low power mode 0
#define McuWakeup()   _BIC_SR(LPM0_EXIT);       // Wake up from low power mode 0
#define GDO0_VECTOR   PORT2_VECTOR
#define GDO0_EVENT    P2IFG
#define TIMER_VECTOR  TIMER1_A0_VECTOR
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
 *  Callback function prototypes
 */

/**
 *  TransferComplete - acts as the callback function for the Protocol Data
 *  Transfer Complete event. When a frame is received that meets addressing
 *  requirements, the Protocol Data Transfer Complete event is triggered and a
 *  callback must be implemented to receive the frame payload (packet).
 *
 *  Note: Please refer to API.h for more information on the TransferComplete
 *  callback for an End Point node.
 */
unsigned char TransferComplete(unsigned char *data,
                               unsigned char length);

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
  TransferComplete          // Protocol Data Transfer Complete callback
};

static struct sPacket gPacketTx = {
  0x00,                     // Set the initial sequence number value to 0
  "Hello"                   // Set the initial payload to a "Hello" string
};

static struct sPacket gPacketRx = {
  0x00,                     // Set the initial sequence number value to 0
  ""                        // Set the initial payload to an empty string
};

// -----------------------------------------------------------------------------

unsigned char TransferComplete(unsigned char *data,
                               unsigned char length)
{
  // Cast the received data pointer to a packet structure pointer so that it may
  // be accessed using the structure member notation.
  struct sPacket *p = (struct sPacket*)data;
  
  // Retrieve the sequence number and copy the payload from the protocol into
  // the local application packet (gPacketRx).
  gPacketRx.seqNum = p->seqNum;
  memcpy(gPacketRx.payload, p->payload, length-1);
  
  return 0;
}

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
 *  main - main application loop. Sets up platform, connects to a Gateway, and 
 *  then performs data transfers (half-duplex) while incrementing the sequence 
 *  number for the lifetime of execution.
 *
 *    @return   Exit code of the main application, however, this application
 *              should never exit.
 */
int main(void)
{
  // Setup hardware and protocol.
  PlatformInit();
  
  while (!ProtocolStatusPhyAddressInfo().connected)
  {
    if (!ProtocolConnect(NULL, 0))
    {
      // Put the microcontroller into a low power state (sleep). Remain here
      // until the ISR wakes up the processor.
      McuSleep();
    }
  }
  
  while (true)
  {
    // Perform a data transfer of the packet.
    if (!ProtocolTransfer((unsigned char*)&gPacketTx, sizeof(struct sPacket)))
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
      gPacketTx.seqNum++;
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

/**
 *  TimerIsr - Provides the protocol a tick using a 1ms hardware timer 
 *  interrupt. The protocol controls when the timer is enabled and disabled. 
 *  Therefore, it is critical that the application does not rely on this timer 
 *  for application-related tasks as it may not behave as expected.
 */
#pragma vector=TIMER_VECTOR
__interrupt void TimerIsr()
{
  // Notify the protocol of a timer interrupt. The protocol uses this interrupt
  // as a tick to increment any counters that are actively running.
  ProtocolEngineTick();
  
  // Wake up the microcontroller to continue normal operation upon exiting the
  // ISR.
  McuWakeup();
}
