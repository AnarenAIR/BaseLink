/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  HalfDuplexTransfer.c - acts as the Gateway node for the HalfDuplexTransfer
 *  example. Receives packets from the End Point node(s) and responds to them,
 *  if necessary.
 *
 *  @version    1.0.00
 *  @date       04 Feb 2013
 *  @author     BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - this is being compiled exclusively with Gateway node(s).
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

#include <string.h>       // memcpy
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
#define McuSleep()    _BIS_SR(LPM0_bits | GIE)  // Low power mode 0
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
 *  Callback function prototypes
 */

/**
 *  LinkRequest - acts as the callback function for the Protocol Link Request 
 *  event. If a link request is received and meets the addressing requirements,
 *  the Protocol Link Request event is triggered and a callback must be 
 *  implemented for notification.
 *
 *  Note: Please refer to API.h for more information on the LinkRequest callback
 *  for a Gateway node.
 */
bool LinkRequest(unsigned char *data, unsigned char length);

/**
 *  TransferComplete - acts as the callback function for the Protocol Data
 *  Transfer Complete event. When a frame is received that meets addressing
 *  requirements, the Protocol Data Transfer Complete event is triggered and a
 *  callback must be implemented to receive the frame payload (packet).
 *
 *  Note: Please refer to API.h for more information on the TransferComplete
 *  callback for a Gateway node.
 */
unsigned char TransferComplete(bool dataRequest,
                               unsigned char *data,
                               unsigned char length);

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

/**
 *  The following instance of sProtocolSetupInfo is used to initialize the
 *  protocol with required parameters. The Gateway node role has the following
 *  parameters [optional parameters are marked with an asterix (*)].
 *        
 *  { channel_list, pan_id, local_address, LinkRequest(*), TransferComplete(*) }
 *
 *  Note: Parameters marked with the asterix (*) may be assigned "NULL" if they
 *  are not needed.
 */
static const struct sProtocolSetupInfo gProtocolSetupInfo = {
  { PROTOCOL_CHANNEL_LIST },// Physical channel list
  { 0x01 },                 // Physical address PAN identifier
  { 0x01 },                 // Physical address
  LinkRequest,              // Protocol Link Request callback
  TransferComplete          // Protocol Data Transfer Complete callback
};

static struct sPacket gPacketRx = {
  0x00,                     // Set the initial sequence number value to 0
  ""                        // Set the initial payload to an empty string
};

static struct sPacket gPacketTx = {
  0x00,                     // Set the initial sequence number value to 0
  "World"                   // Set the initial payload to a "World" string
};

// -----------------------------------------------------------------------------

bool LinkRequest(unsigned char *data, unsigned char length)
{
  /**
   *  All incoming link requests are redirected to this callback. This is the
   *  area where final application validation should occur (e.g. a light switch
   *  application attempting to connect to a light source application). This
   *  validation can be done using the link request message (inside the data
   *  parameter above) or by addressing (please refer to sProtocolFrameInfo).
   */
  
  // For now, accept all incoming link requests.
  return true;
}

unsigned char TransferComplete(bool dataRequest,
                               unsigned char *data,
                               unsigned char length)
{
  // Cast the received data pointer to a packet structure pointer so that it may
  // be accessed using the structure member notation.
  struct sPacket *p = (struct sPacket*)data;
  
  // Retrieve the sequence number and copy the payload from the protocol into
  // the local application packet (gPacket).
  gPacketRx.seqNum = p->seqNum;
  memcpy(gPacketRx.payload, p->payload, length-1);
  
  // Check if a data request has been made.
  if (dataRequest)
  {
    // Provide any available data in a "data request" response transfer.
    ProtocolLoadDataResponse((unsigned char*)&gPacketTx, sizeof(struct sPacket));
    
    // Increment the sequence number for the next transmission.
    gPacketTx.seqNum++;
  }
  
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

int main(void)
{
  // Setup hardware and protocol.
  PlatformInit();
  
  /**
   *  For this example, all operations are performed inside the protocol ISR
   *  (ProtocolEngine) and callback function(s) (LinkRequest, TransferComplete). 
   *  The protocol puts the Gateway node into a receive state by default. The 
   *  application main loop can simply sleep.
   */
  while (true)
  {
    // Put the microcontroller into a low power state (sleep).
    McuSleep();
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
}
