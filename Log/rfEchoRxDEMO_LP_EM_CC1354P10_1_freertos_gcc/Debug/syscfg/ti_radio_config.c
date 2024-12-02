/*
 *  ======== ti_radio_config.c ========
 *  Configured RadioConfig module definitions
 *
 *  DO NOT EDIT - This file is generated for the CC1354P10RSK
 *  by the SysConfig tool.
 *
 *  Radio Config module version : 1.18.0
 *  SmartRF Studio data version : 2.31.0
 */

#include "ti_radio_config.h"
#include DeviceFamily_constructPath(rf_patches/rf_patch_cpe_multi_protocol.h)


// *********************************************************************************
//   RF Frontend configuration
// *********************************************************************************
// RF design based on: LP_EM_CC1354P10_1

// TX Power tables
// The RF_TxPowerTable_DEFAULT_PA_ENTRY and RF_TxPowerTable_HIGH_PA_ENTRY macros are defined in RF.h.
// The following arguments are required:
// RF_TxPowerTable_DEFAULT_PA_ENTRY(bias, gain, boost, coefficient)
// RF_TxPowerTable_HIGH_PA_ENTRY(bias, ibboost, boost, coefficient, ldoTrim)
// See the Technical Reference Manual for further details about the "txPower" Command field.
// The PA settings require the CCFG_FORCE_VDDR_HH = 0 unless stated otherwise.

// 868 MHz, 13 dBm
RF_TxPowerTable_Entry txPowerTable_868_pa13[TXPOWERTABLE_868_PA13_SIZE] =
{
    {-20, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(21, 3, 0, 0, 3) }, // 0x0300D5
    {-19, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(24, 3, 0, 0, 3) }, // 0x0300D8
    {-18, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(27, 3, 0, 4, 3) }, // 0x0308DB
    {-17, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(31, 3, 0, 4, 3) }, // 0x0308DF
    {-16, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(15, 3, 0, 4, 2) }, // 0x0208CF
    {-15, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(17, 3, 0, 4, 2) }, // 0x0208D1
    {-14, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(47, 3, 0, 4, 3) }, // 0x0308EF
    {-13, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(22, 3, 0, 7, 2) }, // 0x020ED6
    {-12, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(25, 3, 0, 7, 2) }, // 0x020ED9
    {-11, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(29, 3, 0, 7, 2) }, // 0x020EDD
    {-10, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(33, 3, 0, 11, 2) }, // 0x0216E1
    {-9, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(39, 3, 0, 11, 2) }, // 0x0216E7
    {-8, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(56, 2, 0, 7, 3) }, // 0x030EB8
    {-7, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(19, 3, 0, 14, 1) }, // 0x011CD3
    {-6, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(28, 2, 0, 14, 2) }, // 0x021C9C
    {-5, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(33, 2, 0, 14, 2) }, // 0x021CA1
    {-4, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(29, 3, 0, 21, 1) }, // 0x012ADD
    {-3, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(35, 3, 0, 25, 1) }, // 0x0132E3
    {-2, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(54, 2, 0, 21, 2) }, // 0x022AB6
    {-1, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(50, 3, 0, 35, 1) }, // 0x0146F2
    {0, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(23, 2, 0, 25, 1) }, // 0x013297
    {1, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(27, 2, 0, 28, 1) }, // 0x01389B
    {2, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(37, 1, 0, 35, 2) }, // 0x024665
    {3, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(45, 1, 0, 39, 2) }, // 0x024E6D
    {4, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(49, 2, 0, 51, 1) }, // 0x0166B1
    {5, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(17, 1, 0, 28, 1) }, // 0x013851
    {6, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(22, 1, 0, 35, 1) }, // 0x014656
    {7, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(28, 1, 0, 42, 1) }, // 0x01545C
    {8, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(37, 1, 0, 56, 1) }, // 0x017065
    {9, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(35, 2, 0, 60, 0) }, // 0x0078A3
    {10, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(14, 1, 0, 32, 0) }, // 0x00404E
    {11, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(31, 1, 0, 74, 0) }, // 0x00945F
    {12, RF_TxPowerTable_CC13x4Sub1GHz_DEFAULT_PA_ENTRY(51, 0, 0, 95, 0) }, // 0x00BE33
    RF_TxPowerTable_TERMINATION_ENTRY
};

// 868 MHz, 20 dBm
RF_TxPowerTable_Entry txPowerTable_868_pa20[TXPOWERTABLE_868_PA20_SIZE] =
{
    {14, RF_TxPowerTable_HIGH_PA_ENTRY(13, 0, 0, 28, 0) }, // 0x00380D
    {15, RF_TxPowerTable_HIGH_PA_ENTRY(18, 0, 0, 36, 0) }, // 0x004812
    {16, RF_TxPowerTable_HIGH_PA_ENTRY(24, 0, 0, 43, 0) }, // 0x005618
    {17, RF_TxPowerTable_HIGH_PA_ENTRY(28, 0, 0, 51, 2) }, // 0x02661C
    {18, RF_TxPowerTable_HIGH_PA_ENTRY(34, 0, 0, 64, 4) }, // 0x048022
    {19, RF_TxPowerTable_HIGH_PA_ENTRY(15, 3, 0, 36, 4) }, // 0x0448CF
    {20, RF_TxPowerTable_HIGH_PA_ENTRY(18, 3, 0, 71, 27) }, // 0x1B8ED2
    RF_TxPowerTable_TERMINATION_ENTRY
};


// 2400 MHz, 5 dBm
RF_TxPowerTable_Entry txPowerTable_2400_pa5[TXPOWERTABLE_2400_PA5_SIZE] =
{
    {-20, RF_TxPowerTable_DEFAULT_PA_ENTRY(7, 3, 0, 0) }, // 0x00C7
    {-18, RF_TxPowerTable_DEFAULT_PA_ENTRY(9, 3, 0, 0) }, // 0x00C9
    {-15, RF_TxPowerTable_DEFAULT_PA_ENTRY(12, 3, 0, 4) }, // 0x08CC
    {-12, RF_TxPowerTable_DEFAULT_PA_ENTRY(10, 2, 0, 4) }, // 0x088A
    {-10, RF_TxPowerTable_DEFAULT_PA_ENTRY(18, 3, 0, 0) }, // 0x00D2
    {-9, RF_TxPowerTable_DEFAULT_PA_ENTRY(14, 2, 0, 7) }, // 0x0E8E
    {-6, RF_TxPowerTable_DEFAULT_PA_ENTRY(19, 2, 0, 11) }, // 0x1693
    {-5, RF_TxPowerTable_DEFAULT_PA_ENTRY(21, 2, 0, 11) }, // 0x1695
    {-3, RF_TxPowerTable_DEFAULT_PA_ENTRY(38, 3, 0, 14) }, // 0x1CE6
    {0, RF_TxPowerTable_DEFAULT_PA_ENTRY(30, 1, 0, 21) }, // 0x2A5E
    {1, RF_TxPowerTable_DEFAULT_PA_ENTRY(35, 1, 0, 25) }, // 0x3263
    {2, RF_TxPowerTable_DEFAULT_PA_ENTRY(22, 0, 0, 35) }, // 0x4616
    {3, RF_TxPowerTable_DEFAULT_PA_ENTRY(29, 0, 0, 46) }, // 0x5C1D
    {4, RF_TxPowerTable_DEFAULT_PA_ENTRY(41, 0, 0, 64) }, // 0x8029
    {5, RF_TxPowerTable_DEFAULT_PA_ENTRY(63, 0, 0, 0) }, // 0x003F
    RF_TxPowerTable_TERMINATION_ENTRY
};



//*********************************************************************************
//  RF Setting:   50 kbps, 25 kHz Deviation, 2-GFSK, 100 kHz RX Bandwidth
//
//  PHY:          2gfsk50kbps
//  Setting file: setting_tc706.json
//*********************************************************************************

// PARAMETER SUMMARY
// RX Address Mode: No address check
// Frequency (MHz): 915.0000
// Deviation (kHz): 25.0
// Packet Length Config: Variable
// Max Packet Length: 255
// Preamble Count: 4 Bytes
// Preamble Mode: Send 0 as the first preamble bit
// RX Filter BW (kHz): 98.0
// Symbol Rate (kBaud): 50.000
// Sync Word: 0x930B51DE
// Sync Word Length: 32 Bits
// TX Power (dBm): 12
// Whitening: No whitening

// TI-RTOS RF Mode Object
RF_Mode RF_prop =
{
    .rfMode = RF_MODE_AUTO,
    .cpePatchFxn = &rf_patch_cpe_multi_protocol,
    .mcePatchFxn = 0,
    .rfePatchFxn = 0
};

// Overrides for CMD_PROP_RADIO_DIV_SETUP_PA
uint32_t pOverrides[] =
{
    // override_txsub1_placeholder.json
    // TX sub-1GHz power override
    TXSUB1_POWER_OVERRIDE(0x00BE33),
    // override_prop_common.json
    // Tx: Set DCDC settings IPEAK=7, dither = off
    (uint32_t)0x004388D3,
    // override_prop_common_sub1g.json
    // TX: Set FSCA divider bias to 1
    HW32_ARRAY_OVERRIDE(0x405C,0x0001),
    // TX: Set FSCA divider bias to 1
    (uint32_t)0x08141131,
    // override_tc706.json
    // Tx: Configure PA ramp time, PACTL2.RC=0x1 (in ADI0, set PACTL2[4:3]=0x1)
    ADI_2HALFREG_OVERRIDE(0,16,0x8,0x8,17,0x1,0x1),
    // Rx: Set AGC reference level to 0x1A (default: 0x2E)
    HW_REG_OVERRIDE(0x609C,0x001A),
    // Rx: Set RSSI offset to adjust reported RSSI by -1 dB at 779-930 MHz
    (uint32_t)0x000188A3,
    // Rx: Set anti-aliasing filter bandwidth to 0xD (in ADI0, set IFAMPCTL3[7:4]=0xD)
    ADI_HALFREG_OVERRIDE(0,61,0xF,0xD),
    // Tx: Configure PA ramping, set wait time before turning off (0x1A ticks of 16/24 us = 17.3 us).
    HW_REG_OVERRIDE(0x6028,0x001A),
    (uint32_t)0xFFFFFFFF
};



// CMD_PROP_RADIO_DIV_SETUP_PA
// Proprietary Mode Radio Setup Command for All Frequency Bands
rfc_CMD_PROP_RADIO_DIV_SETUP_PA_t RF_cmdPropRadioDivSetup =
{
    .commandNo = 0x3807,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .modulation.modType = 0x1,
    .modulation.deviation = 0x64,
    .modulation.deviationStepSz = 0x0,
    .symbolRate.preScale = 0xF,
    .symbolRate.rateWord = 0x8000,
    .symbolRate.decimMode = 0x0,
    .rxBw = 0x52,
    .preamConf.nPreamBytes = 0x4,
    .preamConf.preamMode = 0x0,
    .formatConf.nSwBits = 0x20,
    .formatConf.bBitReversal = 0x0,
    .formatConf.bMsbFirst = 0x1,
    .formatConf.fecMode = 0x0,
    .formatConf.whitenMode = 0x0,
    .config.frontEndMode = 0x0,
    .config.biasMode = 0x1,
    .config.analogCfgMode = 0x0,
    .config.bNoFsPowerUp = 0x0,
    .config.bSynthNarrowBand = 0x0,
    .txPower = 0xFFFE,
    .pRegOverride = pOverrides,
    .centerFreq = 0x0393,
    .intFreq = 0x8000,
    .loDivider = 0x05,
    .pRegOverrideTxStd = 0,
    .pRegOverrideTx20 = 0
};

// CMD_FS
// Frequency Synthesizer Programming Command
rfc_CMD_FS_t RF_cmdFs =
{
    .commandNo = 0x0803,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .frequency = 0x0393,
    .fractFreq = 0x0000,
    .synthConf.bTxMode = 0x0,
    .synthConf.refFreq = 0x0,
    .__dummy0 = 0x00,
    .__dummy1 = 0x00,
    .__dummy2 = 0x00,
    .__dummy3 = 0x0000
};

// CMD_PROP_TX
// Proprietary Mode Transmit Command
rfc_CMD_PROP_TX_t RF_cmdPropTx =
{
    .commandNo = 0x3801,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bVarLen = 0x1,
    .pktLen = 0x14,
    .syncWord = 0x930B51DE,
    .pPkt = 0
};

// CMD_PROP_RX
// Proprietary Mode Receive Command
rfc_CMD_PROP_RX_t RF_cmdPropRx =
{
    .commandNo = 0x3802,
    .status = 0x0000,
    .pNextOp = 0,
    .startTime = 0x00000000,
    .startTrigger.triggerType = 0x0,
    .startTrigger.bEnaCmd = 0x0,
    .startTrigger.triggerNo = 0x0,
    .startTrigger.pastTrig = 0x0,
    .condition.rule = 0x1,
    .condition.nSkip = 0x0,
    .pktConf.bFsOff = 0x0,
    .pktConf.bRepeatOk = 0x0,
    .pktConf.bRepeatNok = 0x0,
    .pktConf.bUseCrc = 0x1,
    .pktConf.bVarLen = 0x1,
    .pktConf.bChkAddress = 0x0,
    .pktConf.endType = 0x0,
    .pktConf.filterOp = 0x0,
    .rxConf.bAutoFlushIgnored = 0x0,
    .rxConf.bAutoFlushCrcErr = 0x0,
    .rxConf.bIncludeHdr = 0x1,
    .rxConf.bIncludeCrc = 0x0,
    .rxConf.bAppendRssi = 0x0,
    .rxConf.bAppendTimestamp = 0x0,
    .rxConf.bAppendStatus = 0x1,
    .syncWord = 0x930B51DE,
    .maxPktLen = 0xFF,
    .address0 = 0xAA,
    .address1 = 0xBB,
    .endTrigger.triggerType = 0x1,
    .endTrigger.bEnaCmd = 0x0,
    .endTrigger.triggerNo = 0x0,
    .endTrigger.pastTrig = 0x0,
    .endTime = 0x00000000,
    .pQueue = 0,
    .pOutput = 0
};


