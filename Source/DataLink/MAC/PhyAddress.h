#ifndef PHY_ADDRESS_H
#define PHY_ADDRESS_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  PhyAddress.h - Data Link layer physical addressing.
 *
 *  @version  1.0.00
 *  @date     17 Sep 2012
 *  @author   BPB, air@anaren.com
 *
 *  Physical addressing provides a filter mechanism for nodes in a network.
 *  
 *  assumptions
 *  ===========
 *  - "NULL" is not a valid argument for pointer parameters unless the function 
 *  definition explicitly states that it is.
 *
 *  file dependency
 *  ===============
 *  stdbool.h : defines the datatype "bool" which represents values "true" and
 *  "false"
 *  string.h : defines the functions "memcpy", "memcmp", and "memset" that are 
 *  used for copying, comparing, and setting addresses
 *
 *  revision history
 *  ================
 *  ver 1.0.00: 17 Sep 2012
 *  - initial release
 */
#define PHY_ADDRESS_INFO "PHY_ADDRESS 1.0.00"

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif
#include <string.h>

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

#ifndef NULL
#define NULL  (void*)0
#endif

// Broadcast address
#define PHY_ADDRESS_BROADCAST    0

// Physical address PAN identifier and address sizes (in bytes)
#define PHY_ADDRESS_PANID_SIZE   PROTOCOL_PHYADDRESS_PANID_SIZE
#define PHY_ADDRESS_ADDRESS_SIZE PROTOCOL_PHYADDRESS_ADDRESS_SIZE

// Backup read/write operation
#define PHY_ADDRESS_BACKUP_READ  true
#define PHY_ADDRESS_BACKUP_WRITE false

/**
 *  sPhyAddressInfo - addressing information required to create links between
 *  two (or many) nodes.
 */
struct sPhyAddressInfo
{
  /**
   *  sPhyAddressLocal - local node addressing information.
   */
  struct sPhyAddressLocal
  {
    #if defined( PROTOCOL_ENDPOINT )
    bool fixed; // Local PAN identifier fixed flag (not broadcast address indicator)
    #endif
    unsigned char panId[PHY_ADDRESS_PANID_SIZE];      // Personal Area Network (PAN) identifier
    unsigned char address[PHY_ADDRESS_ADDRESS_SIZE];  // Local unique physical address
    #if defined( PROTOCOL_ENDPOINT )
    // Note: The remote address corresponds to a Gateway node address. An End
    // Point may only communicate with one Gateway at any time.
    unsigned char remote[PHY_ADDRESS_ADDRESS_SIZE];   // Remote unique physical address
    #endif
  } local;

  /**
   *  Backup - store local node addressing information in a non-volatile backup. 
   *  This backup is to be used as a way of recovering the network if power were 
   *  to be completely lost on a node. Boot up should use this Backup as a way 
   *  of retrieving lost information (in RAM) from a non-volatile source such 
   *  as an EEPROM or Flash device.
   *
   *  For an End Point, the address list will be a size of 1 as it can only ever 
   *  be paired with a Gateway. A Gateway does not store link information of 
   *  other nodes and does not store any remote addresses.
   *
   *  Note: It is assumed that the required information is stored in the
   *  following order,
   *
   *      For End Point:
   *       -----------------------------------------------------------------
   *      |   fixed   |   pan id   |   local address   |   remote address   |
   *       -----------------------------------------------------------------
   *            1           n               m                     m
   *
   *    where the number defined below each field is in bytes, n is the PAN
   *    identifier size, and m is the address size (in bytes).
   *
   *    @param  read  Read/write control flag.
   *    @param  data  Local node address information.
   *    @param  size  Number of bytes.
   *
   *    @return Success of the operation.
   */
  #if defined( PROTOCOL_ENDPOINT )
  bool(*Backup)(bool read, unsigned char *data, unsigned char size);
  #endif
};

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

// -----------------------------------------------------------------------------
// Physical Addressing Configuration

/**
 *  PhyAddressInit - setup physical addressing and provide the local node with
 *  an address. A Gateway MUST BE assigned a unique Personal Area Network (PAN)
 *  identifier. An End Point may or may not be assigned a PAN identifier at
 *  initialization.
 *
 *  Note: It is assumed that the address provided is a unique identifier.
 *
 *    @param  panId   Local node Personal Area Network (PAN) identifier. This is
 *                    used to differentiate co-existing networks. This value
 *                    must be set to a value other than the broadcast address on
 *                    a Gateway. An End Point may or may not have a fixed PAN
 *                    identifier at startup so the field is optional (may
 *                    provide a broadcast address instead).
 *    @param  address Unique local node address.
 *    @param  Backup  Callback mechanism to backup addressing information in
 *                    non-volatile storage. The backup is only used on End Point
 *                    nodes.
 */
void PhyAddressInit(const unsigned char panId[PHY_ADDRESS_PANID_SIZE],
                    const unsigned char address[PHY_ADDRESS_ADDRESS_SIZE],
                    bool(*Backup)(bool read, unsigned char *data, unsigned char size));

// -----------------------------------------------------------------------------
// Physical addressing information

/**
 *  PhyAddressGetLocalInfo - get local node's addressing information.
 *
 *    @return Physical addressing information for the local node.
 */
const struct sPhyAddressLocal* PhyAddressGetLocalInfo(void);

/**
 *  PhyAddressIsBroadcast - perform a comparison of an address to the broadcast
 *  address to determine if they match.
 *
 *    @param  addr  Physical address to compare to the broadcast address.
 *    @param  size  Size of the address in bytes.
 *
 *    @return Success if the provided address matches the broadcast address.
 */
bool PhyAddressIsBroadcast(const unsigned char *addr, unsigned char size);

/**
 *  PhyAddressLinkExists - verify that a link has been established between the
 *  local node and a remote node. This should never succeed until a valid link
 *  is established using PhyAddressLinkEstablish().
 *
 *  Note: This function should only be used on an End Point node.
 *
 *    @return Success if the local node is linked to a remote node.
 */
bool PhyAddressLinkExists(void);

// -----------------------------------------------------------------------------
// Physical addressing utilities

/**
 *  PhyAddressCopy - utility function used to set the destination address with 
 *  the source address value. This operation can be performed on an entire
 *  address (PAN identifier and address) or on the individual components (PAN
 *  identifier or address). Essentially, this function performs a memcpy from
 *  one address to another.
 *
 *  Note: Implemented with the compiler-native memcpy library function.
 *
 *    @param  unsigned char*  dest  Address that will be updated by the source 
 *                                  value.
 *    @param  unsigned char*  src   Address that contains the value to update 
 *                                  the destination.
 *    @param  unsigned char   size  Number of bytes to copy.
 */
#define PhyAddressCopy(dest, src, size)  memcpy(dest, src, size)

/**
 *  PhyAddressCompare - utility function used to compare two addresses.
 *  Essentially this function performs a memcmp on two addresses.
 *
 *  Note: Implemented with the compiler implementation of memcmp. This function
 *  is part of the C standard library.
 *
 *    @param  unsigned char*  addr1   Address 1.
 *    @param  unsigned char*  addr2   Address 2.
 *    @param  unsigned char   size    Size of the addresses.
 *
 *    @return int   Value indicating the relationship between the content of the
 *                  addresses. A value of 0 indicates that the addresses are
 *                  equal. A value greater than 0 indicates that the first byte
 *                  that does not match has a greater value in address 1 than in
 *                  address 2. A value less than 0 indicates the opposite.
 */
#define PhyAddressCompare(addr1, addr2, size) memcmp(addr1, addr2, size)

// -----------------------------------------------------------------------------
// Physical addressing operations

/**
 *  PhyAddressLinkEstablish - create a link between the local and a remote node.
 *
 *  Note: This function should only be used on an End Point node.
 *
 *    @param  panId   Remote node PAN identifier.
 *    @param  remote  Remote node address.
 */
void PhyAddressLinkEstablish(unsigned char panId[PHY_ADDRESS_PANID_SIZE],
                             unsigned char remote[PHY_ADDRESS_ADDRESS_SIZE]);

/**
 *  PhyAddressLinkDestroy - remove a link between the local and a remote node.
 *
 *  Note: This function should only be used on an End Point node.
 */
void PhyAddressLinkDestroy(void);

#endif  /* PHY_ADDRESS_H */
