#ifndef PHY_BRIDGE_H
#define PHY_BRIDGE_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: EULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  PhyBridge.h - bridge between the Physical layer and the Data Link layer. 
 *  Provides the interface for the protocol and the implementation from the
 *  physical hardware.
 *
 *  @version  1.0.01
 *  @date     2 Jul 2012
 *  @author   BPB, air@anaren.com
 *
 *  This interface contains all the necessary physical hardware operations for
 *  the protocol to operate. If the functions in this interface are not
 *  implemented to their specification with the desired hardware, undefined
 *  behavior will occur.
 *
 *  assumptions
 *  ===========
 *  none
 *
 *  file dependency
 *  ===============
 *  stdbool.h : defines the datatype "bool" which represents values "true" and
 *  "false"
 *
 *  revision history
 *  ================
 *  ver 1.0.01 : 16 Oct 2012
 *  - updated internal documentation; comments revised
 *  ver 1.0.00 : 2 Jul 2012
 *  - initial release
 */
#define PHY_BRIDGE_INFO   "PHY_BRIDGE 1.0.01"

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#if defined( PROTOCOL_ENDPOINT ) && defined( PROTOCOL_GATEWAY )
#error "PhyBridge Error: End Point and Gateway roles cannot be defined at the same time."
#endif

// -----------------------------------------------------------------------------
/**
 *  Defines, enumerations, and structure definitions
 */

// Critical region
#define PROTOCOL_ENABLE_INTERRUPT()       MCU_ENABLE_INTERRUPT()
#define PROTOCOL_DISABLE_INTERRUPT()      MCU_DISABLE_INTERRUPT()
#define PROTOCOL_CRITICAL_SECTION(code)   MCU_CRITICAL_SECTION(code)

// Maximum data stream size
// Note: Currently set to the maximum physical hardware TX FIFO size.
#define PROTOCOL_DATASTREAM_MAX_SIZE      PHY_MAX_TXFIFO_SIZE

// Data stream overhead
#define PROTOCOL_DATASTREAM_HEADER_LENGTH 1
#define PROTOCOL_DATASTREAM_FOOTER_LENGTH 2   // Not sent over-the-air
#define PROTOCOL_DATASTREAM_OVERHEAD      PROTOCOL_DATASTREAM_HEADER_LENGTH + PROTOCOL_DATASTREAM_FOOTER_LENGTH
#define PROTOCOL_DATASTREAM_FOOTER_CRC    (0x0080u)
#define PROTOCOL_DATASTREAM_FOOTER_LQI    (0x007Fu)
   
/**
 *  tPower - absolute power (dBm) representation.
 */
typedef signed int tPower;

/**
 *  tTime - timer tick representation.
 */
typedef unsigned long tTime;

/**
 *  sPhyDevice - 
 */
struct sPhyDevice
{
  
  void *phyInfo;                      // Physical information
  
  /**
   *  sPhyDeviceStatus - 
   */
  struct sPhyDeviceStatus
  {
    volatile bool transmitting;       // Device transmitting flag
    
    /**
     *  DataStreamSent - 
     *
     *    @return
     */
    unsigned char(*DataStreamSent)(void);
    
    /**
     *  DataStreamAvailable - 
     *
     *    @param  dataField
     *    @param  length
     *
     *    @return
     */
    unsigned char(*DataStreamAvailable)(unsigned char *dataField, 
                                        unsigned char length);
  } status;
  
  /**
   *  sPhyTimer - 
   */
  struct sPhyTimer
  {
    bool running;                         // Hardware timer running flag
    
    /**
     *  Generic - 
     *
     *    @return
     */
    unsigned char(*Generic)(void);        // Generic timer function
    
    #if defined( PROTOCOL_ENDPOINT ) && defined( PROTOCOL_USE_RX_TIMEOUT )
    /**
     *  sPhyTimerRxTimeout - 
     */
    struct sPhyTimerRxTimeout
    {
      bool enable;                        // Enable Rx timeout
      tTime compare;                      // Rx timeout compare value
      tTime counter;                      // Rx timeout counter
      
      /**
       *  SyncTimeout - 
       *
       *    @return
       */
      unsigned char(*RxTimeout)(void);    // Rx timeout callback
    } rxTimeout;
    #endif
  } timer;
  
  /**
   *  sPhyDataStream - physical layer encapsulation (data stream).
   *
   *  The following displays the Physical data stream structure,
   *  
   *           ------------------------------------------
   *          | Length | Address + Data Field | (Status) |
   *           ------------------------------------------
   *    Bytes:    1               n                2
   *
   *    Field Information:
   *      Length      Length of the entire data stream being transmitted. 
   *      Data Field  Address and message being encapsulated in the data stream. 
   *                  The first byte is used for address filtering.
   *      Status      On reception, data stream status is appended. This 
   *                  information includes RSSI, LQI, and CRC_OK. This information
   *                  is not sent over-the-air.
   *
   *  Note: It is assumed that the first byte in the data stream data field is a
   *  value that can be used for address filtering.
   */
  struct sPhyDataStream
  {
    /**
     *  sPhyDataStreamHeader - 
     */
    struct sPhyDataStreamHeader
    {
      unsigned char length;       // Length of the data stream
    } header;
    unsigned char *dataField;     // Address + data field (payload)
    /**
     *  sPhyDataStreamFooter - 
     */
    struct sPhyDataStreamFooter
    {
      signed char rssi;           // Last received data stream RSSI (raw value)
//      unsigned char lqi : 7;      // Last received data stream LQI
//      bool crc          : 1;      // Last received data stream CRC
      unsigned char status;      // Last received data stream [RSSI][LQI(7) + CRC(1)]
    } footer;
  } stream;
  
};

// -----------------------------------------------------------------------------
/**
 *  Global data
 */

// -----------------------------------------------------------------------------
/**
 *  Public interface
 */

/**
 *  PhyInit - initialize the Physical layer. Setup structures to initial values,
 *  assign callback functions, and configure the physical hardware.
 *
 *  Note: If either callback is not desired, they may be set to NULL. The
 *  implementation must check for NULL callback function pointers.
 *
 *    @param  DataStreamSent      Callback invoked when a data stream has been
 *                                sent (EOP interrupt issued during TX).
 *    @param  DataStreamAvailable Callback invoked when a data stream has been
 *                                received (EOP interrupt issued during RX).
 *
 *    @return Success of physical hardware and bridge setup. Will return true
 *            if initialization is successful, false otherwise.
 */
bool PhyInit(unsigned char(*DataStreamSent)(void), 
             unsigned char(*DataStreamAvailable)(unsigned char *dataField, unsigned char length));

/**
 *  PhyEnable - enable communication at the Physical hardware.
 */
void PhyEnable(void);

/**
 *  PhyDisable - disable communication at the Physical hardware.
 */
void PhyDisable(void);

// -----------------------------------------------------------------------------
// Physical configuration

/**
 *  PhyConfigure - configure the physical hardware.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  config  Index of the configuration desired from a lookup table.
 *
 *    @return Success of physical hardware configuration. Please refer to the
 *            associated hardware driver for more information on potential
 *            failure conditions.
 */
bool PhyConfigure(unsigned char config);

/**
 *  PhyEnableAddressFilter - set the hardware device address to filter on and enable
 *  hardware address filtering.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  deviceAddr  Desired physical device hardware address.
 */
void PhyEnableAddressFilter(unsigned char deviceAddr);

/**
 *  PhyDisableAddressFilter - disables hardware address filtering.
 */
void PhyDisableAddressFilter(void);

/**
 *  PhySetChannel - set the physical hardware communication channel.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  channel Index of the channel desired from a lookup table.
 *
 *    @return Success of physical hardware channel change. Please refer to the
 *            associated hardware driver for more information on potential
 *            failure conditions.
 */
bool PhySetChannel(unsigned char channel);

/**
 *  PhySetOutputPower - set the physical hardware transmitter's output power.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  power Index of the power level desired from a lookup table.
 */
void PhySetOutputPower(tPower power);

// -----------------------------------------------------------------------------
// Physical status

/**
 *  PhyGetInstantSignalStrength - get instantaneous signal strength from the
 *  surrounding environment.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @return Sampled absolute power level.
 */
tPower PhyGetInstantSignalStrength(void);

/**
 *  PhyGetDataStreamStatus - retrieve the last received data stream's status
 *  information located in the data stream footer. This information includes
 *  the received signal strength indicator, link quality indicator, and CRC.
 *
 *    @return Location of the data stream's status information.
 */
struct sPhyDataStreamFooter* PhyGetDataStreamStatus(void);

// -----------------------------------------------------------------------------
// Physical operation

/**
 *  PhyIdle - put the Physical hardware into an idle state.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 */
void PhyIdle(void);

/**
 *  PhyCalibrate - calibrate the Physical hardware.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 */
void PhyCalibrate(void);

/**
 *  PhyReceiverOn - turn on the Physical receiver.
 *
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  dataField   Buffer to store the received data field. This buffer 
 *                        is assumed to be large enough to store the largest
 *                        expected data field.
 */
void PhyReceiverOn(unsigned char *dataField);

/**
 *  PhyTransmit -  Build a data stream from the data field provided and transmit 
 *  the resulting data stream using the Physical hardware.
 * 
 *  Note: The implementation of this function should automatically handle 
 *  transitioning Physical hardware from a low power state to an active state
 *  before performing the operation.
 *
 *    @param  dataField   Buffer that stores the data field to be encapsulated
 *                        into a data stream.
 *    @param  count       Number of bytes in the data field buffer.
 *
 *    @return Success of the operation. If a device is already performing a
 *            transmission, this operation will fail until the initial transmit
 *            completes.
 */
bool PhyTransmit(unsigned char *dataField, 
                 unsigned char count);

/**
 *  PhyLowPowerMode - put the Physical hardware into a low power state.
 */
void PhyLowPowerMode(void);

// -----------------------------------------------------------------------------
// Physical timer

/**
 *  PhyTimerInit - initialize the physical timer. Setup callback function
 *  pointers to provide the Data Link layer with a millisecond tick.
 *
 *    @param  SyncTimeout   Callback invoked when a SYNC timeout occurs with in
 *                          RX. This is used strictly for power savings as the
 *                          callback may then issue the Physical hardware to go
 *                          to sleep.
 *    @param  GenericTimer  Callback invoked while the Physical timer is running
 *                          to provide a millisecond tick to other Data Link
 *                          layer modules.
 */
void PhyTimerInit(unsigned char(*GenericTimer)(void));

/**
 *  PhyTimerStart - start the physical timer. 
 */
void PhyTimerStart(void);

/**
 *  PhyTimerStop - stop the physical timer.
 */
void PhyTimerStop(void);

/**
 *  PhySyncTimerInit - initialize the SYNC timeout timer.
 *  
 *    @param  SyncTimeout Timeout callback issued when the SYNC timeout timer
 *                        expires.
 */
void PhySyncTimerInit(unsigned char(*SyncTimeout)(void));

// -----------------------------------------------------------------------------
// Physical interrupt service routines

/**
 *  PhySyncEopIsr - SYNC/End-Of-Packet (EOP) interrupt service routine.
 *  Performs necessary polarity changes to interrupt on both sync and EOP.
 *
 *  On reception, a data stream available flag will be set when the EOP is
 *  received. Physical header/footer information is then stripped off and a
 *  DataStreamAvailable callback is invoked.
 *
 *  On transmission and EOP interrupt is set, a DataStreamSent callback is 
 *  invoked.
 *
 *    @param  event   SYNC/EOP event indicator. This is usually in the form of
 *                    a hardware peripheral flag indicating that the associated
 *                    hardware has been interrupted.
 * 
 *    @return Status message from callee (currently not being used for physical
 *            bridge use).
 */
unsigned char PhySyncEopIsr(volatile unsigned char event);

/**
 *  PhyTimerIsr - physical timer interrupt service routine.
 *
 *  Note: The Physical Bridge requires a 16-bit hardware timer with at least a 
 *  1ms tick rate. The tick rate should be calculated as 1ms + crystal error %.
 * 
 *    @return Status message from callee (currently not being used for physical
 *            bridge use).
 */
unsigned char PhyTimerIsr(void);

#endif  /* PHY_BRIDGE_H */
