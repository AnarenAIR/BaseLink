/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  Frame.c - Data Link layer Media Access Control (MAC) framing and scheduling 
 *  sub layer.
 *
 *  @version    1.0.01
 *  @date       17 Sep 2012
 *  @author     BPB, air@anaren.com
 *
 *  For details on the interface, please see Frame.h.
 *
 *  assumptions
 *  ===========
 *  Same as Frame.h assumptions
 *
 *  file dependency
 *  ===============
 *  string.h : defines the functions "memcpy" that is used for copying addresses
 *  Frame.h : provides interface function prototypes and global definitions
 *
 *  revision history
 *  ================
 *  ver 1.0.01 : 16 Oct 2012
 *  - updated internal documentation; comments revised
 *  - added a test example (stub) to perform various frame operations
 *  ver 1.0.00 : 17 Sep 2012
 *  - initial release
 */
#include <string.h>   // memcpy
#include "Frame.h"

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

static struct sFrameScheduler gFrameScheduler;  // MAC scheduler information

// -----------------------------------------------------------------------------
/**
 *  Private interface
 */

// -----------------------------------------------------------------------------
// Frame operations

void FrameBuild(enum eFrameType type,
                bool dataRequest,
                unsigned char *payload,
                unsigned char length)
{
  static unsigned char seqNumber = 0;

  // Update frame destination and source.
  PhyAddressCopy(gFrameScheduler.frame.header.panId,
                 PhyAddressGetLocalInfo()->panId,
                 PROTOCOL_PHYADDRESS_PANID_SIZE);
  #if defined( PROTOCOL_ENDPOINT )
  // End Point can only transmit to its remote address (Gateway node).
  PhyAddressCopy(gFrameScheduler.frame.header.destAddr, 
                 PhyAddressGetLocalInfo()->remote, 
                 PROTOCOL_PHYADDRESS_ADDRESS_SIZE);
  #elif defined( PROTOCOL_GATEWAY )
  // Gateway can respond to nodes that contacted it (simply swap the destination
  // and source addresses).
  PhyAddressCopy(gFrameScheduler.frame.header.destAddr,
                 gFrameScheduler.frame.header.srcAddr,
                 PROTOCOL_PHYADDRESS_ADDRESS_SIZE);
  #endif
  PhyAddressCopy(gFrameScheduler.frame.header.srcAddr,
                 PhyAddressGetLocalInfo()->address,
                 PROTOCOL_PHYADDRESS_ADDRESS_SIZE);
  
  gFrameScheduler.frame.header.control &= ~FRAME_CONTROL_TYPE;
  gFrameScheduler.frame.header.control |= type;
  gFrameScheduler.frame.header.control &= ~FRAME_CONTROL_DATA_REQ;
  if (dataRequest)
  {
    gFrameScheduler.frame.header.control |= FRAME_CONTROL_DATA_REQ;
  }
  #if defined( PROTOCOL_ENDPOINT )
  gFrameScheduler.frame.header.control &= ~FRAME_CONTROL_MODE;
  #elif defined( PROTOCOL_GATEWAY )
  gFrameScheduler.frame.header.control |= FRAME_CONTROL_MODE;
  #endif
  gFrameScheduler.frame.header.seqNumber = seqNumber++;
    
  // Copy the payload into the internal frame buffer.
  gFrameScheduler.length = length;
  memcpy(&gFrameScheduler.frame.payload, payload, length);
}

/**
 *  FrameEndPointValidate - validate the incoming frame. Check if the frame was
 *  destined for the local node and that it meets all the addressing 
 *  requirements to be considered a valid frame.
 *
 *    @param  destPan   Incoming frame destination PAN identifer.
 *    @param  destAddr  Incoming frame destination address.
 *
 *    @return Success of validating the incoming frame. If the frame is valid,
              return true. Otherwise return false.
 */
#if defined( PROTOCOL_ENDPOINT )
bool FrameEndPointValidate(unsigned char destPan[PHY_ADDRESS_PANID_SIZE],
                           unsigned char destAddr[PHY_ADDRESS_ADDRESS_SIZE])
{
  const struct sPhyAddressLocal* local = PhyAddressGetLocalInfo();
  
  // Verify that the received frame was intended for the local node.
  // Was the received link request from a Gateway node?
  if ((gFrameScheduler.frame.header.control & FRAME_CONTROL_MODE) != FRAME_CONTROL_MODE_GATEWAY)
  {
    return false;
  }
  
  // Verify destination address.
  if (PhyAddressCompare(local->address, destAddr, PHY_ADDRESS_ADDRESS_SIZE) != 0)
  {
    return false;
  }
  
  // Verify PAN identifier.
  if (PhyAddressCompare(local->panId, destPan, PHY_ADDRESS_PANID_SIZE) != 0)
  {
    if ((gFrameScheduler.frame.header.control & FRAME_CONTROL_TYPE) != eFrameTypeLinkRequest)
    {
      return false;
    }
    else
    {
      /**
       *  Check if a link already exists or if the local PAN identifier is fixed.
       *  If a link exists, ignore the invalid link request as this End Point
       *  can only be linked to one Gateway at any time. If the PAN identifier 
       *  is fixed, disallow any PAN identifier except ones that are in the 
       *  local node's PAN.
       */
      if (PhyAddressLinkExists() || local->fixed)
      {
        return false;
      }
    }
  }

  return true;
}
#endif

/**
 *  FrameGatewayValidate - 
 *
 *    @param  destPan
 *    @param  destAddr
 *
 *    @return 
 */
#if defined( PROTOCOL_GATEWAY )
bool FrameGatewayValidate(unsigned char destPan[PHY_ADDRESS_PANID_SIZE],
                          unsigned char destAddr[PHY_ADDRESS_ADDRESS_SIZE])
{
  const struct sPhyAddressLocal* local = PhyAddressGetLocalInfo();
  
  // Verify that the received frame was intended for the local node.
  // Was the received link request from a Gateway node?
  if ((gFrameScheduler.frame.header.control & FRAME_CONTROL_MODE) != FRAME_CONTROL_MODE_ENDPOINT)
  {
    return false;
  }
  
  // Verify destination address.
  if (PhyAddressCompare(local->address, destAddr, PHY_ADDRESS_ADDRESS_SIZE) != 0)
  {
    // A Gateway may accept a message to the broadcast address.
    if (!PhyAddressIsBroadcast(destAddr, PHY_ADDRESS_ADDRESS_SIZE))
    {
      return false;
    }
  }
  
  // Verify PAN identifier.
  if (PhyAddressCompare(local->panId, destPan, PHY_ADDRESS_PANID_SIZE) != 0)
  {
    if (gFrameScheduler.frame.header.control != eFrameTypeLinkRequest)
    {
      if (!PhyAddressIsBroadcast(destPan, PHY_ADDRESS_PANID_SIZE))
      {
        return false;
      }
    }
    else
    {
      // Check if the address is to the broadcast PAN identifier.
      if (!PhyAddressIsBroadcast(destPan, PHY_ADDRESS_PANID_SIZE))
      {
        return false;
      }
    }
  }

  return true;
}
#endif

// -----------------------------------------------------------------------------
// Frame scheduler operations

/**
 *  FrameSchedulerData - scheduler operation to process all incoming data
 *  frames.
 *
 *    @return Status message from callee (currently not being used for frame 
 *            use).
 */
unsigned char FrameSchedulerData(void)
{ 
  // Pass the payload (without frame header and footer) up to the next layer.
  if (gFrameScheduler.FrameComplete != NULL)
  {
    #if defined( PROTOCOL_GATEWAY )
    bool dataRequest = (gFrameScheduler.frame.header.control & FRAME_CONTROL_DATA_REQ) >> 1;
    #endif
    unsigned char statusMessage = 0;
    
    gFrameScheduler.frame.header.control &= ~FRAME_CONTROL_DATA_REQ;
    
    #if defined( PROTOCOL_ENDPOINT )
    statusMessage = gFrameScheduler.FrameComplete(gFrameScheduler.frame.payload, 
                                                  gFrameScheduler.length);
    #elif defined( PROTOCOL_GATEWAY )
    statusMessage = gFrameScheduler.FrameComplete(dataRequest,
                                                  gFrameScheduler.frame.payload, 
                                                  gFrameScheduler.length);
    #endif
    
    #if defined( PROTOCOL_GATEWAY )
    // Send data back to the requesting node, if required.
    if (dataRequest && (gFrameScheduler.dataResponse.length > 0))
    {
      // Send a data response.
      PhyEnable();
      FrameSend(eFrameTypeData, 
                dataRequest, 
                gFrameScheduler.dataResponse.payload, 
                gFrameScheduler.dataResponse.length);
    }
    #endif

    // Return user callback status message.
    return statusMessage;
  }
  
  return 0;
}

/**
 *  FrameSchedulerLinkRequest - scheduler operation to process all incoming link 
 *  requests.
 *
 *    @return Status message from callee (currently not being used for frame 
 *            use).
 */
unsigned char FrameSchedulerLinkRequest(void)
{
  #if defined( PROTOCOL_ENDPOINT )
  // An End Point will receive a link request response if the link request was
  // approved.
  PhyAddressLinkEstablish(gFrameScheduler.frame.header.panId,
                          gFrameScheduler.frame.header.srcAddr);
  #elif defined( PROTOCOL_GATEWAY )
  if ((gFrameScheduler.frame.header.control & FRAME_CONTROL_MODE) == FRAME_CONTROL_MODE_ENDPOINT)
  {
    bool accept = true;     // Default behavior: accept all incoming requests
    
    // A Gateway node will verify the link request and send a link request 
    // response if the link request has been approved.
    if (gFrameScheduler.LinkRequest != NULL)
    {
      accept = gFrameScheduler.LinkRequest(gFrameScheduler.frame.payload, 
                                           gFrameScheduler.length);
    }
    
    if (accept)
    {
      // The link request has been accepted. Provide a response to the remote
      // node.
      PhyEnable();
      FrameSend(eFrameTypeLinkRequest, false, NULL, 0);
    }
  }
  #endif

  return 0;
}

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

// -----------------------------------------------------------------------------
// Frame configuration

#if defined( PROTOCOL_ENDPOINT )
void FrameInit(unsigned char(*FrameComplete)(unsigned char*, unsigned char))
#elif defined( PROTOCOL_GATEWAY )
void FrameInit(unsigned char(*FrameComplete)(bool, unsigned char*, unsigned char),
               bool(*LinkRequest)(unsigned char *payload, unsigned char length))
#endif
{
  gFrameScheduler.FrameComplete = FrameComplete;
  #if defined( PROTOCOL_GATEWAY )
  gFrameScheduler.LinkRequest = LinkRequest;
  #endif
  // By default, an End Point will be in low power mode and a Gateway will be
  // in listen mode.
  FrameIdle();
}

struct sFrame* FrameGetInfo()
{
  return &gFrameScheduler.frame;
}
                 
#if defined( PROTOCOL_GATEWAY )
void FrameSetDataResponse(unsigned char *payload, 
                          unsigned char length)
{
  gFrameScheduler.dataResponse.payload = payload;
  gFrameScheduler.dataResponse.length = length;
}
#endif

// -----------------------------------------------------------------------------
// Frame basic operations

void FrameIdle()
{
  #if defined( PROTOCOL_ENDPOINT )
  PhyLowPowerMode();
  #elif defined( PROTOCOL_GATEWAY )
  PhyDisable();
  FrameListen();
  #endif
}

bool FrameListen()
{
  if (!gFrameScheduler.busy)
  {
    gFrameScheduler.busy = true;
    PhyReceiverOn((unsigned char*)&gFrameScheduler.frame);
    
    return true;
  }
  
  return false;
}

bool FrameSend(enum eFrameType type,
               bool dataRequest,
               unsigned char *payload, 
               unsigned char length)
{
  if (!gFrameScheduler.busy)
  {
    // Build the frame.
    FrameBuild(type, dataRequest, payload, length);

    // Transmit the frame.
    if ((length + FRAME_OVERHEAD_LENGTH) <= PROTOCOL_DATASTREAM_MAX_SIZE)
    {
      if (PhyTransmit((unsigned char*)&gFrameScheduler.frame,
                      length + FRAME_OVERHEAD_LENGTH))
      {
        // The frame scheduler is only busy if the physical layer has accepted
        // to transmit the frame.
        gFrameScheduler.busy = true;
        return true;
      }
      else
      {
        // Error: physical layer was unable to perform the transmission.
        return false;
      }
    }
    else
    {
      // Error: Segmentation is not currently supported. Size of the frame is
      // too large.
      return false;
    }
  }
  
  // Error: the frame scheduler is currently busy.
  return false;
}

// -----------------------------------------------------------------------------
// Frame scheduling operations

bool FrameBusy()
{
  return gFrameScheduler.busy;
}

unsigned char FrameAssemble(unsigned char *payload, unsigned char length)
{            
  gFrameScheduler.busy = false;

  // Clear the size of the buffer for the next RX or TX payload.
  gFrameScheduler.length = 0;
  #if defined( PROTOCOL_GATEWAY )
  FrameSetDataResponse(NULL, 0);
  #endif
    
  // Is the received message at least the size of the frame overhead and is the
  // CRC valid?
  if (length >= FRAME_OVERHEAD_LENGTH 
      && (PhyGetDataStreamStatus()->status & PROTOCOL_DATASTREAM_FOOTER_CRC))
  {
    gFrameScheduler.length = length - FRAME_OVERHEAD_LENGTH;
    
    // Filter the incoming frame.
    #if defined( PROTOCOL_ENDPOINT)
    if (FrameEndPointValidate(gFrameScheduler.frame.header.panId,
                              gFrameScheduler.frame.header.destAddr))
    #elif defined( PROTOCOL_GATEWAY )
    if (FrameGatewayValidate(gFrameScheduler.frame.header.panId,
                             gFrameScheduler.frame.header.destAddr))  
    #endif
    {
      unsigned char statusMessage = 0;

      switch (gFrameScheduler.frame.header.control & FRAME_CONTROL_TYPE)
      {
      case eFrameTypeData:
        statusMessage = FrameSchedulerData();
        break;
      case eFrameTypeLinkRequest:
        statusMessage = FrameSchedulerLinkRequest();
        break;
      default:
        break;
      }
      
      // Check if the protocol is performing a data response to the last
      // incoming message before going into an IDLE state.
      if (!gFrameScheduler.busy)
      {
        FrameIdle();
      }
      
      return statusMessage;
    }
  }
  
  /**
   *  If an invalid frame has been received or an unknown error has occurred. Go
   *  back into a protocol IDLE state. The possible states include the: a frame 
   *  of invalid length was received or a frame with an invalid CRC was 
   *  received.
   */
  FrameIdle();
  
  return 0;
}

unsigned char FrameDisassemble()
{
  gFrameScheduler.busy = false;

  // Check if the data transfer requires a response.
  if (gFrameScheduler.frame.header.control & FRAME_CONTROL_DATA_REQ)
  {
    // Begin listening for a response (data or data + ACK). Filter on the 
    // destination address of the frame just sent.
    FrameListen();
    return 0;
  }
  else
  {
    // Check if the data transfer was a simplex.
    if (!(gFrameScheduler.frame.header.control & FRAME_CONTROL_DATA_REQ))
    {
      unsigned char statusMessage = 0;
      
      // Clear the size of the buffer for the next RX or TX payload.
      gFrameScheduler.length = 0;
    
      // Invoke the data complete callback.
      if (gFrameScheduler.FrameComplete != NULL)
      {
        #if defined( PROTOCOL_ENDPOINT )
        statusMessage = gFrameScheduler.FrameComplete(NULL, 0);
        #elif defined( PROTOCOL_GATEWAY )
        statusMessage = gFrameScheduler.FrameComplete(false, NULL, 0);
        #endif
      }
      
      FrameIdle();
      
      return statusMessage;
    }
  }

  FrameIdle();
  
  return 0;
}

unsigned char FrameTimeout()
{
  gFrameScheduler.busy = false;
  FrameIdle();
  return 0;
}

// -----------------------------------------------------------------------------
/**
 *  Test stub - test functionality of frame module.
 */

/**
 *  To test this module, define the following in your compiler preprocessor
 *  definitions: "TEST_FRAME".
 *
 *  It is strongly suggested that you leave the test stub in this source file.
 *  This stub will allow you to easily test your implementation using unit tests 
 *  defined and by adding more to suit your application needs.
 */
#ifdef TEST_FRAME

/**
 *  Test Example - test the functionality of the MAC frame sub layer.
 *
 *  @version    1.0.00
 *  @date       16 Oct 2012
 *  @author     BPB, air@anaren.com
 *  @platform   Texas Instruments MSP430 (MSP430G2553/LaunchPad Dev Kit)
 *  @compiler   IAR C/C++ Compiler for MSP430
 *              5.50.1 (5.50.1.50465)
 *
 *  assumptions
 *  ===========
 *  none
 *
 *  file dependency
 *  ===============
 *  intrinsics.h : processor intrinsic functions
 *  - This file also depend on the compiler that is listed above.
 *  msp430g2553.h : processor register definitions
 *
 *  revision history
 *  ================
 *  ver 1.0.00 : 16 Oct 2012
 *  - initial release
 */
// msp430 intrinsic functions (compiler specific).
#include "intrinsics.h"
// msp430g2553 peripheral and register definitions.
#include "msp430g2553.h"

// -----------------------------------------------------------------------------

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;
  
  while (true)
  {
    // TODO: Implement a test example for the frame module.
    __no_operation();
  }
}

#endif  /* TEST_FRAME */
