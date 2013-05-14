/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  API.c - protocol Application Programming Interface (API).
 *
 *  @version  1.0.01
 *  @date     18 Oct 2012
 *  @author   BPB, air@anaren.com
 *
 *  assumptions
 *  ===========
 *  - "NULL" is not a valid argument for pointer parameters unless the function 
 *  definition explicitly states that it is.
 *
 *  file dependency
 *  ===============
 *  API.h : 
 *  Frame.h :
 *  PhyAddress.h :
 *  PhyBridge.h : 
 *
 *  revision history
 *  ================
 *  ver 1.0.01 : 18 Oct 2012
 *  - updated internal documentation; comments revised
 *  ver 1.0.00 : 21 Sep 2012
 *  - initial release
 */
#include "API.h"
#include "Frame.h"
#include "PhyAddress.h"
#include "PhyBridge.h"

//------------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

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
// Protocol configuration

bool ProtocolInit(const struct sProtocolSetupInfo *setup)
{
  if (setup == NULL)
  {
    return false;
  }
  
  // Setup the Physical layer.
  PhyInit(FrameDisassemble, FrameAssemble);
  PhySetChannel(setup->channel[0]);
  PhyTimerInit(NULL);
  
	// Setup the Data Link layer.
  #if defined( PROTOCOL_ENDPOINT )
  #if defined( PROTOCOL_USE_RX_TIMEOUT )
  PhySyncTimerInit(FrameTimeout);
  #endif
  PhyAddressInit(setup->panId, setup->address, setup->Backup);
  FrameInit(setup->TransferComplete);
  #elif defined( PROTOCOL_GATEWAY )
  PhyAddressInit(setup->panId, setup->address, NULL);
  FrameInit(setup->TransferComplete, setup->LinkRequest);
  #endif
  
  PhyEnable();
  
  return true;
}

// -----------------------------------------------------------------------------
// Protocol status information

struct sProtocolPhyAddressInfo ProtocolStatusPhyAddressInfo()
{
  struct sProtocolPhyAddressInfo phyAddressInfo;
  
  memcpy(phyAddressInfo.panId, PhyAddressGetLocalInfo()->panId, PROTOCOL_PHYADDRESS_PANID_SIZE);
  #if defined( PROTOCOL_ENDPOINT)
  phyAddressInfo.connected = PhyAddressLinkExists();
  #endif
  
  return phyAddressInfo;
}

struct sProtocolFrameInfo ProtocolStatusFrameInfo()
{
  struct sProtocolFrameInfo frameInfo;
  
  memcpy(frameInfo.srcAddr, FrameGetInfo()->header.srcAddr, PROTOCOL_PHYADDRESS_ADDRESS_SIZE);
  frameInfo.seqNumber = FrameGetInfo()->header.seqNumber;
  
  return frameInfo;
}
  
const struct sProtocolPhysicalInfo* ProtocolStatusPhysicalInfo()
{
  return (struct sProtocolPhysicalInfo*)PhyGetDataStreamStatus();
}

bool ProtocolBusy()
{
  return FrameBusy();
}

// -----------------------------------------------------------------------------
// Protocol operations

#if defined( PROTOCOL_ENDPOINT )
bool ProtocolConnect(const unsigned char *txData, unsigned char length)
{
  if (!PhyAddressLinkExists())
  {
    FrameSend(eFrameTypeLinkRequest, true, (unsigned char*)txData, length);
    return false;
  }
  
  return true;
}
#endif

#if defined( PROTOCOL_ENDPOINT )
void ProtocolDisconnect()
{
  PhyAddressLinkDestroy();
}
#endif

#if defined( PROTOCOL_ENDPOINT )
bool ProtocolSimpleTransfer(const unsigned char *txData, unsigned char length)
{
  if (FrameSend(eFrameTypeData, false, (unsigned char*)txData, length))
  {
    return true;
  }
  return false;
}
#endif

#if defined( PROTOCOL_ENDPOINT )
bool ProtocolTransfer(const unsigned char *txData, unsigned char txLength)
{
  if (PhyAddressLinkExists())
  {
    if (FrameSend(eFrameTypeData, true, (unsigned char*)txData, txLength))
    {
      return true;
    }
  }
  
  return false;
}
#endif

#if defined( PROTOCOL_GATEWAY )
void ProtocolLoadDataResponse(unsigned char *txData,
                              unsigned char txLength)
{
  FrameSetDataResponse(txData, txLength);
}
#endif

// -----------------------------------------------------------------------------
// Protocol interrupt service routine operations

unsigned char ProtocolEngine(volatile unsigned char event)
{
  return PhySyncEopIsr(event);
}

void ProtocolEngineTick()
{
  PhyTimerIsr();
}
