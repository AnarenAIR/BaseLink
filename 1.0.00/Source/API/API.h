#ifndef API_H
#define API_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
*  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  API.h - protocol Application Programming Interface (API).
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
 *  none
 *
 *  revision history
 *  ================
 *  ver 1.0.01 : 18 Oct 2012
 *  - updated internal documentation; comments revised
 *  ver 1.0.00 : 21 Sep 2012
 *  - initial release
 */
#define API_INFO "API 1.0.01"

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif
   
//------------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

#ifndef NULL
#define NULL  (void*)0
#endif

// Physical Status Information Masks
#define PROTOCOL_PHYSICAL_STATUS_CRC    (0x0080u)
#define PROTOCOL_PHYSICAL_STATUS_LQI    (0x007Fu)

#if defined( PROTOCOL_ENDPOINT )    
/**
 *  sProtocolSetupInfo - setup information used to initialize the protocol. For
 *  an End Point node, { channel, panId, address } are required parameters.
 */
struct sProtocolSetupInfo
{
  unsigned char channel[PROTOCOL_CHANNEL_LIST_SIZE];
  // Physical addressing setup
  unsigned char panId[PROTOCOL_PHYADDRESS_PANID_SIZE];
  unsigned char address[PROTOCOL_PHYADDRESS_ADDRESS_SIZE];
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
  bool(*Backup)(bool read, unsigned char *data, unsigned char size);
  /**
   *  TransferComplete - notification of a transfer complete event. Allows the
   *  application to read an incoming message and act on it.
   *
   *    @param  payload     Data being received.
   *    @param  length      Number of bytes in the payload.
   *
   *    @return Status message (application use only).
   */
  unsigned char(*TransferComplete)(unsigned char *payload, unsigned char length);
};
#elif defined( PROTOCOL_GATEWAY )
/**
 *  sProtocolSetupInfo - setup information used to initialize the protocol. For
 *  a Gateway node, { channel, panId, address } are required parameters.
 */
struct sProtocolSetupInfo
{
  unsigned char channel[PROTOCOL_CHANNEL_LIST_SIZE];
  unsigned char panId[PROTOCOL_PHYADDRESS_PANID_SIZE];
  unsigned char address[PROTOCOL_PHYADDRESS_ADDRESS_SIZE];
  /**
   *  LinkRequest - notification of a link request event. Allows the application 
   *  to read the optional request payload and accept/deny the request.
   *
   *    @param  payload   May contain an optional link request message.
   *    @param  length    Number of bytes in the payload.
   *
   *    @return Accept/deny of the request. If true, accept the request. 
   *    Otherwise, deny the request.
   */
  bool(*LinkRequest)(unsigned char *payload, unsigned char length);
  /**
   *  TransferComplete - notification of a transfer complete event. Allows the
   *  application to read an incoming message and act on it.
   *
   *    @param  dataRequest Data requested indicator.
   *    @param  payload     Data being received.
   *    @param  length      Number of bytes in the payload.
   *
   *    @return Status message (application use only).
   */
  unsigned char(*TransferComplete)(bool dataRequest, 
                                   unsigned char *payload, 
                                   unsigned char length);
};
#endif

/**
 *  sProtocolStatusInfo - protocol status information that is available for the
 *  application. This is broken down into the sub layers of the protocol.
 */
struct sProtocolStatusInfo
{
  /**
   *  sProtocolDataLinkInfo - Data Link layer status information that is
   *  available for the application. This is broken down into the sub modules
   *  of the Data Link layer.
   */
  struct sProtocolDataLinkInfo
  {
    /**
     *  sProtocolPhyAddressInfo - Data Link layer physical addressing 
     *  information that is available for the application.
     */
    struct sProtocolPhyAddressInfo
    {
      unsigned char panId[PROTOCOL_PHYADDRESS_PANID_SIZE];  // Personal Area Network (PAN) identifier
      #if defined( PROTOCOL_ENDPOINT )
      bool connected;                                       // Connection status
      #endif
    } phyAddressInfo;
    /**
     *  sProtocolFrameInfo - Data Link layer MAC frame information that is
     *  available for the application.
     */
    struct sProtocolFrameInfo
    {
      unsigned char srcAddr[PROTOCOL_PHYADDRESS_ADDRESS_SIZE];  // Source of the payload
      unsigned char seqNumber;                                  // Frame sequence number
    } frameInfo;
  } dataLink;
  
  /**
   *  sProtocolPhysicalInfo - Physical layer status information that is 
   *  available for the application.
   */
  struct sProtocolPhysicalInfo
  {
    /**
     *  sProtocolDataStreamInfo - data stream information associated with the
     *  last message. This information is updated after every receive occurs.
     */
    struct sProtocolDataStreamInfo
    {
      signed char rssi;             // Received signal strength indicator
      unsigned char status;         // Status [CRC(1):LQI(7)]
    } dataStreamInfo;
  } physical;
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
// Protocol configuration

/**
 *  ProtocolInit - initialize the protocol structures and available hardware
 *  (e.g. communication and timer peripheral).
 *
 *    @param  setup   Setup information required to properly initialize the
 *                    protocol.
 *
 *    @return Success of initializing the protocol.
 */
bool ProtocolInit(const struct sProtocolSetupInfo *setup);

// -----------------------------------------------------------------------------
// Protocol status information

/**
 *  ProtocolStatusPhyAddressInfo - get the Data Link layer physical addressing
 *  information for the last message.
 *
 *    @return Physical address information associated with the last message.
 */
struct sProtocolPhyAddressInfo ProtocolStatusPhyAddressInfo(void);

/**
 *  ProtocolStatusFrameInfo - get the Data Link layer MAC frame information for
 *  the last message.
 *
 *    @return Frame information associated with the last message.
 */
struct sProtocolFrameInfo ProtocolStatusFrameInfo(void);

/**
 *  ProtocolStatusPhysicalInfo - get the Physical layer status information for
 *  the last message.
 *
 *    @return Physical information associated with the last message.
 */
const struct sProtocolPhysicalInfo* ProtocolStatusPhysicalInfo(void);

/**
 *  ProtocolBusy - inidicates if the protocol is currently busy or ready for the
 *  next operation.
 *
 *    @return   If true, the protocol is busy performing an operation. If false,
 *              the protocol is ready.
 */
bool ProtocolBusy(void);

// -----------------------------------------------------------------------------
// Protocol operations

/**
 *  ProtocolConnect - attempts to connect the local node to a remote node.
 *
 *  Note: This function is only applicable to nodes that perform two-way 
 *  communication and may only be called on an End Point node.
 *
 *    @param  txData      Data to be transferred during the connection attempt.
 *    @param  txLength    Number of data bytes to transfer.
 *
 *    @return Success of the operation.
 */
bool ProtocolConnect(const unsigned char *txData, unsigned char length);

/**
 *  ProtocolDisconnect - disconnects local node from a remote node.
 *
 *  Note: This function is only applicable to nodes that perform two-way 
 *  communication and may only be called on an End Point node.
 */
void ProtocolDisconnect(void);

/**
 *  ProtocolSimpleTransfer - initiates a simple protocol data transfer. This
 *  function can be used as a "brute force" mechanism for transferring a 
 *  message; no connection is required.
 *
 *  Note: This function is only supported by End Point nodes!
 *
 *    @param  txData      Data to be transferred.
 *    @param  txLength    Number of data bytes to transfer.
 *
 *    @return Success of the operation.
 */
bool ProtocolSimpleTransfer(const unsigned char *txData, unsigned char length);

/**
 *  ProtocolTransfer - initiates a standard protocol data transfer.
 *
 *  Note: This function is only supported by End Point nodes!
 *
 *  Note: If an End Point is not connected to a Gateway node the provided data 
 *  will not be transferred until a connection exists. Be sure to ALWAYS call 
 *  Connect before attempting to use this function.
 *
 *    @param  txData      Data to be transferred.
 *    @param  txLength    Number of data bytes to transfer.
 *
 *    @return Success of the operation.
 */
bool ProtocolTransfer(const unsigned char *txData, unsigned char txLength);

/**
 *  ProtocolLoadDataResponse - loads a response to a data request into the 
 *  protocol transmission buffer. 
 *
 *  Note: This function is only supported by Gateway nodes!
 *
 *  Note: This function should be used in conjunction with data requests only!
 *  When a remote node requests data, load any new available data using this
 *  function.
 *
 *    @param  txData      Data to be transferred in a response message.
 *    @param  txLength    Number of data bytes to transfer.
 */
void ProtocolLoadDataResponse(unsigned char *txData,
                              unsigned char txLength);

// -----------------------------------------------------------------------------
// Protocol interrupt service routine operations

/**
 *  ProtocolEngine - main operations for the protocol.
 *
 *  Note: It is assumed that this function is called inside an interrupt service
 *  routine where global interrupts are DISABLED. The protocol engine will 
 *  reenable global interrupts when it is ready to do so.
 *  
 *    @param  event   
 *
 *    @return 
 */
unsigned char ProtocolEngine(volatile unsigned char event);

/**
 *  ProtocolEngineTick - main operations system tick.
 *
 *  Note: The Physical Bridge requires a 16-bit hardware timer with at least a 
 *  1ms tick rate. The tick rate should be calculated as 1ms + crystal error %.
 */
void ProtocolEngineTick(void);

#endif  /* API_H */
