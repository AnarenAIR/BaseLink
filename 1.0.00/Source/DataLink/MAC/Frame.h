#ifndef FRAME_H
#define FRAME_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  Frame.h - Data Link layer Media Access Control (MAC) framing and scheduling 
 *  sub layer.
 *
 *  @version    1.0.01
 *  @date       16 Oct 2012
 *  @author     BPB, air@anaren.com
 *
 *  This module defines the structure of a frame and a scheduler for the Data
 *  Link layer Media Access Control (MAC) sub layer.
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
 *  ver 1.0.01 : 16 Oct 2012
 *  - updated internal documentation; comments revised
 *  ver 1.0.00 : 17 Sep 2012
 *  - initial release
 */
#define FRAME_INFO "FRAME 1.0.01"

#include "PhyBridge.h"
#include "PhyAddress.h"

#if defined( PROTOCOL_ENDPOINT ) && defined( PROTOCOL_GATEWAY )
#error "Frame Error: End Point and Gateway roles cannot be defined at the same time."
#endif

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

#ifndef NULL
#define NULL (void*)0
#endif

#define FRAME_HEADER_ADDRESS_LENGTH PROTOCOL_PHYADDRESS_PANID_SIZE + (2 * PROTOCOL_PHYADDRESS_ADDRESS_SIZE)
#define FRAME_HEADER_LENGTH         FRAME_HEADER_ADDRESS_LENGTH + 2
#define FRAME_FOOTER_LENGTH         0
#define FRAME_OVERHEAD_LENGTH       (FRAME_HEADER_LENGTH + FRAME_FOOTER_LENGTH)

#define FRAME_CONTROL_TYPE_MASK         0xC0u
#define FRAME_CONTROL_SECURE_MASK       0x20u
#define FRAME_CONTROL_PENDING_MASK      0x10u
#define FRAME_CONTROL_ACKREQUEST_MASK   0x08u
#define FRAME_CONTROL_ACK_MASK          0x04u
#define FRAME_CONTROL_DATAREQUEST_MASK  0x02u
#define FRAME_CONTROL_MODE_MASK         0x01u

// Node roles
#define FRAME_CONTROL_MODE_ENDPOINT     0x00u
#define FRAME_CONTROL_MODE_GATEWAY      0x01u

/**
 *  eFrameType - type indicating the intent of a frame to the scheduler. A Data
 *  frame type contains a message. A Link Request frame type may contain an 
 *  optional message as well as the intent to complete a link.
 */
enum eFrameType
{
  eFrameTypeData        = 0x00u,        // Data frame
  eFrameTypeLinkRequest = 0x40u         // Link request frame
};

#define FRAME_CONTROL_TYPE              0xC0u
#define FRAME_CONTROL_SECURE            0x20u
#define FRAME_CONTROL_PENDING           0x10u
#define FRAME_CONTROL_ACK_REQ           0x08u
#define FRAME_CONTROL_ACK               0x04u
#define FRAME_CONTROL_DATA_REQ          0x02u
#define FRAME_CONTROL_MODE              0x01u

/**
 *  sFrame - represents a Data Link layer frame.
 *
 *  The following displays the Physical data stream structure,
 *  
 *           --------------------------------------------------------------
 *          | PAN ID | Destination | Source | Control | Sequence | Payload |
 *           --------------------------------------------------------------
 *    Bytes:    n           n           n        1        1           n
 *
 *    Field Information:
 *      PAN ID      Personal Area Network (PAN) identifier.
 *      Destination Frame destination address.
 *      Source      Frame source address.
 *      Control     Frame control information. This includes type, security,
 *                  segmentation, acknowledgements, and data request. This
 *                  information is used by the scheduler.
 *      Sequence    Sequence number.
 *      Payload     Message being encapsulated in the frame.
 *  
 *  Note: The PAN ID, Destination, Source, and Payload sizes are configurable.
 */
struct sFrame
{
  /**
   *  sFrameHeader - header information definition.
   */
  struct sFrameHeader
  {
    unsigned char panId[PROTOCOL_PHYADDRESS_PANID_SIZE];
    unsigned char destAddr[PROTOCOL_PHYADDRESS_ADDRESS_SIZE]; // Destination for the payload
    unsigned char srcAddr[PROTOCOL_PHYADDRESS_ADDRESS_SIZE];  // Source of the payload
    /**
     *  sFrameControl - control information used by the scheduler to determine
     *  different actions that need to be made for incoming and outgoing frames.
     */
//    struct sFrameControl
//    {
//      enum eFrameType type  : 2;    // Type of frame message
//      bool secure           : 1;    // Security-enabled frame
//      bool pending          : 1;    // Frame is pending
//      bool ackRequest       : 1;    // Acknowledgement request
//      bool ack              : 1;    // Acknowledgement
//      bool dataRequest      : 1;    // Data request (0:simplex, 1:half duplex)
//      bool mode             : 1;    // Mode of the local node (0:End Point, 1:Gateway)
//    } control;
    unsigned char control;          // Control information
    unsigned char seqNumber;        // Frame sequence number
  } header;
  unsigned char payload[PROTOCOL_FRAME_MAX_PAYLOAD_LENGTH]; // Frame payload buffer
};

/**
 *  sFrameDataResponse - response to a data request.
 *
 *  Note: This should only be used by a Gateway node to provide a response to
 *  an End Point data request.
 */
struct sFrameDataResponse
{
  unsigned char *payload;   // Location of the response buffer
  unsigned char length;     // Number of bytes in the response
};

/**
 *  sFrameScheduler - Media Access Control (MAC) scheduler information.
 */
struct sFrameScheduler
{
  /**
   *  FrameComplete - the frame scheduler has entered a receive completion state 
   *  for the current operation (e.g. a frame has been validated and received).
   *
   *  Note: Data requests are sent from an End Point node to a Gateway node.
   *  Therefore, the End Point FrameComplete callback does not need a data 
   *  request indicator; a Gateway node CANNOT request data!
   *
   *    @param  [dataRequest] Data request indicator.
   *    @param  payload       Location of the frame payload buffer. Contains 
   *                          data for the callee.
   *    @param  length        Number of bytes in the payload.
   *
   *    @return Status message from callee (currently not being used for frame 
   *            use).
   */
  #if defined( PROTOCOL_ENDPOINT )
  unsigned char(*FrameComplete)(unsigned char *payload, 
                                unsigned char length);
  #elif defined( PROTOCOL_GATEWAY )
  unsigned char(*FrameComplete)(bool dataRequest, 
                                unsigned char *payload, 
                                unsigned char length);
  
  /**
   *  LinkRequest - notification of a link request or link request status.
   *
   *  For the Gateway, this callback will allow the application to read the
   *  request's payload and accept or deny the request. 
   *
   *  Note: This callback does not exist for an End Point.
   *
   *    @param  payload   Location of the frame payload buffer. Contains a link
   *                      request message for the callee.
   *    @param  length    Number of bytes in the payload.
   *
   *    @return Status message from callee (currently not being used for frame
   *            use).
   */
  bool(*LinkRequest)(unsigned char *payload, unsigned char length);
  
  struct sFrameDataResponse dataResponse; // Data request response information
  #endif
  
  // --------------------------------------------------------------------------
  // Frame scheduler resources
  
  volatile bool busy;             // Frame scheduler busy flag
  volatile bool txBusy;           // Frame transmit busy flag
  volatile bool rxBusy;           // Frame receive busy flag
  struct sFrame frame;            // Frame for RX/TX
  unsigned char length;           // Frame length in bytes
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
// Frame configuration

/**
 *  FrameInit - initialize the frame scheduler.
 *
 *    @param  FrameComplete Callback which is invoked when a frame scheduler 
 *                          receive operation is complete. It is used as a
 *                          method of providing the callee any received data.
 *    @param  [LinkRequest] Callback invoked when a link request frame is
 *                          received and validated on a Gateway. It is used as
 *                          a method of providing the callee any received link
 *                          data and a method of accepting/denying the request.
 */
#if defined( PROTOCOL_ENDPOINT )
void FrameInit(unsigned char(*FrameComplete)(unsigned char*, unsigned char));
#elif defined( PROTOCOL_GATEWAY )
void FrameInit(unsigned char(*FrameComplete)(bool, unsigned char*, unsigned char),
               bool(*LinkRequest)(unsigned char *payload, unsigned char length));
#endif

/**
 *  FrameGetInfo - 
 */
struct sFrame* FrameGetInfo(void);

/**
 *  FrameSetDataResponse - setup the data response structure with a payload
 *  buffer and number of bytes available in the buffer.
 * 
 *  Note: This is used by the Gateway role to provide a response to potential
 *  data requests originating from End Point nodes.
 *
 *    @param  payload Location of the frame payload buffer. Contains the data
 *                    response for the caller.
 *    @param  length  Number of bytes in the payload.
 */
void FrameSetDataResponse(unsigned char *payload, 
                          unsigned char length);   

// -----------------------------------------------------------------------------
// Frame basic operations

/**
 *  FrameIdle - perform the scheduler's idle operation. For an End Endpoint, go 
 *  to a low power state. For a Gateway, turn on the receiver.
 */
void FrameIdle(void);

/**
 *  FrameListen - listen for an incoming frame.
 *
 *    @return Frame scheduler busy status (operation was successful if true,
 *            else try again at a later time).
 */
bool FrameListen(void);

/**
 *  FrameSend - send a frame to the destination. Use FrameBuild(...) to 
 *  construct the frame.
 *
 *    @param  type      Type of frame being constructed.
 *    @param  payload   Buffer holding the frame payload.
 *    @param  length    Number of payload bytes.
 *
 *    @return Frame scheduler busy status (operation was successful if true,
 *            else try again at a later time).
 */
bool FrameSend(enum eFrameType type,
               bool dataRequest,
               unsigned char *payload, 
               unsigned char length);

// -----------------------------------------------------------------------------
// Frame scheduling operations

/**
 *  FrameBusy - 
 */
bool FrameBusy(void);

/**
 *  FrameAssemble - assemble the incoming data streams into a complete frame.
 *  Once a complete frame is created, send a notification to the layer above.
 *
 *  Note: It is assumed that this function is called from inside a critical
 *  region. Physical device interrupts should be disabled when entering/exiting 
 *  this function.
 *
 *    @param  dataField Buffer containing the received data field (frame).
 *    @param  length    Length of the data field.
 *
 *    @return Status message from caller (currently not being used for frame
 *            use).
 */
unsigned char FrameAssemble(unsigned char *dataField, unsigned char length);

/**
 *  FrameDisassemble - disassemble the outgoing frame into data streams. Once
 *  the segmented frame has been completely transmitted, send a notification to
 *  the layer above.
 *
 *  Note: It is assumed that this function is called from inside a critical
 *  region. Physical device interrupts should be disabled when entering/exiting 
 *  this function.
 *
 *    @return Status message from caller (currently not being used for frame
 *            use).
 */
unsigned char FrameDisassemble(void);

/**
 *  FrameTimeout - callback that is invoked when an End Point operation times
 *  out.
 *
 *  Note: This callback only occurs on an End Point.
 *
 *    @return Status message from caller (currently not being used for frame
 *            use).
 */
unsigned char FrameTimeout(void);

#endif  /* FRAME_H */
