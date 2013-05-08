/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  PhyAddress.c - Data Link layer physical addressing.
 *
 *  @version    1.0.00
 *  @date       17 Sep 2012
 *  @author     BPB, air@anaren.com
 *
 *  For details on the interface, please see PhyAddress.h.
 *
 *  assumptions
 *  ===========
 *  Same as PhyAddress.h assumptions
 *
 *  file dependency
 *  ===============
 *  - PhyAddress.h defines the sPhyAddressInfo structure as well as includes
 *  common functions and prototypes
 *  - PhyBridge.h provides physical layer access through a protocol-defined
 *  interface
 *
 *  revision history
 *  ================
 *  ver 1.0.00 : 17 Sep 2012
 *  - initial release
 */
#include "PhyAddress.h"
#include "PhyBridge.h"

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

// Local node addressing information
static struct sPhyAddressInfo gPhyAddressInfo;

// -----------------------------------------------------------------------------
/**
 *  Private interface
 */

/**
 *  PhyAddressToBroadcast - assigns the provided address buffer to the broadcast
 *  address. Essentially, this function performs a memset from one address to 
 *  another.
 *
 *  Note: Implemented with the compiler-native memset library function.
 *
 *    @param  unsigned char*  addr  Address buffer to convert to the broadcast
 *                                  address.
 *    @param  unsigned char   size  Size of the address in bytes.
 */
#define PhyAddressToBroadcast(addr, size)\
  memset(addr, PHY_ADDRESS_BROADCAST, size)

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

// -----------------------------------------------------------------------------
// Physical addressing configuration
    
void PhyAddressInit(const unsigned char panId[PHY_ADDRESS_PANID_SIZE],
                    const unsigned char address[PHY_ADDRESS_ADDRESS_SIZE],
                    bool(*Backup)(bool read, unsigned char *data, unsigned char size))
{
  #if defined( PROTOCOL_ENDPOINT )
  gPhyAddressInfo.local.fixed = false;
  
  // Check if PAN identifier is set to the broadcast address. If it isn't the
  // application is using fixed addressing (PAN identifier is known).
  if (!PhyAddressIsBroadcast(panId, PHY_ADDRESS_PANID_SIZE))
  {
    gPhyAddressInfo.local.fixed = true;
  }
  
  // Setup backup mechanism, if provided.
  gPhyAddressInfo.Backup = Backup;
  #endif
  
  // Set the local address and clear the size of the address list.
  PhyAddressCopy(gPhyAddressInfo.local.panId,
                 panId,
                 PHY_ADDRESS_PANID_SIZE);
  PhyAddressCopy(gPhyAddressInfo.local.address, 
                 address,
                 PHY_ADDRESS_ADDRESS_SIZE);
  
  #if defined( PROTOCOL_ENDPOINT )
  if (gPhyAddressInfo.local.fixed)
  {
  #endif  
    // Set device address for physical filtering. Physical filtering is based off
    // of the PAN identifier (lowest byte).
    PhyEnableAddressFilter(gPhyAddressInfo.local.panId[0]);
  #if defined( PROTOCOL_ENDPOINT )
  }
  else
  {
    // Disable hardware filtering until a link is established.
    PhyDisableAddressFilter();
  }
  #endif
}

// -----------------------------------------------------------------------------
// Physical addressing information

const struct sPhyAddressLocal* PhyAddressGetLocalInfo()
{
  return &gPhyAddressInfo.local;
}

bool PhyAddressIsBroadcast(const unsigned char *addr, unsigned char size)
{
  unsigned char i;
  
  for (i = 0; i < size; i++)
  {
    if (addr[i] != PHY_ADDRESS_BROADCAST)
    {
      return false;
    }
  }
  
  return true;
}

// Note: A link exists if the local node (an End Point) has a remote address
// (the Gateway).
#if defined( PROTOCOL_ENDPOINT )
bool PhyAddressLinkExists()
{
  // Check if a remote address exists in the local node addressing information.
  if (!PhyAddressIsBroadcast(gPhyAddressInfo.local.remote, 
                             PHY_ADDRESS_ADDRESS_SIZE))
  {
    return true;
  }
  
  return false;
}
#endif

// -----------------------------------------------------------------------------
// Physical addressing operations

void PhyAddressLinkEstablish(unsigned char panId[PHY_ADDRESS_PANID_SIZE],
                             unsigned char remote[PHY_ADDRESS_ADDRESS_SIZE])
{
  #if defined( PROTOCOL_ENDPOINT )
  if (!gPhyAddressInfo.local.fixed)
  {
    PhyAddressCopy(gPhyAddressInfo.local.panId, panId, PHY_ADDRESS_PANID_SIZE);
    
    // Set device address for physical filtering. Physical filtering is based off
    // of the PAN identifier (lowest byte).
    PhyEnableAddressFilter(gPhyAddressInfo.local.panId[0]);
  }
  PhyAddressCopy(gPhyAddressInfo.local.remote, remote, PHY_ADDRESS_ADDRESS_SIZE);

  // Store information in non-volatile backup.
  if (gPhyAddressInfo.Backup != NULL)
  {
    gPhyAddressInfo.Backup(PHY_ADDRESS_BACKUP_WRITE, 
                           (unsigned char*)&gPhyAddressInfo.local,
                           sizeof(gPhyAddressInfo.local) / sizeof(struct sPhyAddressInfo));
  }
  #endif
}

void PhyAddressLinkDestroy()
{
  #if defined( PROTOCOL_ENDPOINT )
  if (!gPhyAddressInfo.local.fixed)
  {
    // Set PAN identifier to broadcast address.
    PhyAddressToBroadcast(gPhyAddressInfo.local.panId, PHY_ADDRESS_PANID_SIZE);
    
    // Disable hardware filtering until a link is established.
    PhyDisableAddressFilter();
  }
  
  // Set remote address to broadcast address.
  PhyAddressToBroadcast(gPhyAddressInfo.local.remote, PHY_ADDRESS_ADDRESS_SIZE);

  // Store information in non-volatile backup.
  if (gPhyAddressInfo.Backup != NULL)
  {
    gPhyAddressInfo.Backup(PHY_ADDRESS_BACKUP_WRITE, 
                           (unsigned char*)&gPhyAddressInfo.local,
                           sizeof(gPhyAddressInfo.local) / sizeof(struct sPhyAddressInfo));
  }
  #endif
}

// -----------------------------------------------------------------------------
/**
 *  Test stub - test functionality of physical addressing.
 */

/**
 *  To test this module, define the following in your compiler preprocessor
 *  definitions: "TEST_PHY_ADDRESS".
 *
 *  It is strongly suggested that you leave the test stub in this source file.
 *  This stub will allow you to easily test your implementation using unit tests 
 *  defined and by adding more to suit your application needs.
 */
#ifdef TEST_PHY_ADDRESS

/**
 *  Test Example - test the functionality of the physical addressing.
 *
 *  @version    1.0.00
 *  @date       02 Oct 2012
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
 *  ver 1.0.00 : 02 Oct 2012
 *  - initial release
 */
// msp430 intrinsic functions (compiler specific).
#include "intrinsics.h"
// msp430g2553 peripheral and register definitions.
#include "msp430g2553.h"

// -----------------------------------------------------------------------------

// Empty backup
static struct sPhyAddressLocal gBackupStorage = {
  #if defined( PROTOCOL_ENDPOINT )
  // Fixed PAN identifier flag
  0xFF,
  #endif
  // Local PAN identifier
  { 0xFF, 0xFF },
  // Local address
  { 0xFF, 0xFF },
  #if defined( PROTOCOL_ENDPOINT )
  // Remote address
  { 0xFF, 0xFF }
  #endif
};

bool Backup(bool read, struct sPhyAddressLocal *local)
{
  if (read)
  {
    /**
     *  This backup routine emulates Flash as the non-volatile backup storage
     *  mechanism. Check for erased flash to determine if data is backed up. The
     *  PAN identifier should never be 0xFFFF.
     */
    if (gBackupStorage.panId[0] != 0xFF)
    {
      // Read in the information.
      *local = gBackupStorage;

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    unsigned int i;
    
    // Erase entire backup.
    for (i = 0; i < PHYADDRESS_PANID_SIZE; i++)
    {
      gBackupStorage.panId[i] = 0xFF;
    }
    for (i = 0; i < PHYADDRESS_ADDRESS_SIZE; i++)
    {
      gBackupStorage.address[i] = 0xFF;
    }
    
    // Write to the backup device.   
    gBackupStorage = *local;
  }
  
  return true;
}

struct sProtocolSetupInfo
{
  // Physical addressing setup
  unsigned char panId[PROTOCOL_PHYADDRESS_PANID_SIZE];
  unsigned char address[PROTOCOL_PHYADDRESS_ADDRESS_SIZE];
  bool(*Backup)(bool read, struct sPhyAddressLocal *local);
  #if defined( PROTOCOL_GATEWAY )
  bool(*LinkRequest)(unsigned char *payload, unsigned char length);
  #endif
};

const struct sProtocolSetupInfo gProtocolSetupInfo = {
  #if defined( PROTOCOL_ENDPOINT )
  { 0x00, 0x01 },
  { 0x00, 0x01 },
  #elif defined( PROTOCOL_GATEWAY )
  { 0x12, 0x34 },
  { 0xBB, 0xBB },
  #endif
  Backup,
  #if defined( PROTOCOL_GATEWAY )
  NULL
  #endif
};

void ProtocolInit(const struct sProtocolSetupInfo *setup)
{
  PhyInit(setup->panId[0], NULL, NULL);
  #if defined( PROTOCOL_ENDPOINT )
  PhyAddressInit(setup->panId, setup->address, setup->Backup);
  #elif defined( PROTOCOL_GATEWAY )
  PhyAddressInit(setup->panId, setup->address, setup->Backup, setup->LinkRequest);
  #endif
}

int main(void)
{ 
  unsigned char panId[2][PROTOCOL_PHYADDRESS_PANID_SIZE] = {
    { 0x12, 0x34 },
    { 0x00, 0x00 }
  };
  unsigned char remote[2][PROTOCOL_PHYADDRESS_ADDRESS_SIZE] = { 
    { 0xBB, 0xBB },
    { 0x00, 0x01 }
  };
  WDTCTL = WDTPW | WDTHOLD;
  ProtocolInit(&gProtocolSetupInfo);
  
  PhyAddressLinkEstablish(panId[0], remote[0]);
  #if defined( PROTOCOL_GATEWAY )
  PhyAddressSetFilter(remote[1]);
  #endif
  PhyAddressLinkDestroy();
  
  return 0;
}

#endif  /* TEST_PHYSICAL_ADDRESS */
