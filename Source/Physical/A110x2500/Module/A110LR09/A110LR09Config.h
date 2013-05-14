#ifndef A110LR09_CONFIG_H
#define A110LR09_CONFIG_H
/**
 *  ----------------------------------------------------------------------------
 *  Copyright (c) 2012-13, Anaren Microwave, Inc.
 *
 *  For more information on licensing, please see Anaren Microwave, Inc's
 *  end user software licensing agreement: A110x2500ModuleDriverEULA.txt.
 *
 *  ----------------------------------------------------------------------------
 *
 *  A110LR09Config.h - configuration file for the A110LR09 module interface. The
 *	registers that are user-modifiable are listed below. These registers
 *	correspond to fields that are user-modifiable in the module User's Manual.
 *
 *	Note: The configuration registers below may be masked off to change only
 *	values (fields) that are allowed to be changed. Please refer to the module
 *	User's Manual for information on which register fields may be changed.
 *
 *  Note: This file must exist for the A110LR09 module interface to compile.
 *  However, the settings definitions may be defined in a preincluded file 
 *  instead of in this file. This is dependent on what the application is 
 *	trying to accomplish.
 */

// Configuration user settings
#define A110LR09_USER_IOCFG2    0x2E  // GDO2 output pin configuration
#define A110LR09_USER_IOCFG1    0x2E  // GDO1 output pin configuration
#define A110LR09_USER_IOCFG0    0x06  // GDO0 output pin configuration
#define A110LR09_USER_FIFOTHR   0x87  // RX FIFO and TX FIFO thresholds
#define A110LR09_USER_PKTLEN    0x3D  // packet length
#define A110LR09_USER_PKTCTRL1  0x67  // packet automation control 1
#define A110LR09_USER_PKTCTRL0  0x45  // packet automation control 0
#define A110LR09_USER_ADDR      0x00  // device address
#define A110LR09_USER_CHANNR    0x00  // channel number
#define A110LR09_USER_FSCTRL0   0x00  // frequency synthesizer control 0
#define A110LR09_USER_MDMCFG2   0x73  // modem configuration 2
#define A110LR09_USER_MCSM2     0x07  // main radio control state machine configuration 2
#define A110LR09_USER_MCSM1     0x30  // main radio control state machine configuration 1
#define A110LR09_USER_MCSM0     0x18  // main radio control state machine configuration 0
#define A110LR09_USER_AGCCTRL1  0x00  // AGC control 1
#define A110LR09_USER_FSCAL3    0xEA  // frequency synthesizer calibration 3

#endif /* A110LR09_CONFIG_H */
