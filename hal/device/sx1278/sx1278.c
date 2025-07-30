/*
 * Copyright (C) 2018 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "./sx1278.h"

#include "emmk-config.h"

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define _ASSERT(x) \
    do { \
        if (!(x)) { \
            for (;;); \
        } \
    } while(0)
    
#define XTAL_FREQ    32000000
#define FREQ_STEP    61.03515625
    
////
//// Register define
////
#define REG_LR_FIFO                                 0x00 
// Common settings
#define REG_LR_OPMODE                               0x01 
#define REG_LR_BANDSETTING                          0x04
#define REG_LR_FRFMSB                               0x06 
#define REG_LR_FRFMID                               0x07
#define REG_LR_FRFLSB                               0x08 
// Tx settings
#define REG_LR_PACONFIG                             0x09 
#define REG_LR_PARAMP                               0x0A 
#define REG_LR_OCP                                  0x0B 
// Rx settings
#define REG_LR_LNA                                  0x0C 
// LoRa registers
#define REG_LR_FIFOADDRPTR                          0x0D
#define REG_LR_FIFOTXBASEADDR                       0x0E 
#define REG_LR_FIFORXBASEADDR                       0x0F 
#define REG_LR_FIFORXCURRENTADDR                    0x10 
#define REG_LR_IRQFLAGSMASK                         0x11 
#define REG_LR_IRQFLAGS                             0x12 
#define REG_LR_NBRXBYTES                            0x13 
#define REG_LR_RXHEADERCNTVALUEMSB                  0x14 
#define REG_LR_RXHEADERCNTVALUELSB                  0x15 
#define REG_LR_RXPACKETCNTVALUEMSB                  0x16 
#define REG_LR_RXPACKETCNTVALUELSB                  0x17 
#define REG_LR_MODEMSTAT                            0x18 
#define REG_LR_PKTSNRVALUE                          0x19 
#define REG_LR_PKTRSSIVALUE                         0x1A 
#define REG_LR_RSSIVALUE                            0x1B 
#define REG_LR_HOPCHANNEL                           0x1C 
#define REG_LR_MODEMCONFIG1                         0x1D 
#define REG_LR_MODEMCONFIG2                         0x1E 
#define REG_LR_SYMBTIMEOUTLSB                       0x1F 
#define REG_LR_PREAMBLEMSB                          0x20 
#define REG_LR_PREAMBLELSB                          0x21 
#define REG_LR_PAYLOADLENGTH                        0x22 
#define REG_LR_PAYLOADMAXLENGTH                     0x23 
#define REG_LR_HOPPERIOD                            0x24 
#define REG_LR_FIFORXBYTEADDR                       0x25
#define REG_LR_MODEMCONFIG3                         0x26
// end of documented register in datasheet
// I/O settings
#define REG_LR_DIOMAPPING1                          0x40
#define REG_LR_DIOMAPPING2                          0x41
// Version
#define REG_LR_VERSION                              0x42
// Additional settings
#define REG_LR_PLLHOP                               0x44
#define REG_LR_TCXO                                 0x4B
#define REG_LR_PADAC                                0x4D
#define REG_LR_FORMERTEMP                           0x5B
#define REG_LR_BITRATEFRAC                          0x5D
#define REG_LR_AGCREF                               0x61
#define REG_LR_AGCTHRESH1                           0x62
#define REG_LR_AGCTHRESH2                           0x63
#define REG_LR_AGCTHRESH3                           0x64


//// RegOpMode
#define RFLR_OPMODE_LONGRANGEMODE_MASK              0x7F 
#define RFLR_OPMODE_LONGRANGEMODE_OFF               0x00 // Default
#define RFLR_OPMODE_LONGRANGEMODE_ON                0x80 

#define RFLR_OPMODE_ACCESSSHAREDREG_MASK            0xBF 
#define RFLR_OPMODE_ACCESSSHAREDREG_ENABLE          0x40 
#define RFLR_OPMODE_ACCESSSHAREDREG_DISABLE         0x00 // Default

#define RFLR_OPMODE_FREQMODE_ACCESS_MASK            0xF7
#define RFLR_OPMODE_FREQMODE_ACCESS_LF              0x08 // Default
#define RFLR_OPMODE_FREQMODE_ACCESS_HF              0x00 

#define RFLR_OPMODE_MASK                            0xF8 
#define RFLR_OPMODE_SLEEP                           0x00 
#define RFLR_OPMODE_STANDBY                         0x01 // Default
#define RFLR_OPMODE_SYNTHESIZER_TX                  0x02 
#define RFLR_OPMODE_TRANSMITTER                     0x03 
#define RFLR_OPMODE_SYNTHESIZER_RX                  0x04 
#define RFLR_OPMODE_RECEIVER                        0x05 
// LoRa specific modes
#define RFLR_OPMODE_RECEIVER_SINGLE                 0x06 
#define RFLR_OPMODE_CAD                             0x07 

//// RegBandSetting 
#define RFLR_BANDSETTING_MASK                    0x3F 
#define RFLR_BANDSETTING_AUTO                    0x00 // Default
#define RFLR_BANDSETTING_DIV_BY_1                0x40
#define RFLR_BANDSETTING_DIV_BY_2                0x80
#define RFLR_BANDSETTING_DIV_BY_6                0xC0

//// RegFrf (MHz)
#define RFLR_FRFMSB_434_MHZ                         0x6C // Default
#define RFLR_FRFMID_434_MHZ                         0x80 // Default
#define RFLR_FRFLSB_434_MHZ                         0x00 // Default

#define RFLR_FRFMSB_863_MHZ                         0xD7
#define RFLR_FRFMID_863_MHZ                         0xC0
#define RFLR_FRFLSB_863_MHZ                         0x00
#define RFLR_FRFMSB_864_MHZ                         0xD8
#define RFLR_FRFMID_864_MHZ                         0x00
#define RFLR_FRFLSB_864_MHZ                         0x00
#define RFLR_FRFMSB_865_MHZ                         0xD8
#define RFLR_FRFMID_865_MHZ                         0x40
#define RFLR_FRFLSB_865_MHZ                         0x00
#define RFLR_FRFMSB_866_MHZ                         0xD8
#define RFLR_FRFMID_866_MHZ                         0x80
#define RFLR_FRFLSB_866_MHZ                         0x00
#define RFLR_FRFMSB_867_MHZ                         0xD8
#define RFLR_FRFMID_867_MHZ                         0xC0
#define RFLR_FRFLSB_867_MHZ                         0x00
#define RFLR_FRFMSB_868_MHZ                         0xD9
#define RFLR_FRFMID_868_MHZ                         0x00
#define RFLR_FRFLSB_868_MHZ                         0x00
#define RFLR_FRFMSB_869_MHZ                         0xD9
#define RFLR_FRFMID_869_MHZ                         0x40
#define RFLR_FRFLSB_869_MHZ                         0x00
#define RFLR_FRFMSB_870_MHZ                         0xD9
#define RFLR_FRFMID_870_MHZ                         0x80
#define RFLR_FRFLSB_870_MHZ                         0x00

#define RFLR_FRFMSB_902_MHZ                         0xE1
#define RFLR_FRFMID_902_MHZ                         0x80
#define RFLR_FRFLSB_902_MHZ                         0x00
#define RFLR_FRFMSB_903_MHZ                         0xE1
#define RFLR_FRFMID_903_MHZ                         0xC0
#define RFLR_FRFLSB_903_MHZ                         0x00
#define RFLR_FRFMSB_904_MHZ                         0xE2
#define RFLR_FRFMID_904_MHZ                         0x00
#define RFLR_FRFLSB_904_MHZ                         0x00
#define RFLR_FRFMSB_905_MHZ                         0xE2
#define RFLR_FRFMID_905_MHZ                         0x40
#define RFLR_FRFLSB_905_MHZ                         0x00
#define RFLR_FRFMSB_906_MHZ                         0xE2
#define RFLR_FRFMID_906_MHZ                         0x80
#define RFLR_FRFLSB_906_MHZ                         0x00
#define RFLR_FRFMSB_907_MHZ                         0xE2
#define RFLR_FRFMID_907_MHZ                         0xC0
#define RFLR_FRFLSB_907_MHZ                         0x00
#define RFLR_FRFMSB_908_MHZ                         0xE3
#define RFLR_FRFMID_908_MHZ                         0x00
#define RFLR_FRFLSB_908_MHZ                         0x00
#define RFLR_FRFMSB_909_MHZ                         0xE3
#define RFLR_FRFMID_909_MHZ                         0x40
#define RFLR_FRFLSB_909_MHZ                         0x00
#define RFLR_FRFMSB_910_MHZ                         0xE3
#define RFLR_FRFMID_910_MHZ                         0x80
#define RFLR_FRFLSB_910_MHZ                         0x00
#define RFLR_FRFMSB_911_MHZ                         0xE3
#define RFLR_FRFMID_911_MHZ                         0xC0
#define RFLR_FRFLSB_911_MHZ                         0x00
#define RFLR_FRFMSB_912_MHZ                         0xE4
#define RFLR_FRFMID_912_MHZ                         0x00
#define RFLR_FRFLSB_912_MHZ                         0x00
#define RFLR_FRFMSB_913_MHZ                         0xE4
#define RFLR_FRFMID_913_MHZ                         0x40
#define RFLR_FRFLSB_913_MHZ                         0x00
#define RFLR_FRFMSB_914_MHZ                         0xE4
#define RFLR_FRFMID_914_MHZ                         0x80
#define RFLR_FRFLSB_914_MHZ                         0x00
#define RFLR_FRFMSB_915_MHZ                         0xE4  // Default
#define RFLR_FRFMID_915_MHZ                         0xC0  // Default
#define RFLR_FRFLSB_915_MHZ                         0x00  // Default
#define RFLR_FRFMSB_916_MHZ                         0xE5
#define RFLR_FRFMID_916_MHZ                         0x00
#define RFLR_FRFLSB_916_MHZ                         0x00
#define RFLR_FRFMSB_917_MHZ                         0xE5
#define RFLR_FRFMID_917_MHZ                         0x40
#define RFLR_FRFLSB_917_MHZ                         0x00
#define RFLR_FRFMSB_918_MHZ                         0xE5
#define RFLR_FRFMID_918_MHZ                         0x80
#define RFLR_FRFLSB_918_MHZ                         0x00
#define RFLR_FRFMSB_919_MHZ                         0xE5
#define RFLR_FRFMID_919_MHZ                         0xC0
#define RFLR_FRFLSB_919_MHZ                         0x00
#define RFLR_FRFMSB_920_MHZ                         0xE6
#define RFLR_FRFMID_920_MHZ                         0x00
#define RFLR_FRFLSB_920_MHZ                         0x00
#define RFLR_FRFMSB_921_MHZ                         0xE6
#define RFLR_FRFMID_921_MHZ                         0x40
#define RFLR_FRFLSB_921_MHZ                         0x00
#define RFLR_FRFMSB_922_MHZ                         0xE6
#define RFLR_FRFMID_922_MHZ                         0x80
#define RFLR_FRFLSB_922_MHZ                         0x00
#define RFLR_FRFMSB_923_MHZ                         0xE6
#define RFLR_FRFMID_923_MHZ                         0xC0
#define RFLR_FRFLSB_923_MHZ                         0x00
#define RFLR_FRFMSB_924_MHZ                         0xE7
#define RFLR_FRFMID_924_MHZ                         0x00
#define RFLR_FRFLSB_924_MHZ                         0x00
#define RFLR_FRFMSB_925_MHZ                         0xE7
#define RFLR_FRFMID_925_MHZ                         0x40
#define RFLR_FRFLSB_925_MHZ                         0x00
#define RFLR_FRFMSB_926_MHZ                         0xE7
#define RFLR_FRFMID_926_MHZ                         0x80
#define RFLR_FRFLSB_926_MHZ                         0x00
#define RFLR_FRFMSB_927_MHZ                         0xE7
#define RFLR_FRFMID_927_MHZ                         0xC0
#define RFLR_FRFLSB_927_MHZ                         0x00
#define RFLR_FRFMSB_928_MHZ                         0xE8
#define RFLR_FRFMID_928_MHZ                         0x00
#define RFLR_FRFLSB_928_MHZ                         0x00

//// RegPaConfig
#define RFLR_PACONFIG_PASELECT_MASK                 0x7F 
#define RFLR_PACONFIG_PASELECT_PABOOST              0x80 
#define RFLR_PACONFIG_PASELECT_RFO                  0x00 // Default

#define RFLR_PACONFIG_MAX_POWER_MASK                0x8F

#define RFLR_PACONFIG_OUTPUTPOWER_MASK              0xF0 

//// RegPaRamp
#define RFLR_PARAMP_TXBANDFORCE_MASK                0xEF 
#define RFLR_PARAMP_TXBANDFORCE_BAND_SEL            0x10 
#define RFLR_PARAMP_TXBANDFORCE_AUTO                0x00 // Default

#define RFLR_PARAMP_MASK                            0xF0 
#define RFLR_PARAMP_3400_US                         0x00 
#define RFLR_PARAMP_2000_US                         0x01 
#define RFLR_PARAMP_1000_US                         0x02
#define RFLR_PARAMP_0500_US                         0x03 
#define RFLR_PARAMP_0250_US                         0x04 
#define RFLR_PARAMP_0125_US                         0x05 
#define RFLR_PARAMP_0100_US                         0x06 
#define RFLR_PARAMP_0062_US                         0x07 
#define RFLR_PARAMP_0050_US                         0x08 
#define RFLR_PARAMP_0040_US                         0x09 // Default
#define RFLR_PARAMP_0031_US                         0x0A 
#define RFLR_PARAMP_0025_US                         0x0B 
#define RFLR_PARAMP_0020_US                         0x0C 
#define RFLR_PARAMP_0015_US                         0x0D 
#define RFLR_PARAMP_0012_US                         0x0E 
#define RFLR_PARAMP_0010_US                         0x0F 

//// RegOcp
#define RFLR_OCP_MASK                               0xDF 
#define RFLR_OCP_ON                                 0x20 // Default
#define RFLR_OCP_OFF                                0x00   

#define RFLR_OCP_TRIM_MASK                          0xE0
#define RFLR_OCP_TRIM_045_MA                        0x00
#define RFLR_OCP_TRIM_050_MA                        0x01   
#define RFLR_OCP_TRIM_055_MA                        0x02 
#define RFLR_OCP_TRIM_060_MA                        0x03 
#define RFLR_OCP_TRIM_065_MA                        0x04 
#define RFLR_OCP_TRIM_070_MA                        0x05 
#define RFLR_OCP_TRIM_075_MA                        0x06 
#define RFLR_OCP_TRIM_080_MA                        0x07  
#define RFLR_OCP_TRIM_085_MA                        0x08
#define RFLR_OCP_TRIM_090_MA                        0x09 
#define RFLR_OCP_TRIM_095_MA                        0x0A 
#define RFLR_OCP_TRIM_100_MA                        0x0B  // Default
#define RFLR_OCP_TRIM_105_MA                        0x0C 
#define RFLR_OCP_TRIM_110_MA                        0x0D 
#define RFLR_OCP_TRIM_115_MA                        0x0E 
#define RFLR_OCP_TRIM_120_MA                        0x0F 
#define RFLR_OCP_TRIM_130_MA                        0x10
#define RFLR_OCP_TRIM_140_MA                        0x11   
#define RFLR_OCP_TRIM_150_MA                        0x12 
#define RFLR_OCP_TRIM_160_MA                        0x13 
#define RFLR_OCP_TRIM_170_MA                        0x14 
#define RFLR_OCP_TRIM_180_MA                        0x15 
#define RFLR_OCP_TRIM_190_MA                        0x16 
#define RFLR_OCP_TRIM_200_MA                        0x17  
#define RFLR_OCP_TRIM_210_MA                        0x18
#define RFLR_OCP_TRIM_220_MA                        0x19 
#define RFLR_OCP_TRIM_230_MA                        0x1A 
#define RFLR_OCP_TRIM_240_MA                        0x1B

//// RegLna
#define RFLR_LNA_GAIN_MASK   0x1F 
#define RFLR_LNA_GAIN_G1     0x20 // Default
#define RFLR_LNA_GAIN_G2     0x40 
#define RFLR_LNA_GAIN_G3     0x60 
#define RFLR_LNA_GAIN_G4     0x80 
#define RFLR_LNA_GAIN_G5     0xA0 
#define RFLR_LNA_GAIN_G6     0xC0 

#define RFLR_LNA_BOOST_LF_MASK                      0xE7 
#define RFLR_LNA_BOOST_LF_DEFAULT                   0x00 // Default
#define RFLR_LNA_BOOST_LF_GAIN                      0x08 
#define RFLR_LNA_BOOST_LF_IP3                       0x10 
#define RFLR_LNA_BOOST_LF_BOOST                     0x18 

#define RFLR_LNA_RXBANDFORCE_MASK                   0xFB 
#define RFLR_LNA_RXBANDFORCE_BAND_SEL               0x04
#define RFLR_LNA_RXBANDFORCE_AUTO                   0x00 // Default

#define RFLR_LNA_BOOST_HF_MASK                      0xFC 
#define RFLR_LNA_BOOST_HF_OFF                       0x00 // Default
#define RFLR_LNA_BOOST_HF_ON                        0x03 

//// RegFifoAddrPtr
#define RFLR_FIFOADDRPTR                            0x00 // Default

//// RegFifoTxBaseAddr
#define RFLR_FIFOTXBASEADDR                         0x80 // Default

//// RegFifoTxBaseAddr
#define RFLR_FIFORXBASEADDR                         0x00 // Default

////RegFifoRxCurrentAddr (Read Only)

//// RegIrqFlagsMask
#define RFLR_IRQFLAGS_RXTIMEOUT_MASK                0x80 
#define RFLR_IRQFLAGS_RXDONE_MASK                   0x40 
#define RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK          0x20 
#define RFLR_IRQFLAGS_VALIDHEADER_MASK              0x10 
#define RFLR_IRQFLAGS_TXDONE_MASK                   0x08 
#define RFLR_IRQFLAGS_CADDONE_MASK                  0x04 
#define RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL_MASK       0x02 
#define RFLR_IRQFLAGS_CADDETECTED_MASK              0x01 


//// RegIrqFlags
#define RFLR_IRQFLAGS_RXTIMEOUT                     0x80 
#define RFLR_IRQFLAGS_RXDONE                        0x40 
#define RFLR_IRQFLAGS_PAYLOADCRCERROR               0x20 
#define RFLR_IRQFLAGS_VALIDHEADER                   0x10 
#define RFLR_IRQFLAGS_TXDONE                        0x08 
#define RFLR_IRQFLAGS_CADDONE                       0x04 
#define RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL            0x02 
#define RFLR_IRQFLAGS_CADDETECTED                   0x01 


////RegModemStat (Read Only)
#define RFLR_MODEMSTAT_RX_CR_MASK                   0x1F 
#define RFLR_MODEMSTAT_MODEM_STATUS_MASK            0xE0 

//// RegModemConfig1
#define RFLR_MODEMCONFIG1_BW_MASK                   0x0F 

#define RFLR_MODEMCONFIG1_BW_7_81_KHZ               0x00 
#define RFLR_MODEMCONFIG1_BW_10_41_KHZ              0x10 
#define RFLR_MODEMCONFIG1_BW_15_62_KHZ              0x20 
#define RFLR_MODEMCONFIG1_BW_20_83_KHZ              0x30 
#define RFLR_MODEMCONFIG1_BW_31_25_KHZ              0x40 
#define RFLR_MODEMCONFIG1_BW_41_66_KHZ              0x50 
#define RFLR_MODEMCONFIG1_BW_62_50_KHZ              0x60 
#define RFLR_MODEMCONFIG1_BW_125_KHZ                0x70 // Default
#define RFLR_MODEMCONFIG1_BW_250_KHZ                0x80 
#define RFLR_MODEMCONFIG1_BW_500_KHZ                0x90 
                                                    
#define RFLR_MODEMCONFIG1_CODINGRATE_MASK           0xF1 
#define RFLR_MODEMCONFIG1_CODINGRATE_4_5            0x02
#define RFLR_MODEMCONFIG1_CODINGRATE_4_6            0x04 // Default
#define RFLR_MODEMCONFIG1_CODINGRATE_4_7            0x06 
#define RFLR_MODEMCONFIG1_CODINGRATE_4_8            0x08 
                                                    
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK       0xFE 
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_ON         0x01 
#define RFLR_MODEMCONFIG1_IMPLICITHEADER_OFF        0x00 // Default

//// RegModemConfig2
#define RFLR_MODEMCONFIG2_SF_MASK                   0x0F 
#define RFLR_MODEMCONFIG2_SF_6                      0x60 
#define RFLR_MODEMCONFIG2_SF_7                      0x70 // Default
#define RFLR_MODEMCONFIG2_SF_8                      0x80 
#define RFLR_MODEMCONFIG2_SF_9                      0x90 
#define RFLR_MODEMCONFIG2_SF_10                     0xA0 
#define RFLR_MODEMCONFIG2_SF_11                     0xB0 
#define RFLR_MODEMCONFIG2_SF_12                     0xC0 

#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_MASK     0xF7 
#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_ON       0x08 
#define RFLR_MODEMCONFIG2_TXCONTINUOUSMODE_OFF      0x00 

#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK         0xFB 
#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_ON           0x04 
#define RFLR_MODEMCONFIG2_RXPAYLOADCRC_OFF          0x00 // Default
 
#define RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK       0xFC 
#define RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB            0x00 // Default

//// RegModemConfig3
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK  0xF7 
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_ON    0x08 
#define RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_OFF   0x00 // Default

#define RFLR_MODEMCONFIG3_AGCAUTO_MASK              0xFB 
#define RFLR_MODEMCONFIG3_AGCAUTO_ON                0x04 // Default 
#define RFLR_MODEMCONFIG3_AGCAUTO_OFF               0x00 


//// RegHopChannel (Read Only)                                                                      
#define RFLR_HOPCHANNEL_PLL_LOCK_TIMEOUT_MASK       0x7F 
#define RFLR_HOPCHANNEL_PLL_LOCK_FAIL               0x80 
#define RFLR_HOPCHANNEL_PLL_LOCK_SUCCEED            0x00 // Default
                                                    
#define RFLR_HOPCHANNEL_PAYLOAD_CRC16_MASK          0xBF
#define RFLR_HOPCHANNEL_PAYLOAD_CRC16_ON            0x40
#define RFLR_HOPCHANNEL_PAYLOAD_CRC16_OFF           0x00 // Default

#define RFLR_HOPCHANNEL_CHANNEL_MASK                0x3F 

//// RegDioMapping1
#define RFLR_DIOMAPPING1_DIO0_MASK                  0x3F
#define RFLR_DIOMAPPING1_DIO0_00                    0x00  // Default
#define RFLR_DIOMAPPING1_DIO0_01                    0x40
#define RFLR_DIOMAPPING1_DIO0_10                    0x80
#define RFLR_DIOMAPPING1_DIO0_11                    0xC0

#define RFLR_DIOMAPPING1_DIO1_MASK                  0xCF
#define RFLR_DIOMAPPING1_DIO1_00                    0x00  // Default
#define RFLR_DIOMAPPING1_DIO1_01                    0x10
#define RFLR_DIOMAPPING1_DIO1_10                    0x20
#define RFLR_DIOMAPPING1_DIO1_11                    0x30

#define RFLR_DIOMAPPING1_DIO2_MASK                  0xF3
#define RFLR_DIOMAPPING1_DIO2_00                    0x00  // Default
#define RFLR_DIOMAPPING1_DIO2_01                    0x04
#define RFLR_DIOMAPPING1_DIO2_10                    0x08
#define RFLR_DIOMAPPING1_DIO2_11                    0x0C

#define RFLR_DIOMAPPING1_DIO3_MASK                  0xFC
#define RFLR_DIOMAPPING1_DIO3_00                    0x00  // Default
#define RFLR_DIOMAPPING1_DIO3_01                    0x01
#define RFLR_DIOMAPPING1_DIO3_10                    0x02
#define RFLR_DIOMAPPING1_DIO3_11                    0x03

//// RegDioMapping2
#define RFLR_DIOMAPPING2_DIO4_MASK                  0x3F
#define RFLR_DIOMAPPING2_DIO4_00                    0x00  // Default
#define RFLR_DIOMAPPING2_DIO4_01                    0x40
#define RFLR_DIOMAPPING2_DIO4_10                    0x80
#define RFLR_DIOMAPPING2_DIO4_11                    0xC0

#define RFLR_DIOMAPPING2_DIO5_MASK                  0xCF
#define RFLR_DIOMAPPING2_DIO5_00                    0x00  // Default
#define RFLR_DIOMAPPING2_DIO5_01                    0x10
#define RFLR_DIOMAPPING2_DIO5_10                    0x20
#define RFLR_DIOMAPPING2_DIO5_11                    0x30

#define RFLR_DIOMAPPING2_MAP_MASK                   0xFE
#define RFLR_DIOMAPPING2_MAP_PREAMBLEDETECT         0x01
#define RFLR_DIOMAPPING2_MAP_RSSI                   0x00  // Default

//// Constant values need to compute the RSSI value
#define RSSI_OFFSET_LF                              -155.0
#define RSSI_OFFSET_HF                              -150.0

#define NOISE_ABSOLUTE_ZERO                         -174.0

#define NOISE_FIGURE_LF                                4.0
#define NOISE_FIGURE_HF                                6.0 

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */
 
/*@{*/

static const double _SIGNAL_BW_LOG[] = {
    3.8927900303521316335038277369285,  // 7.8 kHz
    4.0177301567005500940384239336392,  // 10.4 kHz
    4.193820026016112828717566631653,   // 15.6 kHz
    4.31875866931372901183597627752391, // 20.8 kHz
    4.4948500216800940239313055263775,  // 31.2 kHz
    4.6197891057238405255051280399961,  // 41.6 kHz
    4.795880017344075219145044421102,   // 62.5 kHz
    5.0969100130080564143587833158265,  // 125 kHz
    5.397940008672037609572522210551,   // 250 kHz
    5.6989700043360188047862611052755   // 500 kHz
};


static const double _RSSI_OFFSET_LF[] = {
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
    -155.0,
};


const double _RSSI_OFFSET_HF[] = {
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
    -150.0,
};

static const float _BW_TO_FREQ[] = {
    7.8,
    10.4,
    15.6,
    20.8,
    31.2,
    41.5,
    62.5,
    125,
    500,
};

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/


/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/



/*@}*/

/**
 * @addtogroup Private Hal interface func
 * @note none
 */
 
/*@{*/

static inline void _readData(SX1278_t *handler, uint8_t address, uint8_t *regData, uint16_t size) {
    handler->interface->onSelect(handler);
    
    address = address & 0x7F;
    handler->interface->onSends(handler, &address, 1);
    handler->interface->onRecvs(handler, regData, size);
    
    handler->interface->onUnselect(handler);
}


static inline void _writeData(SX1278_t *handler, uint8_t address, uint8_t *regData, uint16_t size) {
    handler->interface->onSelect(handler);
    
    address = address | 0x80;
    handler->interface->onSends(handler, &address, 1);
    handler->interface->onSends(handler, regData, size);
    
    handler->interface->onUnselect(handler);
}


static inline void _writeFifo(SX1278_t *handler, uint8_t *data, uint16_t size) {
    _writeData(handler, 0x00, data, size);
}


static inline void _readFifo(SX1278_t *handler, uint8_t *data, uint16_t size) {
    _readData(handler, 0x00, data, size);
}


/*@}*/

/**
 * @addtogroup Private radio base func
 * @note none
 */
 
/*@{*/

static uint8_t _setOperateMode(SX1278_t *handler, uint8_t mode) {
    uint8_t opModeNow = 0;
    uint8_t opModePrev;
    
    _readData(handler, REG_LR_OPMODE, &opModeNow, 1);
    
    opModePrev = opModeNow & ~RFLR_OPMODE_MASK;
    
    if (mode != opModePrev) {
        opModeNow = (opModeNow & RFLR_OPMODE_MASK) | mode;
        
        _writeData(handler, REG_LR_OPMODE, &opModeNow, 1);
    }
    
    return opModeNow;
}


static void _setPower(SX1278_t *handler, uint8_t isPowerUp) {
    uint8_t opModeNow = 0;
    
    if (isPowerUp) {
        _readData(handler, REG_LR_OPMODE, &opModeNow, 1);
        
        opModeNow = _setOperateMode(handler, RFLR_OPMODE_SLEEP);
        
        opModeNow = (opModeNow & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_ON;
        _writeData(handler, REG_LR_OPMODE, &opModeNow, 1);
        
        opModeNow = _setOperateMode(handler, RFLR_OPMODE_STANDBY);
    } else {
        _readData(handler, REG_LR_OPMODE, &opModeNow, 1);
        
        opModeNow = _setOperateMode(handler, RFLR_OPMODE_SLEEP);
        
        opModeNow = (opModeNow & RFLR_OPMODE_LONGRANGEMODE_MASK) | RFLR_OPMODE_LONGRANGEMODE_OFF;
        _writeData(handler, REG_LR_OPMODE, &opModeNow, 1);
        
        opModeNow = _setOperateMode(handler, RFLR_OPMODE_STANDBY);
    }
}


static void _setRFFrequency(SX1278_t *handler, uint32_t freq) {
    freq = (uint32_t) ((double) freq / (double) FREQ_STEP);
    
    handler->regBuffer[0] = (uint8_t) ((freq >> 16) & 0xFF);
    handler->regBuffer[1] = (uint8_t) ((freq >> 8) & 0xFF);
    handler->regBuffer[2] = (uint8_t) (freq & 0xFF);
    
    _writeData(handler, REG_LR_FRFMSB, handler->regBuffer, 3);
}


static void _setNbTrigPeaks(SX1278_t *handler, uint8_t value) {
    uint8_t testReserved31;
    
    _readData(handler, 0x31, &testReserved31, 1);
    testReserved31 = (testReserved31 & 0xF8) | value;
    _writeData(handler, 0x31, &testReserved31, 1);
}


static void _setSpreadingFactor(SX1278_t *handler, uint8_t factor) {
    if (factor > 12) {
        factor = 12;
    } else if (factor < 6) {
        factor = 6; 
    }

    if (factor == 6) {
        _setNbTrigPeaks(handler, 5);
    } else {
        _setNbTrigPeaks(handler, 3);
    }

    _readData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 1);

    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG2_SF_MASK) | (factor << 4);
    
    _writeData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 1);    
}


static void _setErrorCoding(SX1278_t *handler, uint8_t value) {
    _readData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1); 
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG1_CODINGRATE_MASK) | (value << 1);
    _writeData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1);
}


static void _setPacketCrcOn(SX1278_t *handler, uint8_t enable) {
    _readData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 1);
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK) | (enable << 2);
    _writeData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 1);
}


static void _setSignalBandwidth(SX1278_t *handler, uint8_t bw) {
    _readData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1);
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG1_BW_MASK) | (bw << 4);
    _writeData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1);
}


static void _setImplicitHeaderOn(SX1278_t *handler, uint8_t enable) {
    _readData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1);
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK) | (enable);
    _writeData(handler, REG_LR_MODEMCONFIG1, &handler->regBuffer[0], 1);
}


static void _setSymbTimeout(SX1278_t *handler, uint16_t value) {
    _readData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 2);

    handler->regBuffer[0] =
        (handler->regBuffer[0] & RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK) 
        | ((value >> 8) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK);
    handler->regBuffer[1] = value & 0xFF;
    
    _writeData(handler, REG_LR_MODEMCONFIG2, &handler->regBuffer[0], 2);
}


static void _setPayloadLength(SX1278_t *handler, uint8_t value) {
    _writeData(handler, REG_LR_PAYLOADLENGTH, &value, 1);
}


static void _setLowDatarateOptimize(SX1278_t *handler, uint8_t enable) {
    _readData(handler, REG_LR_MODEMCONFIG3, &handler->regBuffer[0], 1);
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK) | (enable << 3);
    _writeData(handler, REG_LR_MODEMCONFIG3, &handler->regBuffer[0], 1);
}


static void _setPAOutput(SX1278_t *handler, uint8_t outputPin) {
    _readData(handler, REG_LR_PACONFIG, &handler->regBuffer[0], 1);
    handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_PASELECT_MASK) | outputPin;
    _writeData(handler, REG_LR_PACONFIG, &handler->regBuffer[0], 1);
}


static void _setPa20dBm(SX1278_t *handler, uint8_t enale) {
    _readData(handler, REG_LR_PADAC, &handler->regBuffer[1], 1);
    _readData(handler, REG_LR_PACONFIG, &handler->regBuffer[0], 1);

    if ((handler->regBuffer[0] & RFLR_PACONFIG_PASELECT_PABOOST) == RFLR_PACONFIG_PASELECT_PABOOST) {    
        if (enale == 1) {
            handler->regBuffer[1] = 0x87;
        }
    } else {
        handler->regBuffer[1] = 0x84;
    }
    
    _writeData(handler, REG_LR_PADAC, &handler->regBuffer[1], 1);
}


void _setPreambleLength(SX1278_t *handler, uint16_t length) {
    _ASSERT(handler != NULL);
    
    if (length < 6) {
        length = 6;
    }
    
    _readData(handler, REG_LR_PREAMBLEMSB, &handler->regBuffer[0], 2);

    handler->regBuffer[0] = (length >> 8) & 0x00FF;
    handler->regBuffer[1] = length & 0xFF;
    
    _writeData(handler, REG_LR_PREAMBLEMSB, &handler->regBuffer[0], 2);
}


static void _setRFPower(SX1278_t *handler, int8_t power) {
    _readData(handler, REG_LR_PACONFIG, &handler->regBuffer[0], 1);
    _readData(handler, REG_LR_PADAC, &handler->regBuffer[1], 1);
    
    if ((handler->regBuffer[0] & RFLR_PACONFIG_PASELECT_PABOOST) == RFLR_PACONFIG_PASELECT_PABOOST) {
        if ((handler->regBuffer[1] & 0x87) == 0x87) {
            if (power < 5) {
                power = 5;
            }
            
            if (power > 20) {
                power = 20;
            }
            
            handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
            handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t) ((uint16_t) (power - 5) & 0x0F);
        } else {
            if (power < 2) {
                power = 2;
            }
            
            if (power > 17) {
                power = 17;
            }
            
            handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
            handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t) ((uint16_t) (power - 2) & 0x0F);
        }
    } else {
        if (power < -1) {
            power = -1;
        }
        
        if (power > 14) {
            power = 14;
        }
        
        handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_MAX_POWER_MASK) | 0x70;
        handler->regBuffer[0] = (handler->regBuffer[0] & RFLR_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t) ((uint16_t) (power + 1) & 0x0F);
    }
    
    _writeData(handler, REG_LR_PACONFIG, &handler->regBuffer[0], 1);
}


static void _setRxGain(SX1278_t *handler, uint8_t lna) {
    _writeData(handler, REG_LR_LNA, &lna, 1);
}


static uint8_t _getRxGain(SX1278_t *handler) {
    _readData(handler, REG_LR_LNA, &handler->regBuffer[0], 1);
    
    return (handler->regBuffer[0] >> 5) & 0x07;
}


static uint32_t _qpow(uint32_t a, uint32_t b) {
    uint32_t c, d; 
    c = 1; 
    d = a;
    
    while (b > 0) {
        if (b & 1)
          c *= d;
        b = b >> 1;
        d = d * d;
    }
    
    return c;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none   
 */
 
/*@{*/

void sx1278_init(SX1278_t *handler, 
        const SX1278_Setting_t *defaultSetting, 
        uint8_t preambleLen,
        const SX1278_Interface_t *interface) {
    _ASSERT(handler != NULL);
    _ASSERT(interface != NULL);

    memset(handler, 0x00, sizeof(SX1278_t));
    
    handler->interface = interface;
    handler->interface->onInit(handler);
    handler->interface->onUnselect(handler);
    handler->interface->onReset(handler);
    handler->currentState = SX1278_STATE_STANDBY;
    
    handler->flag = 0;
    
    //// Default setting
    handler->setting = defaultSetting;
    
    //// Interface check
    _readData(handler, 0x06, &handler->regBuffer[0], 1);
    _readData(handler, 0x06, &handler->regBuffer[0], 1);
    if (handler->regBuffer[0] != 0x6C) {
        handler->flag |= SX1278_FLAG_HAL_FAILED;
        return;
    }
    
    handler->calcConfig.preambleLen = preambleLen;
    handler->calcConfig.tsmValue = (1.0f / (_BW_TO_FREQ[handler->setting->SignalBw] / _qpow(2, handler->setting->SpreadingFactor)));
    handler->calcConfig.cadTime = (_qpow(2, handler->setting->SpreadingFactor) + 32.0f) / _BW_TO_FREQ[handler->setting->SignalBw];
    
    //// Startup
    _setPower(handler, 1);
    _setRxGain(handler, RFLR_LNA_GAIN_G1);
    _setRFFrequency(handler, handler->setting->RFFrequency);
    _setSpreadingFactor(handler, handler->setting->SpreadingFactor);
    _setErrorCoding(handler, handler->setting->ErrorCoding);
    _setPacketCrcOn(handler, handler->setting->CrcOn);
    _setSignalBandwidth(handler, handler->setting->SignalBw);
    _setImplicitHeaderOn(handler, handler->setting->ImplicitHeaderOn);
    _setSymbTimeout(handler, 0x3FF);
    _setPayloadLength(handler, handler->setting->PayloadLength);
    _setLowDatarateOptimize(handler, 1);
    _setPreambleLength(handler, handler->calcConfig.preambleLen);
    if (handler->setting->RFFrequency > 360000000) {
        _setPAOutput(handler, RFLR_PACONFIG_PASELECT_PABOOST);
        _setPa20dBm(handler, 1);
        _setRFPower(handler, 20);
    } else {
        _setPAOutput(handler,RFLR_PACONFIG_PASELECT_RFO);
        _setPa20dBm(handler, 0);
        _setRFPower(handler, 14);
    } 
    _setOperateMode(handler, RFLR_OPMODE_STANDBY);
    
    handler->flag |= SX1278_FLAG_IS_IDLE;
}


SX1278_ProcessReturnCodes_t sx1278_poll(SX1278_t *handler) {
    _ASSERT(handler != NULL);
    
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return RF_HAL_FAILED;
    }
    
    SX1278_ProcessReturnCodes_t rc = RF_BUSY;
    
    switch (handler->currentState) {
        case SX1278_STATE_STANDBY:
        case SX1278_STATE_SLEEP:
            rc = RF_IDLE;
            break;
        
        case SX1278_STATE_TX_INIT: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);

            // irqFlagsMask
            handler->regBuffer[0] = RFLR_IRQFLAGS_RXTIMEOUT |
                RFLR_IRQFLAGS_RXDONE |
                RFLR_IRQFLAGS_PAYLOADCRCERROR |
                RFLR_IRQFLAGS_VALIDHEADER |
                // RFLR_IRQFLAGS_TXDONE |
                RFLR_IRQFLAGS_CADDONE |
                RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                RFLR_IRQFLAGS_CADDETECTED;
            _writeData(handler, REG_LR_IRQFLAGSMASK, &handler->regBuffer[0], 1);
            
            // hopPeriod
            handler->regBuffer[0] = 0;
            _writeData(handler, REG_LR_HOPPERIOD, &handler->regBuffer[0], 1);
            
            //// payloadLength, Initializes the payload size
            handler->regBuffer[0] = handler->transmitPacketSize;
            _writeData(handler, REG_LR_PAYLOADLENGTH, &handler->regBuffer[0], 1);

            // fifoTxBaseAddr, Full buffer used for Tx
            handler->regBuffer[0] = 0x00;
            _writeData(handler, REG_LR_FIFOTXBASEADDR, &handler->regBuffer[0], 1);

            // fifoAddrPtr
            handler->regBuffer[0] = 0x00;
            _writeData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[0], 1);
            
            //// Write payload buffer to LORA modem
            _writeFifo(handler, handler->transmitPacket, handler->transmitPacketSize);
            
            // dioMapping1
            handler->regBuffer[0] = 
                RFLR_DIOMAPPING1_DIO0_01     // DIO0-TxDone
                | RFLR_DIOMAPPING1_DIO1_00   // DIO1-RxTimeout
                | RFLR_DIOMAPPING1_DIO2_00   // DIO2-FhssChangeChannel
                | RFLR_DIOMAPPING1_DIO3_01;  // DIO3-ValidHeader
            _writeData(handler, REG_LR_DIOMAPPING1, &handler->regBuffer[0], 1);
            
            // dioMapping2
            handler->regBuffer[0] = 
                RFLR_DIOMAPPING2_DIO4_01     // DIO4-PllLock  
                | RFLR_DIOMAPPING2_DIO5_00;  // DIO5-ModeReady
            _writeData(handler, REG_LR_DIOMAPPING2, &handler->regBuffer[0], 2);

            _setOperateMode(handler, RFLR_OPMODE_TRANSMITTER);

            handler->currentState = SX1278_STATE_TX_RUNNING;
        } break;
        
        case SX1278_STATE_TX_RUNNING: {
            //// onTxDone
            handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_0);
            if (handler->regBuffer[0] == 0xFF) {
                _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_TXDONE;
            }
            if (handler->regBuffer[0]) {
                handler->regBuffer[0] = RFLR_IRQFLAGS_TXDONE;
                _writeData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->currentState = SX1278_STATE_TX_DONE;
            }

            //// onFHSSChangedChannel
            handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_2);
            if (handler->regBuffer[0] == 0xFF) {
                _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
            }
            if (handler->regBuffer[0]) {
                //// Clear irq
                handler->regBuffer[0] = RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
                _writeData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
            }
            
            if (qSTimer_Expired(&handler->timeout) || !qSTimer_Status(&handler->timeout)) {
                handler->currentState = SX1278_STATE_TX_TIMEOUT;
            }
        } break;
        
        case SX1278_STATE_TX_TIMEOUT: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            //// Clear all Irq
            uint8_t regData = 0xFF;
            _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
            
            handler->currentState = SX1278_STATE_STANDBY;
            rc = RF_TX_TIMEOUT;
        } break;
        
        case SX1278_STATE_TX_DONE: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            //// Clear all Irq
            uint8_t regData = 0xFF;
            _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
            
            handler->currentState = SX1278_STATE_STANDBY;
            rc = RF_TX_DONE;
        } break;
        
        case SX1278_STATE_RX_INIT: {
            handler->flag &= ~SX1278_FLAG_RECV_COMPLETED;
            
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            // irqFlagsMask
            handler->regBuffer[0] = RFLR_IRQFLAGS_RXTIMEOUT |
                // RFLR_IRQFLAGS_RXDONE |
                // RFLR_IRQFLAGS_PAYLOADCRCERROR |
                RFLR_IRQFLAGS_VALIDHEADER |
                RFLR_IRQFLAGS_TXDONE |
                RFLR_IRQFLAGS_CADDONE |
                // RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                RFLR_IRQFLAGS_CADDETECTED;
            _writeData(handler, REG_LR_IRQFLAGSMASK, &handler->regBuffer[0], 1);

            // hopPeriod
            handler->regBuffer[0] = 255;
            _writeData(handler, REG_LR_HOPPERIOD, &handler->regBuffer[0], 1);
            
            // dioMapping1
            handler->regBuffer[0] = 
                RFLR_DIOMAPPING1_DIO0_00       // DIO0-RxDone
                | RFLR_DIOMAPPING1_DIO1_00     // DIO1-RxTimeout
                | RFLR_DIOMAPPING1_DIO2_00     // DIO2-FhssChangeChannel
                | RFLR_DIOMAPPING1_DIO3_00;    // DIO3-CadDone
            _writeData(handler, REG_LR_DIOMAPPING1, &handler->regBuffer[0], 1);
            
            // dioMapping2  
            handler->regBuffer[0] = 
                RFLR_DIOMAPPING2_DIO4_00       // DIO4-CadDetected
                | RFLR_DIOMAPPING2_DIO5_00;    // DIO5-ModeReady
            _writeData(handler, REG_LR_DIOMAPPING2, &handler->regBuffer[0], 1);
            
            if (handler->setting->RxSingleOn) { // Rx single mode
                _setOperateMode(handler, RFLR_OPMODE_RECEIVER_SINGLE);
            } else { // Rx continuous mode
                // fifoRxBaseAddr
                _readData(handler, REG_LR_FIFORXBASEADDR, &handler->regBuffer[1], 1);
                
                // fifoAddrPtr
                handler->regBuffer[0] = handler->regBuffer[1];
                _writeData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[0], 1);
                
                _setOperateMode(handler, RFLR_OPMODE_RECEIVER);
            }
            
            handler->transmitPacketSize = 0;
            handler->currentState = SX1278_STATE_RX_RUNNING;
        } break;
        
        case SX1278_STATE_RX_RUNNING: {
            // onRxDone
            handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_0);
            if (handler->regBuffer[0] == 0xFF) {
                _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_RXDONE;
            }
            if (handler->regBuffer[0]) {
                qSTimer_Set(&handler->timeout, 999999);

                //// Clear Irq
                handler->regBuffer[0] = RFLR_IRQFLAGS_RXDONE;
                _writeData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);

                handler->currentState = SX1278_STATE_RX_DONE;
            }
            
            // onFHSSChangedChannel
            handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_2);
            if (handler->regBuffer[0] == 0xFF) {
                _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
            }
            if (handler->regBuffer[0]) {
                qSTimer_Set(&handler->timeout, handler->rxPacketTimeout);

                //// Clear irq
                handler->regBuffer[0] = RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
                _writeData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                
#if SX1278_IMPL_RSSI == 1
                handler->rxGain = _getRxGain(handler);
#endif
            }
            
            if (handler->setting->RxSingleOn) {
                if (qSTimer_Expired(&handler->timeout) || !qSTimer_Status(&handler->timeout)) {
                    handler->currentState = SX1278_STATE_RX_TIMEOUT;
                }
            }
        } break;
        
        case SX1278_STATE_RX_TIMEOUT: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            //// Clear all Irq
            uint8_t regData = 0xFF;
            _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
            
            handler->currentState = SX1278_STATE_STANDBY;
            rc = RF_RX_TIMEOUT;
        } break;
        
        case SX1278_STATE_RX_DONE: {
            // irqFlags
            _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
            
            //// CRC check
            if ((handler->regBuffer[0] & RFLR_IRQFLAGS_PAYLOADCRCERROR) == RFLR_IRQFLAGS_PAYLOADCRCERROR) {
                //// Clear Irq
                handler->regBuffer[1] = RFLR_IRQFLAGS_PAYLOADCRCERROR;
                _writeData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[1], 1);
                
                if (handler->setting->RxSingleOn) { // Rx single mode
                    handler->currentState = SX1278_STATE_TO_STANDBY;
                } else { // Rx continuous mode
                    handler->currentState = SX1278_STATE_RX_RUNNING;
                }
                
                handler->flag |= SX1278_FLAG_RECV_FAILED;
                break;
            }
            
            _readData(handler, REG_LR_PKTSNRVALUE, &handler->regBuffer[3], 1);
#if SX1278_IMPL_RSSI == 1
            if (handler->regBuffer[3] & 0x80) { // The SNR sign bit is 1
                // Invert and divide by 4
                handler->rxPacketSnrEstimate = ((~handler->regBuffer[3] + 1) & 0xFF) >> 2;
                handler->rxPacketSnrEstimate = -handler->rxPacketSnrEstimate;
            } else {
                // Divide by 4
                handler->rxPacketSnrEstimate = (handler->regBuffer[3] & 0xFF) >> 2;
            }
#endif
            
#if SX1278_IMPL_RSSI == 1            
            if (handler->setting->RFFrequency < 860000000) { // LF  

                if (handler->rxPacketSnrEstimate < 0) {
                    handler->rxPacketRssiValue = 
                        NOISE_ABSOLUTE_ZERO 
                        + 10.0 * _SIGNAL_BW_LOG[handler->setting->SignalBw] 
                        + NOISE_FIGURE_LF + (double) handler->rxPacketSnrEstimate;
                } else {    
                    // pktRssiValue
                    _readData(handler, REG_LR_PKTRSSIVALUE, &handler->regBuffer[0], 1);
                    handler->rxPacketRssiValue = _RSSI_OFFSET_LF[handler->setting->SignalBw] 
                        + (double) handler->regBuffer[0];
                }

            } else { // HF
                if (handler->rxPacketSnrEstimate < 0) {
                    handler->rxPacketRssiValue = 
                        NOISE_ABSOLUTE_ZERO 
                        + 10.0 * _SIGNAL_BW_LOG[handler->setting->SignalBw] 
                        + NOISE_FIGURE_HF + (double) handler->rxPacketSnrEstimate;
                } else {
                    // pktRssiValue
                    _readData(handler, REG_LR_PKTRSSIVALUE, &handler->regBuffer[0], 1);
                    handler->rxPacketRssiValue = 
                        _RSSI_OFFSET_HF[handler->setting->SignalBw] 
                        + (double) handler->regBuffer[0];
                }
            }
#endif           
 
            if (handler->setting->RxSingleOn) { // Rx single mode
                // fifoAddrPtr
                _readData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[0], 1);
                
                // fifoRxBaseAddr
                _readData(handler, REG_LR_FIFORXBASEADDR, &handler->regBuffer[1], 1);
                
                handler->regBuffer[0] = handler->regBuffer[1];
                _writeData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[0], 1);

                if (handler->setting->ImplicitHeaderOn) {
                    // payloadLength
                    _readData(handler, REG_LR_PAYLOADLENGTH, &handler->regBuffer[0], 1);
                    
                    handler->transmitPacketSize = handler->regBuffer[0];
                    if (handler->transmitPacketSize > RF_BUFFER_SIZE) {
                        handler->transmitPacketSize = RF_BUFFER_SIZE;
                    }
                    _readFifo(handler, handler->transmitPacket, handler->transmitPacketSize);
                } else {
                    // nbRxBytes
                    _readData(handler, REG_LR_NBRXBYTES, &handler->regBuffer[0], 1);
                    handler->transmitPacketSize = handler->regBuffer[0];
                    if (handler->transmitPacketSize > RF_BUFFER_SIZE) {
                        handler->transmitPacketSize = RF_BUFFER_SIZE;
                    }
                    _readFifo(handler, handler->transmitPacket, handler->transmitPacketSize);
                }
            } else { // Rx continuous mode
                // fifoRxCurrentAddr
                _readData(handler, REG_LR_FIFORXCURRENTADDR, &handler->regBuffer[0] ,1);

                if (handler->setting->ImplicitHeaderOn) {
                    // payloadLength
                    _readData(handler, REG_LR_PAYLOADLENGTH, &handler->regBuffer[1], 1);
                    handler->transmitPacketSize = handler->regBuffer[1];
                    if (handler->transmitPacketSize > RF_BUFFER_SIZE) {
                        handler->transmitPacketSize = RF_BUFFER_SIZE;
                    }
                    
                    // fifoAddrPtr
                    _readData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[2], 1);

                    handler->regBuffer[2] = handler->regBuffer[0];
                    _writeData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[2], 1);
                    _readFifo(handler, handler->transmitPacket, handler->transmitPacketSize);
                } else {
                    // nbRxBytes
                    _readData(handler, REG_LR_NBRXBYTES, &handler->regBuffer[1], 1);
                    handler->transmitPacketSize = handler->regBuffer[1];
                    if (handler->transmitPacketSize > RF_BUFFER_SIZE) {
                        handler->transmitPacketSize = RF_BUFFER_SIZE;
                    }
                    
                    handler->regBuffer[2] = handler->regBuffer[0];
                    _writeData(handler, REG_LR_FIFOADDRPTR, &handler->regBuffer[2], 1);
                    
                    _readFifo(handler, handler->transmitPacket, handler->transmitPacketSize);
                }
            }
            
            if (handler->setting->RxSingleOn) { // Rx single mode
                // System enter standyby mode by auto
                // _setOperateMode(handler, RFLR_OPMODE_STANDBY);
                handler->currentState = SX1278_STATE_STANDBY;
                
                handler->flag |= SX1278_FLAG_RECV_COMPLETED;
            } else { // Rx continuous mode
                handler->currentState = SX1278_STATE_RX_RUNNING;
            }
            
            rc = RF_RX_DONE;
        } break;
        
        case SX1278_STATE_CAD_INIT: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            // irqFlagsMask
            handler->regBuffer[0] = RFLR_IRQFLAGS_RXTIMEOUT 
                | RFLR_IRQFLAGS_RXDONE
                | RFLR_IRQFLAGS_PAYLOADCRCERROR 
                | RFLR_IRQFLAGS_VALIDHEADER 
                | RFLR_IRQFLAGS_TXDONE 
                // | RFLR_IRQFLAGS_CADDONE 
                | RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL;
                // | RFLR_IRQFLAGS_CADDETECTED;
            _writeData(handler, REG_LR_IRQFLAGSMASK, &handler->regBuffer[0], 1);
            
            // dioMapping1
            handler->regBuffer[0] = 
                RFLR_DIOMAPPING1_DIO0_10      // DIO0-CadDone*
                | RFLR_DIOMAPPING1_DIO1_10    // DIO1-CADDetected*
                | RFLR_DIOMAPPING1_DIO2_00    // DIO2-FhssChangeChannel
                | RFLR_DIOMAPPING1_DIO3_00;   // DIO3-CadDone
            _writeData(handler, REG_LR_DIOMAPPING1, &handler->regBuffer[0], 1);
            // dioMapping2
            handler->regBuffer[0] =    
                RFLR_DIOMAPPING2_DIO4_00      // DIO4-CADDetected
                | RFLR_DIOMAPPING2_DIO5_00;   // DIO5-ModeReady
            _writeData(handler, REG_LR_DIOMAPPING2, &handler->regBuffer[0], 2);
            
            _setOperateMode(handler, RFLR_OPMODE_CAD);
            handler->currentState = SX1278_STATE_CAD_RUNNING;
        } break;
        
        case SX1278_STATE_CAD_RUNNING: {
            // onCADDOne
            handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_0);
            if (handler->regBuffer[0] == 0xFF) {
                _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_CADDONE;
            }
            if (handler->regBuffer[0]) {
                //// Clear Irq
                uint8_t regData = RFLR_IRQFLAGS_CADDONE;
                _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
                
                // onCADDetected
                handler->regBuffer[0] = handler->interface->onReadDIO(handler, SX1278_DIO_1);
                if (handler->regBuffer[0] == 0xFF) {
                    _readData(handler, REG_LR_IRQFLAGS, &handler->regBuffer[0], 1);
                    handler->regBuffer[0] = handler->regBuffer[0] & RFLR_IRQFLAGS_CADDETECTED;
                }
                if (handler->regBuffer[0]) {
                    //// Clear Irq
                    uint8_t regData = RFLR_IRQFLAGS_CADDETECTED;
                    _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
                    
                    // System enter standyby mode by auto
                    // _setOperateMode(handler, RFLR_OPMODE_STANDBY);
                    handler->flag |= SX1278_FLAG_IS_IDLE;
                                  
                    handler->flag |= SX1278_FLAG_CAD_COMPLETED | SX1278_FLAG_CAD_DETECTED;
                    
                    handler->currentState = SX1278_STATE_STANDBY;
                    rc = RF_CHANNEL_ACTIVITY_DETECTED; 
                } else {
                    // System enter standyby mode by auto
                    // _setOperateMode(handler, RFLR_OPMODE_STANDBY);
                    handler->flag |= SX1278_FLAG_IS_IDLE;
                     
                    handler->flag &= ~SX1278_FLAG_CAD_DETECTED;
                    handler->flag |= SX1278_FLAG_CAD_COMPLETED;
                    
                    handler->currentState = SX1278_STATE_STANDBY;
                    rc = RF_CHANNEL_EMPTY;  
                }
            }
        } break;
        
        case SX1278_STATE_TO_STANDBY: {
            _setOperateMode(handler, RFLR_OPMODE_STANDBY);
            
            //// Clear all Irq
            uint8_t regData = 0xFF;
            _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);
            
            handler->flag |= SX1278_FLAG_IS_IDLE;
            
            handler->currentState = SX1278_STATE_STANDBY;
            rc = RF_IDLE;
        } break;
        case SX1278_STATE_TO_SLEEP: {
            _setOperateMode(handler, RFLR_OPMODE_SLEEP);

            //// Clear all Irq
            uint8_t regData = 0xFF;
            _writeData(handler, REG_LR_IRQFLAGS, &regData, 1);

            handler->flag |= SX1278_FLAG_IS_IDLE;

            handler->currentState = SX1278_STATE_SLEEP;
            rc = RF_IDLE;
            break;
        }
    }
    
    return rc;
}


int32_t sx1278_sends(SX1278_t *handler,
        uint8_t *data, uint16_t length, 
        uint32_t timeout) {
    _ASSERT(handler != NULL);
            
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return -1;
    }
    
    if (handler->currentState != SX1278_STATE_STANDBY) {
        return -1;
    }

    handler->flag &= ~SX1278_FLAG_IS_IDLE;
    
    if (length > RF_BUFFER_SIZE) {
        return -1;
    }
    
    if (length != 0) {
        memcpy(handler->transmitPacket, data, length);
    }
    handler->transmitPacketSize = length;
    
    qSTimer_Set(&handler->timeout, timeout);
    handler->currentState = SX1278_STATE_TX_INIT;
    
    return 0;
}


int32_t sx1278_recvs(SX1278_t *handler, 
        SX1278_TransmitOperate_t op,
        uint8_t *data, uint16_t expectedRecvLength, uint16_t *actualRecvLength, 
        uint32_t timeout) {
    _ASSERT(handler != NULL);
            
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return -1;
    }
            
    switch (op) {
        case SX1278_TRANSMIT_OPERATE_START: {
            if (handler->currentState != SX1278_STATE_STANDBY) {
                return -1;
            }
            
            handler->flag &= ~(SX1278_FLAG_IS_IDLE | SX1278_FLAG_RECV_COMPLETED | SX1278_FLAG_RECV_FAILED);
            
            handler->rxPacketTimeout = timeout;
            qSTimer_Set(&handler->timeout, timeout);
            handler->currentState = SX1278_STATE_RX_INIT;
        } break;
        
        case SX1278_TRANSMIT_OPERATE_GET_RESULT_COUNT: {
            if (handler->flag & SX1278_FLAG_RECV_COMPLETED) {
                if (handler->transmitPacketSize == 0) {
                    if (actualRecvLength != NULL) {
                        *actualRecvLength = expectedRecvLength;
                    }
                    
                    return 0;
                }
                
                *actualRecvLength = handler->transmitPacketSize;
            } else {
                *actualRecvLength = 0;
            }
            
            return 0;
        } break;
        
        case SX1278_TRANSMIT_OPERATE_GET_RESULT: {
            if (handler->flag & SX1278_FLAG_RECV_COMPLETED) {
                handler->flag &= ~SX1278_FLAG_RECV_COMPLETED;
                
                if (handler->transmitPacketSize == 0) {
                    if (actualRecvLength != NULL) {
                        *actualRecvLength = 0;
                    }
                    
                    return 0;
                }
                
                if (handler->transmitPacketSize < expectedRecvLength) {
                    expectedRecvLength = handler->transmitPacketSize;
                }
                
                memcpy(data, handler->transmitPacket, expectedRecvLength);
            
                if (actualRecvLength != NULL) {
                    *actualRecvLength = expectedRecvLength;
                }
                
                handler->transmitPacketSize = 0;
                
                return expectedRecvLength;
            } else {
                if (actualRecvLength != NULL) {
                    *actualRecvLength = 0;
                }
                
                return -1;
            }
        } break;
    }
    
    return 0;
}


int32_t sx1278_cadDetect(SX1278_t *handler, SX1278_TransmitOperate_t op) {
    _ASSERT(handler != NULL);
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return -1;
    }
    switch (op) {
        case SX1278_TRANSMIT_OPERATE_START: {
            if (handler->currentState != SX1278_STATE_STANDBY) {
                return -1;
            }
            
            handler->flag &= ~(SX1278_FLAG_IS_IDLE | SX1278_FLAG_CAD_COMPLETED | SX1278_FLAG_CAD_DETECTED);
            handler->currentState = SX1278_STATE_CAD_INIT;
        } break;
        
        case SX1278_TRANSMIT_OPERATE_GET_RESULT: {
            if (handler->flag & SX1278_FLAG_CAD_COMPLETED) {
                handler->flag &= ~(SX1278_FLAG_CAD_COMPLETED);
                
                return handler->flag & SX1278_FLAG_CAD_DETECTED ? 1 : 0;
            } 
        } break;
        
        default:
            break;
    }

    return 0;
}


int32_t sx1278_standby(SX1278_t *handler) {
    _ASSERT(handler != NULL);
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return -1;
    }
    handler->flag &= ~(0x7FFFFFFFu);
    
    handler->currentState = SX1278_STATE_TO_STANDBY;
    return 0;
}


int32_t sx1278_sleep(SX1278_t *handler) {
    _ASSERT(handler != NULL);
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return -1;
    }
    handler->flag &= ~(0x7FFFFFFFu);

    handler->currentState = SX1278_STATE_TO_SLEEP;
    return 0;
}


uint16_t sx1278_getTpayload(SX1278_t *handler, uint8_t payloadLength) {
    _ASSERT(handler != NULL);

    uint16_t tp = 8 + fmax(
            ceil((8 * payloadLength - 4 * handler->setting->SpreadingFactor + 28 + 16 - 20 * 1 /*H=1*/) / (4 * (handler->setting->SpreadingFactor - 2 * 1 /*DE = 1*/)))
                * (handler->setting->ErrorCoding + 4)
        , 0);
    return ((float) tp * handler->calcConfig.tsmValue) + 0.5f;
}


uint16_t sx1278_getTpreamble(SX1278_t *handler) {
    _ASSERT(handler != NULL);
    
    return ((handler->calcConfig.preambleLen + 4.25f) * handler->calcConfig.tsmValue) + 0.5f;
}


uint16_t sx1278_getCadTime(SX1278_t *handler) {
    _ASSERT(handler != NULL);
    
    return handler->calcConfig.cadTime;
}


void sx1278_setFreq(SX1278_t *handler, uint32_t freq) {
    _ASSERT(handler != NULL);
    if (handler->flag & SX1278_FLAG_HAL_FAILED) {
        return;
    }
    _setRFFrequency(handler, freq);
}


uint8_t sx1278_isReady(SX1278_t *handler) {
    return handler->flag & SX1278_FLAG_HAL_FAILED ? 0 : 1;
}

/*@}*/
