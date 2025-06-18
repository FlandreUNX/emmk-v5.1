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
 
#include "emmk-config.h"
#include "emmk-driver.h"
#include "./aw8623.h"

/**
 * @addtogroup ProgramProfile
 * @note none
 */
 
/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define ADDRESS  0x5A

//#define AW8623_REG_ID            0x00
//#define AW8623_REG_SYSST         0x01
//#define AW8623_REG_SYSINT        0x02
//#define AW8623_REG_SYSINTM       0x03
//#define AW8623_REG_SYSCTRL       0x04
//#define AW8623_REG_GO            0x05
//#define AW8623_REG_RTP_DATA      0x06
//#define AW8623_REG_WAVSEQ1       0x07
//#define AW8623_REG_WAVSEQ2       0x08
//#define AW8623_REG_WAVSEQ3       0x09
//#define AW8623_REG_WAVSEQ4       0x0a
//#define AW8623_REG_WAVSEQ5       0x0b
//#define AW8623_REG_WAVSEQ6       0x0c
//#define AW8623_REG_WAVSEQ7       0x0d
//#define AW8623_REG_WAVSEQ8       0x0e
//#define AW8623_REG_WAVLOOP1      0x0f
//#define AW8623_REG_WAVLOOP2      0x10
//#define AW8623_REG_WAVLOOP3      0x11
//#define AW8623_REG_WAVLOOP4      0x12
//#define AW8623_REG_MAIN_LOOP     0x13
//#define AW8623_REG_TRG1_SEQP    0x14
//#define AW8623_REG_TRG1_SEQN    0x17
//#define AW8623_REG_PLAY_PRIO      0x1a 
//#define AW8623_REG_TRG_CFG1      0x1b
//#define AW8623_REG_TRG_CFG2      0x1c
//#define AW8623_REG_DBGCTRL       0x20
//#define AW8623_REG_BASE_ADDRH    0x21
//#define AW8623_REG_BASE_ADDRL    0x22
//#define AW8623_REG_FIFO_AEH      0x23
//#define AW8623_REG_FIFO_AEL      0x24
//#define AW8623_REG_FIFO_AFH      0x25
//#define AW8623_REG_FIFO_AFL      0x26
//#define AW8623_REG_WAKE_DLY      0x27
//#define AW8623_REG_START_DLY     0x28
//#define AW8623_REG_END_DLY_H     0x29
//#define AW8623_REG_END_DLY_L     0x2a
//#define AW8623_REG_DATCTRL       0x2b
//#define AW8623_REG_PWMDEL       0x2c
//#define AW8623_REG_PWMPRC        0x2d
//#define AW8623_REG_PWMDBG        0x2e
//#define AW8623_REG_LDOCTRL        0x2f
//#define AW8623_REG_DBGSTAT       0x30
//#define AW8623_REG_WAVECTRL       0x31
//#define AW8623_REG_BRAKE0_CTRL       0x32 
//#define AW8623_REG_BRAKE1_CTRL       0x33 
//#define AW8623_REG_BRAKE2_CTRL       0x34
//#define AW8623_REG_BRAKE_NUM        0x35
//#define AW8623_REG_ANADBG1       0x36
//#define AW8623_REG_ANADBG2        0x37
//#define AW8623_REG_ANACTRL        0x38
//#define AW8623_REG_SW_BRAKE        0x39
//#define AW8623_REG_GLBDBG       0x3a
//#define AW8623_REG_DATDBG       0x3b
//#define AW8623_REG_WDCTRL       0x3c
//#define AW8623_REG_HDRVDBG       0x3d
//#define AW8623_REG_PRLVL         0x3e
//#define AW8623_REG_PRTIME        0x3f
//#define AW8623_REG_RAMADDRH      0x40
//#define AW8623_REG_RAMADDRL      0x41
//#define AW8623_REG_RAMDATA       0x42
//#define AW8623_REG_TM       0x43
//#define AW8623_REG_BRA_MAX_NUM      0x44
//#define AW8623_REG_BEMF_ERM_FAC       0x45
//#define AW8623_REG_BEMF_BRA_FAC       0x46
//#define AW8623_REG_GLB_STATE       0x47
//#define AW8623_REG_CONT_CTRL     0x48
//#define AW8623_REG_F_PRE_H       0x49
//#define AW8623_REG_F_PRE_L       0x4a
//#define AW8623_REG_TD_H          0x4b
//#define AW8623_REG_TD_L          0x4c
//#define AW8623_REG_TSET          0x4d
//#define AW8623_REG_THRS_BRA_RAP   0x4e
//#define AW8623_REG_THRS_BRA_END   0x4f
//#define AW8623_REG_EF_CTRL     0x50
//#define AW8623_REG_EF_WDATAH      0x53
//#define AW8623_REG_EF_WDATAL     0x54
//#define AW8623_REG_EF_RDATAH     0x55
//#define AW8623_REG_EF_RDATAL     0x56
//#define AW8623_REG_DLY                  0x57
//#define AW8623_REG_EF_WR_WIDTH      0x58
//#define AW8623_REG_EF_RD_WIDTH      0x59
//#define AW8623_REG_TRIM_LRA      0x5b
//#define AW8623_REG_TRIM_OSC      0x5c
//#define AW8623_REG_R_SPARE      0x5d
//#define AW8623_REG_D2SCFG        0x5e
//#define AW8623_REG_DETCTRL       0x5f
//#define AW8623_REG_RLDET         0x60
//#define AW8623_REG_OSDET         0x61
//#define AW8623_REG_VBATDET       0x62
//#define AW8623_REG_TESTDET       0x63
//#define AW8623_REG_DETLO       0x64
//#define AW8623_REG_BEMFDBG       0x65
//#define AW8623_REG_ADCTEST       0x66
//#define AW8623_REG_BEMFTEST      0x67
//#define AW8623_REG_F_LRA_F0_H    0x68
//#define AW8623_REG_F_LRA_F0_L    0x69
//#define AW8623_REG_F_LRA_CONT_H  0x6a
//#define AW8623_REG_F_LRA_CONT_L  0x6b
//#define AW8623_REG_WAIT_VOL_MP   0x6e 
//#define AW8623_REG_WAIT_VOL_MN   0x6f
//#define AW8623_REG_BEMF_VOL_H    0x70
//#define AW8623_REG_BEMF_VOL_L    0x71
//#define AW8623_REG_ZC_THRSH_H    0x72
//#define AW8623_REG_ZC_THRSH_L    0x73
//#define AW8623_REG_BEMF_VTHH_H   0x74
//#define AW8623_REG_BEMF_VTHH_L   0x75
//#define AW8623_REG_BEMF_VTHL_H   0x76
//#define AW8623_REG_BEMF_VTHL_L   0x77
//#define AW8623_REG_BEMF_NUM      0x78
//#define AW8623_REG_DRV_TIME      0x79
//#define AW8623_REG_TIME_NZC      0x7a
//#define AW8623_REG_DRV_LVL       0x7b
//#define AW8623_REG_DRV_LVL_OV    0x7c
//#define AW8623_REG_NUM_F0_1      0x7d
//#define AW8623_REG_NUM_F0_2      0x7e
//#define AW8623_REG_NUM_F0_3      0x7f


///******************************************************
// * Register Detail
// *****************************************************/
//// SYSST  0x01
//#define AW8623_BIT_SYSST_OVS                     (1<<6)
//#define AW8623_BIT_SYSST_UVLS                    (1<<5)
//#define AW8623_BIT_SYSST_FF_AES                  (1<<4)
//#define AW8623_BIT_SYSST_FF_AFS                  (1<<3)
//#define AW8623_BIT_SYSST_OCDS                    (1<<2)
//#define AW8623_BIT_SYSST_OTS                     (1<<1)
//#define AW8623_BIT_SYSST_DONES                   (1<<0)

//// SYSINT  0x02
//#define AW8623_BIT_SYSINT_OVI                     (1<<6)
//#define AW8623_BIT_SYSINT_UVLI                   (1<<5)
//#define AW8623_BIT_SYSINT_FF_AEI                 (1<<4)
//#define AW8623_BIT_SYSINT_FF_AFI                 (1<<3)
//#define AW8623_BIT_SYSINT_OCDI                   (1<<2)
//#define AW8623_BIT_SYSINT_OTI                    (1<<1)
//#define AW8623_BIT_SYSINT_DONEI                  (1<<0)

//// SYSINTM 0x03
//#define AW8623_BIT_SYSINTM_OV_MASK               (~(1<<6))
//#define AW8623_BIT_SYSINTM_OV_OFF                (1<<6)
//#define AW8623_BIT_SYSINTM_OV_EN                 (0<<6)
//#define AW8623_BIT_SYSINTM_UVLO_MASK             (~(1<<5))
//#define AW8623_BIT_SYSINTM_UVLO_OFF              (1<<5)
//#define AW8623_BIT_SYSINTM_UVLO_EN               (0<<5)
//#define AW8623_BIT_SYSINTM_FF_AE_MASK            (~(1<<4))
//#define AW8623_BIT_SYSINTM_FF_AE_OFF             (1<<4)
//#define AW8623_BIT_SYSINTM_FF_AE_EN              (0<<4)
//#define AW8623_BIT_SYSINTM_FF_AF_MASK            (~(1<<3))
//#define AW8623_BIT_SYSINTM_FF_AF_OFF             (1<<3)
//#define AW8623_BIT_SYSINTM_FF_AF_EN              (0<<3)
//#define AW8623_BIT_SYSINTM_OCD_MASK              (~(1<<2))
//#define AW8623_BIT_SYSINTM_OCD_OFF               (1<<2)
//#define AW8623_BIT_SYSINTM_OCD_EN                (0<<2)
//#define AW8623_BIT_SYSINTM_OT_MASK               (~(1<<1))
//#define AW8623_BIT_SYSINTM_OT_OFF                (1<<1)
//#define AW8623_BIT_SYSINTM_OT_EN                 (0<<1)
//#define AW8623_BIT_SYSINTM_DONE_MASK             (~(1<<0))
//#define AW8623_BIT_SYSINTM_DONE_OFF              (1<<0)
//#define AW8623_BIT_SYSINTM_DONE_EN               (0<<0)

//// SYSCTRL 0x04
//#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_MASK      (~(3<<6))
//#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_4X        (3<<6)
//#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_2X        (0<<6)
//#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_1X        (1<<6)
//#define AW8623_BIT_SYSCTRL_RAMINIT_MASK          (~(1<<5))
//#define AW8623_BIT_SYSCTRL_RAMINIT_EN            (1<<5)
//#define AW8623_BIT_SYSCTRL_RAMINIT_OFF           (0<<5)
//#define AW8623_BIT_SYSCTRL_PLAY_MODE_MASK        (~(3<<2))
//#define AW8623_BIT_SYSCTRL_PLAY_MODE_CONT        (2<<2)
//#define AW8623_BIT_SYSCTRL_PLAY_MODE_RTP         (1<<2)
//#define AW8623_BIT_SYSCTRL_PLAY_MODE_RAM         (0<<2)
//#define AW8623_BIT_SYSCTRL_WORK_MODE_MASK        (~(1<<0))
//#define AW8623_BIT_SYSCTRL_STANDBY               (1<<0)
//#define AW8623_BIT_SYSCTRL_ACTIVE                (0<<0)

//// GO 0x05
//#define AW8623_BIT_GO_MASK                       (~(1<<0))
//#define AW8623_BIT_GO_ENABLE                     (1<<0)
//#define AW8623_BIT_GO_DISABLE                    (0<<0)

//// WAVSEQ1 0x07
//#define AW8623_BIT_WAVSEQ1_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ1_WAV_FRM_SEQ1_MASK     (~(127<<0))

//// WAVSEQ2 0x08
//#define AW8623_BIT_WAVSEQ2_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ2_WAV_FRM_SEQ2_MASK     (~(127<<0))

//// WAVSEQ3 0x09
//#define AW8623_BIT_WAVSEQ3_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ3_WAV_FRM_SEQ3_MASK     (~(127<<0))

//// WAVSEQ4 0x0A
//#define AW8623_BIT_WAVSEQ4_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ4_WAV_FRM_SEQ4_MASK     (~(127<<0))

//// WAVSEQ5 0X0B
//#define AW8623_BIT_WAVSEQ5_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ5_WAV_FRM_SEQ5_MASK     (~(127<<0))

//// WAVSEQ6 0X0C
//#define AW8623_BIT_WAVSEQ6_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ6_WAV_FRM_SEQ6_MASK     (~(127<<0))

//// WAVSEQ7 
//#define AW8623_BIT_WAVSEQ7_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ7_WAV_FRM_SEQ7_MASK     (~(127<<0))

//// WAVSEQ8
//#define AW8623_BIT_WAVSEQ8_WAIT                  (1<<7)
//#define AW8623_BIT_WAVSEQ8_WAV_FRM_SEQ8_MASK     (~(127<<0))

//// WAVLOOP
//#define AW8623_BIT_WAVLOOP_SEQN_MASK            (~(15<<4))
//#define AW8623_BIT_WAVLOOP_SEQNP1_MASK          (~(15<<0))
//#define AW8623_BIT_WAVLOOP_INIFINITELY          (15<<0)

//// WAVLOOP1
//#define AW8623_BIT_WAVLOOP1_SEQ1_MASK            (~(15<<4))
//#define AW8623_BIT_WAVLOOP1_SEQ2_MASK            (~(15<<0))

//// WAVLOOP2
//#define AW8623_BIT_WAVLOOP2_SEQ3_MASK            (~(15<<4))
//#define AW8623_BIT_WAVLOOP2_SEQ4_MASK            (~(15<<0))

//// WAVLOOP3
//#define AW8623_BIT_WAVLOOP3_SEQ5_MASK            (~(15<<4))
//#define AW8623_BIT_WAVLOOP3_SEQ6_MASK            (~(15<<0))

//// WAVLOOP4
//#define AW8623_BIT_WAVLOOP4_SEQ7_MASK            (~(15<<4))
//#define AW8623_BIT_WAVLOOP4_SEQ8_MASK            (~(15<<0))


//// PLAYPRIO
//#define AW8623_BIT_PLAYPRIO_GO_MASK              (~(3<<6))
//#define AW8623_BIT_PLAYPRIO_TRIG3_MASK           (~(3<<4))
//#define AW8623_BIT_PLAYPRIO_TRIG2_MASK           (~(3<<2))
//#define AW8623_BIT_PLAYPRIO_TRIG1_MASK           (~(3<<0))

//// TRGCFG1
//#define AW8623_BIT_TRGCFG1_TRG3_POLAR_MASK       (~(1<<5))
//#define AW8623_BIT_TRGCFG1_TRG3_POLAR_NEG        (1<<5)
//#define AW8623_BIT_TRGCFG1_TRG3_POLAR_POS        (0<<5)
//#define AW8623_BIT_TRGCFG1_TRG3_EDGE_MASK        (~(1<<4))
//#define AW8623_BIT_TRGCFG1_TRG3_EDGE_POS         (1<<4)
//#define AW8623_BIT_TRGCFG1_TRG3_EDGE_POS_NEG     (0<<4)
//#define AW8623_BIT_TRGCFG1_TRG2_POLAR_MASK       (~(1<<3))
//#define AW8623_BIT_TRGCFG1_TRG2_POLAR_NEG        (1<<3)
//#define AW8623_BIT_TRGCFG1_TRG2_POLAR_POS        (0<<3)
//#define AW8623_BIT_TRGCFG1_TRG2_EDGE_MASK        (~(1<<2))
//#define AW8623_BIT_TRGCFG1_TRG2_EDGE_POS         (1<<2)
//#define AW8623_BIT_TRGCFG1_TRG2_EDGE_POS_NEG     (0<<2)
//#define AW8623_BIT_TRGCFG1_TRG1_POLAR_MASK       (~(1<<1))
//#define AW8623_BIT_TRGCFG1_TRG1_POLAR_NEG        (1<<1)
//#define AW8623_BIT_TRGCFG1_TRG1_POLAR_POS        (0<<1)
//#define AW8623_BIT_TRGCFG1_TRG1_EDGE_MASK        (~(1<<0))
//#define AW8623_BIT_TRGCFG1_TRG1_EDGE_POS         (1<<0)
//#define AW8623_BIT_TRGCFG1_TRG1_EDGE_POS_NEG     (0<<0)

//// TRGCFG2
//#define AW8623_BIT_TRGCFG2_TRG3_ENABLE_MASK      (~(1<<2))
//#define AW8623_BIT_TRGCFG2_TRG3_ENABLE           (1<<2)
//#define AW8623_BIT_TRGCFG2_TRG3_DISABLE          (0<<2)
//#define AW8623_BIT_TRGCFG2_TRG2_ENABLE_MASK      (~(1<<1))
//#define AW8623_BIT_TRGCFG2_TRG2_ENABLE           (1<<1)
//#define AW8623_BIT_TRGCFG2_TRG2_DISABLE          (0<<1)
//#define AW8623_BIT_TRGCFG2_TRG1_ENABLE_MASK      (~(1<<0))
//#define AW8623_BIT_TRGCFG2_TRG1_ENABLE           (1<<0)
//#define AW8623_BIT_TRGCFG2_TRG1_DISABLE          (0<<0)

////DBGCTRL //0X20
//#define AW8623_BIT_DBGCTRL_INTN_TRG_SEL_MASK        (~(1<<5))
//#define AW8623_BIT_DBGCTRL_INTN_SEL_ENABLE           (1<<5)
//#define AW8623_BIT_DBGCTRL_TRG_SEL_ENABLE           (0<<5)

//// DATCTRL
//#define AW8623_BIT_DATCTRL_FC_MASK              (~(1<<6))
//#define AW8623_BIT_DATCTRL_FC_1000HZ            (3<<6)
//#define AW8623_BIT_DATCTRL_FC_800HZ             (3<<6)
//#define AW8623_BIT_DATCTRL_FC_600HZ             (1<<6)
//#define AW8623_BIT_DATCTRL_FC_400HZ             (0<<6)
//#define AW8623_BIT_DATCTRL_LPF_ENABLE_MASK      (~(1<<5))
//#define AW8623_BIT_DATCTRL_LPF_ENABLE           (1<<5)
//#define AW8623_BIT_DATCTRL_LPF_DISABLE          (0<<5)

////PWMPRC //0X2D
//#define AW8623_BIT_PWMPRC_PRC_EN_MASK          (~(1<<7))
//#define AW8623_BIT_PWMPRC_PRC_ENABLE                (1<<7)
//#define AW8623_BIT_PWMPRC_PRC_DISABLE               (0<<7)
//#define AW8623_BIT_PWMPRC_PRCTIME_MASK       (~(0x7f<<0))

//// PWMDBG
//#define AW8623_BIT_PWMDBG_PWM_MODE_MASK          (~(3<<5))
//#define AW8623_BIT_PWMDBG_PWM_12K                (3<<5)
//#define AW8623_BIT_PWMDBG_PWM_24K                (2<<5)
//#define AW8623_BIT_PWMDBG_PWM_48K                (0<<5)

//// WAVECTRL
//#define AW8623_BIT_WAVECTRL_NUM_OV_DRIVER_MASK          (~(0xF<<4))
//#define AW8623_BIT_WAVECTRL_NUM_OV_DRIVER                (0<<4)

//// BST_AUTO
//#define AW8623_BIT_BST_AUTO_BST_AUTOSW_MASK      (~(1<<2))
//#define AW8623_BIT_BST_AUTO_BST_AUTOMATIC_BOOST    (1<<2)
//#define AW8623_BIT_BST_AUTO_BST_MANUAL_BOOST   (0<<2)

//// CONT_CTRL
//#define AW8623_BIT_CONT_CTRL_ZC_DETEC_MASK       (~(1<<7))
//#define AW8623_BIT_CONT_CTRL_ZC_DETEC_ENABLE     (1<<7)
//#define AW8623_BIT_CONT_CTRL_ZC_DETEC_DISABLE    (0<<7)
//#define AW8623_BIT_CONT_CTRL_WAIT_PERIOD_MASK    (~(3<<5))
//#define AW8623_BIT_CONT_CTRL_WAIT_8PERIOD        (3<<5)
//#define AW8623_BIT_CONT_CTRL_WAIT_4PERIOD        (2<<5)
//#define AW8623_BIT_CONT_CTRL_WAIT_2PERIOD        (1<<5)
//#define AW8623_BIT_CONT_CTRL_WAIT_1PERIOD        (0<<5)
//#define AW8623_BIT_CONT_CTRL_MODE_MASK           (~(1<<4))
//#define AW8623_BIT_CONT_CTRL_BY_DRV_TIME         (1<<4)
//#define AW8623_BIT_CONT_CTRL_BY_GO_SIGNAL        (0<<4)
//#define AW8623_BIT_CONT_CTRL_EN_CLOSE_MASK       (~(1<<3))
//#define AW8623_BIT_CONT_CTRL_CLOSE_PLAYBACK      (1<<3)
//#define AW8623_BIT_CONT_CTRL_OPEN_PLAYBACK       (0<<3)
//#define AW8623_BIT_CONT_CTRL_F0_DETECT_MASK      (~(1<<2))
//#define AW8623_BIT_CONT_CTRL_F0_DETECT_ENABLE    (1<<2)
//#define AW8623_BIT_CONT_CTRL_F0_DETECT_DISABLE   (0<<2)
//#define AW8623_BIT_CONT_CTRL_O2C_MASK            (~(1<<1))
//#define AW8623_BIT_CONT_CTRL_O2C_ENABLE          (1<<1)
//#define AW8623_BIT_CONT_CTRL_O2C_DISABLE         (0<<1)
//#define AW8623_BIT_CONT_CTRL_AUTO_BRK_MASK       (~(1<<0))
//#define AW8623_BIT_CONT_CTRL_AUTO_BRK_ENABLE     (1<<0)
//#define AW8623_BIT_CONT_CTRL_AUTO_BRK_DISABLE    (0<<0)

//#define AW8623_BIT_D2SCFG_CLK_ADC_MASK  (~(7<<4))
//#define AW8623_BIT_D2SCFG_CLK_ASC_1P5MHZ     (3<<4)

//// DETCTRL
//#define AW8623_BIT_DETCTRL_RL_OS_MASK            (~(1<<6))
//#define AW8623_BIT_DETCTRL_RL_DETECT             (1<<6)
//#define AW8623_BIT_DETCTRL_OS_DETECT             (0<<6)
//#define AW8623_BIT_DETCTRL_PROTECT_MASK          (~(1<<5))
//#define AW8623_BIT_DETCTRL_PROTECT_NO_ACTION     (1<<5)
//#define AW8623_BIT_DETCTRL_PROTECT_SHUTDOWN      (0<<5)
//#define AW8623_BIT_DETCTRL_VBAT_GO_MASK          (~(1<<1))
//#define AW8623_BIT_DETCTRL_VABT_GO_ENABLE        (1<<1)
//#define AW8623_BIT_DETCTRL_VBAT_GO_DISBALE       (0<<1)
//#define AW8623_BIT_DETCTRL_DIAG_GO_MASK          (~(1<<0))
//#define AW8623_BIT_DETCTRL_DIAG_GO_ENABLE        (1<<0)
//#define AW8623_BIT_DETCTRL_DIAG_GO_DISABLE       (0<<0)


//// VBAT MODE
//#define AW8623_BIT_DETCTRL_VBAT_MODE_MASK        (~(1<<6))
//#define AW8623_BIT_DETCTRL_VBAT_HW_COMP          (1<<6)
//#define AW8623_BIT_DETCTRL_VBAT_SW_COMP          (0<<6)


//// ANACTRL
//#define AW8623_BIT_ANACTRL_LRA_SRC_MASK         (~(1<<5))
//#define AW8623_BIT_ANACTRL_LRA_SRC_REG          (1<<5)
//#define AW8623_BIT_ANACTRL_LRA_SRC_EFUSE        (0<<5)
//#define AW8623_BIT_ANACTRL_EN_IO_PD1_MASK         (~(1<<0))
//#define AW8623_BIT_ANACTRL_EN_IO_PD1_HIGH         (1<<0)
//#define AW8623_BIT_ANACTRL_EN_IO_PD1_LOW         (0<<0)

////SW_BRAKE
//#define AW8623_BIT_EN_BRAKE_CONT_MASK              (~(1<<3))
//#define AW8623_BIT_EN_BRAKE_CONT_ENABLE          (1<<3)
//#define AW8623_BIT_EN_BRAKE_CONT_DISABLE        (0<<3)
//#define AW8623_BIT_EN_BRAKE_RAM_MASK              (~(1<<2))
//#define AW8623_BIT_EN_BRAKE_RAM_ENABLE          (1<<2)
//#define AW8623_BIT_EN_BRAKE_RAM_DISABLE        (0<<2)
//#define AW8623_BIT_EN_BRAKE_RTP_MASK              (~(1<<1))
//#define AW8623_BIT_EN_BRAKE_RTP_ENABLE          (1<<1)
//#define AW8623_BIT_EN_BRAKE_RTP_DISABLE        (0<<1)
//#define AW8623_BIT_EN_BRAKE_TRIG_MASK              (~(1<<0))
//#define AW8623_BIT_EN_BRAKE_TRIG_ENABLE          (1<<0)
//#define AW8623_BIT_EN_BRAKE_TRIG_DISABLE        (0<<0)

////PRLVL
//#define AW8623_BIT_PRLVL_PR_EN_MASK         (~(1<<7))
//#define AW8623_BIT_PRLVL_PR_ENABLE          (1<<7)
//#define AW8623_BIT_PRLVL_PR_DISABLE          (0<<7)
//#define AW8623_BIT_PRLVL_PRLVL_MASK       (~(0x7f<<0))

////PRTIME
//#define AW8623_BIT_PRTIME_PRTIME_MASK       (~(0xff<<0))

//#define AW8623_BIT_BEMF_NUM_BRK_MASK            (~(0xf<<0))

//#define AW8623_SEQUENCER_SIZE               8
//#define AW8623_SEQUENCER_LOOP_SIZE          4

//#define AW8623_RTP_I2C_SINGLE_MAX_NUM       512

//#define HAPTIC_MAX_TIMEOUT                  10000

//#define AW8623_HAPTIC_F0_PRE                2350
//#define AW8623_HAPTIC_F0_CALI_PERCEN        7       //-7%~7%
//#define AW8623_HAPTIC_CONT_DRV_LVL          125 //80 //125=2.98v
//#define AW8623_HAPTIC_CONT_DRV_LVL_OV       155 //  127  //155=3.69v
//#define AW8623_HAPTIC_CONT_TD               0xf06c  //0x005d
//#define AW8623_HAPTIC_CONT_ZC_THR           0x0ff1 // 0x009a
//#define AW8623_HAPTIC_CONT_NUM_BRK          3
//#define AW8623_HAPTIC_F0_COEFF              260    //2.604167

/********************************************
 * Register List
 *******************************************/
#define AW8623_REG_ID			0x00
#define AW8623_REG_SYSST		0x01
#define AW8623_REG_SYSINT		0x02
#define AW8623_REG_SYSINTM		0x03
#define AW8623_REG_SYSCTRL		0x04
#define AW8623_REG_GO			0x05
#define AW8623_REG_RTP_DATA		0x06
#define AW8623_REG_WAVSEQ1		0x07
#define AW8623_REG_WAVSEQ2		0x08
#define AW8623_REG_WAVSEQ3		0x09
#define AW8623_REG_WAVSEQ4		0x0a
#define AW8623_REG_WAVSEQ5		0x0b
#define AW8623_REG_WAVSEQ6		0x0c
#define AW8623_REG_WAVSEQ7		0x0d
#define AW8623_REG_WAVSEQ8		0x0e
#define AW8623_REG_WAVLOOP1		0x0f
#define AW8623_REG_WAVLOOP2		0x10
#define AW8623_REG_WAVLOOP3		0x11
#define AW8623_REG_WAVLOOP4		0x12
#define AW8623_REG_MAIN_LOOP		0x13
#define AW8623_REG_TRG1_SEQP		0x14
#define AW8623_REG_TRG1_SEQN		0x17
#define AW8623_REG_PLAY_PRIO		0x1a
#define AW8623_REG_TRG_CFG1		0x1b
#define AW8623_REG_TRG_CFG2		0x1c
#define AW8623_REG_DBGCTRL		0x20
#define AW8623_REG_BASE_ADDRH		0x21
#define AW8623_REG_BASE_ADDRL		0x22
#define AW8623_REG_FIFO_AEH		0x23
#define AW8623_REG_FIFO_AEL		0x24
#define AW8623_REG_FIFO_AFH		0x25
#define AW8623_REG_FIFO_AFL		0x26
#define AW8623_REG_WAKE_DLY		0x27
#define AW8623_REG_START_DLY		0x28
#define AW8623_REG_END_DLY_H		0x29
#define AW8623_REG_END_DLY_L		0x2a
#define AW8623_REG_DATCTRL		0x2b
#define AW8623_REG_PWMDEL		0x2c
#define AW8623_REG_PWMPRC		0x2d
#define AW8623_REG_PWMDBG		0x2e
#define AW8623_REG_LDOCTRL		0x2f
#define AW8623_REG_DBGSTAT		0x30
#define AW8623_REG_WAVECTRL		0x31
#define AW8623_REG_BRAKE0_CTRL		0x32
#define AW8623_REG_BRAKE1_CTRL		0x33
#define AW8623_REG_BRAKE2_CTRL		0x34
#define AW8623_REG_BRAKE_NUM		0x35
#define AW8623_REG_ANADBG1		0x36
#define AW8623_REG_ANADBG2		0x37
#define AW8623_REG_ANACTRL		0x38
#define AW8623_REG_SW_BRAKE		0x39
#define AW8623_REG_GLBDBG		0x3a
#define AW8623_REG_DATDBG		0x3b
#define AW8623_REG_WDCTRL		0x3c
#define AW8623_REG_HDRVDBG		0x3d
#define AW8623_REG_PRLVL		0x3e
#define AW8623_REG_PRTIME		0x3f
#define AW8623_REG_RAMADDRH		0x40
#define AW8623_REG_RAMADDRL		0x41
#define AW8623_REG_RAMDATA		0x42
#define AW8623_REG_TM			0x43
#define AW8623_REG_BRA_MAX_NUM		0x44
#define AW8623_REG_BEMF_ERM_FAC		0x45
#define AW8623_REG_BEMF_BRA_FAC		0x46
#define AW8623_REG_GLB_STATE		0x47
#define AW8623_REG_CONT_CTRL		0x48
#define AW8623_REG_F_PRE_H		0x49
#define AW8623_REG_F_PRE_L		0x4a
#define AW8623_REG_TD_H			0x4b
#define AW8623_REG_TD_L			0x4c
#define AW8623_REG_TSET			0x4d
#define AW8623_REG_THRS_BRA_RAP		0x4e
#define AW8623_REG_THRS_BRA_END		0x4f
#define AW8623_REG_EF_CTRL		0x50
#define AW8623_REG_EF_WDATAH		0x53
#define AW8623_REG_EF_WDATAL		0x54
#define AW8623_REG_EF_RDATAH		0x55
#define AW8623_REG_EF_RDATAL		0x56
#define AW8623_REG_DLY			0x57
#define AW8623_REG_EF_WR_WIDTH		0x58
#define AW8623_REG_EF_RD_WIDTH		0x59
#define AW8623_REG_TRIM_LRA		0x5b
#define AW8623_REG_TRIM_OSC		0x5c
#define AW8623_REG_R_SPARE		0x5d
#define AW8623_REG_D2SCFG		0x5e
#define AW8623_REG_DETCTRL		0x5f
#define AW8623_REG_RLDET		0x60
#define AW8623_REG_OSDET		0x61
#define AW8623_REG_VBATDET		0x62
#define AW8623_REG_TESTDET		0x63
#define AW8623_REG_DETLO		0x64
#define AW8623_REG_BEMFDBG		0x65
#define AW8623_REG_ADCTEST		0x66
#define AW8623_REG_BEMFTEST		0x67
#define AW8623_REG_F_LRA_F0_H		0x68
#define AW8623_REG_F_LRA_F0_L		0x69
#define AW8623_REG_F_LRA_CONT_H		0x6a
#define AW8623_REG_F_LRA_CONT_L		0x6b
#define AW8623_REG_WAIT_VOL_MP		0x6e
#define AW8623_REG_WAIT_VOL_MN		0x6f
#define AW8623_REG_BEMF_VOL_H		0x70
#define AW8623_REG_BEMF_VOL_L		0x71
#define AW8623_REG_ZC_THRSH_H		0x72
#define AW8623_REG_ZC_THRSH_L		0x73
#define AW8623_REG_BEMF_VTHH_H		0x74
#define AW8623_REG_BEMF_VTHH_L		0x75
#define AW8623_REG_BEMF_VTHL_H		0x76
#define AW8623_REG_BEMF_VTHL_L		0x77
#define AW8623_REG_BEMF_NUM		0x78
#define AW8623_REG_DRV_TIME		0x79
#define AW8623_REG_TIME_NZC		0x7a
#define AW8623_REG_DRV_LVL		0x7b
#define AW8623_REG_DRV_LVL_OV		0x7c
#define AW8623_REG_NUM_F0_1		0x7d
#define AW8623_REG_NUM_F0_2		0x7e
#define AW8623_REG_NUM_F0_3		0x7f



/********************************************
 * Register Access
 *******************************************/
#define REG_NONE_ACCESS		0
#define REG_RD_ACCESS		(1 << 0)
#define REG_WR_ACCESS			(1 << 1)

/******************************************************
 * Register Detail
 *****************************************************/
 /* SYSST  0x01 */
#define AW8623_BIT_SYSST_OVS				(1<<6)
#define AW8623_BIT_SYSST_UVLS				(1<<5)
#define AW8623_BIT_SYSST_FF_AES				(1<<4)
#define AW8623_BIT_SYSST_FF_AFS				(1<<3)
#define AW8623_BIT_SYSST_OCDS				(1<<2)
#define AW8623_BIT_SYSST_OTS				(1<<1)
#define AW8623_BIT_SYSST_DONES				(1<<0)

 /* SYSINT  0x02 */
#define AW8623_BIT_SYSINT_OVI				(1<<6)
#define AW8623_BIT_SYSINT_UVLI				(1<<5)
#define AW8623_BIT_SYSINT_FF_AEI			(1<<4)
#define AW8623_BIT_SYSINT_FF_AFI			(1<<3)
#define AW8623_BIT_SYSINT_OCDI				(1<<2)
#define AW8623_BIT_SYSINT_OTI				(1<<1)
#define AW8623_BIT_SYSINT_DONEI				(1<<0)

 /* SYSINTM 0x03 */
#define AW8623_BIT_SYSINTM_OV_MASK			(~(1<<6))
#define AW8623_BIT_SYSINTM_OV_OFF			(1<<6)
#define AW8623_BIT_SYSINTM_OV_EN			(0<<6)
#define AW8623_BIT_SYSINTM_UVLO_MASK			(~(1<<5))
#define AW8623_BIT_SYSINTM_UVLO_OFF			(1<<5)
#define AW8623_BIT_SYSINTM_UVLO_EN			(0<<5)
#define AW8623_BIT_SYSINTM_FF_AE_MASK			(~(1<<4))
#define AW8623_BIT_SYSINTM_FF_AE_OFF			(1<<4)
#define AW8623_BIT_SYSINTM_FF_AE_EN			(0<<4)
#define AW8623_BIT_SYSINTM_FF_AF_MASK			(~(1<<3))
#define AW8623_BIT_SYSINTM_FF_AF_OFF			(1<<3)
#define AW8623_BIT_SYSINTM_FF_AF_EN			(0<<3)
#define AW8623_BIT_SYSINTM_OCD_MASK			(~(1<<2))
#define AW8623_BIT_SYSINTM_OCD_OFF			(1<<2)
#define AW8623_BIT_SYSINTM_OCD_EN			(0<<2)
#define AW8623_BIT_SYSINTM_OT_MASK			(~(1<<1))
#define AW8623_BIT_SYSINTM_OT_OFF			(1<<1)
#define AW8623_BIT_SYSINTM_OT_EN			(0<<1)
#define AW8623_BIT_SYSINTM_DONE_MASK			(~(1<<0))
#define AW8623_BIT_SYSINTM_DONE_OFF			(1<<0)
#define AW8623_BIT_SYSINTM_DONE_EN			(0<<0)

 /* SYSCTRL 0x04 */
#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_MASK		(~(3<<6))
#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_4X		(3<<6)
#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_2X		(0<<6)
#define AW8623_BIT_SYSCTRL_WAVDAT_MODE_1X		(1<<6)
#define AW8623_BIT_SYSCTRL_RAMINIT_MASK			(~(1<<5))
#define AW8623_BIT_SYSCTRL_RAMINIT_EN			(1<<5)
#define AW8623_BIT_SYSCTRL_RAMINIT_OFF			(0<<5)
#define AW8623_BIT_SYSCTRL_PLAY_MODE_MASK		(~(3<<2))
#define AW8623_BIT_SYSCTRL_PLAY_MODE_CONT		(2<<2)
#define AW8623_BIT_SYSCTRL_PLAY_MODE_RTP		(1<<2)
#define AW8623_BIT_SYSCTRL_PLAY_MODE_RAM		(0<<2)
#define AW8623_BIT_SYSCTRL_WORK_MODE_MASK		(~(1<<0))
#define AW8623_BIT_SYSCTRL_STANDBY			(1<<0)
#define AW8623_BIT_SYSCTRL_ACTIVE			(0<<0)

 /* GO 0x05 */
#define AW8623_BIT_GO_MASK				(~(1<<0))
#define AW8623_BIT_GO_ENABLE				(1<<0)
#define AW8623_BIT_GO_DISABLE				(0<<0)

 /* WAVSEQ1 0x07 */
#define AW8623_BIT_WAVSEQ1_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ1_WAV_FRM_SEQ1_MASK		(~(127<<0))

 /* WAVSEQ2 0x08 */
#define AW8623_BIT_WAVSEQ2_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ2_WAV_FRM_SEQ2_MASK		(~(127<<0))

 /* WAVSEQ3 0x09 */
#define AW8623_BIT_WAVSEQ3_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ3_WAV_FRM_SEQ3_MASK		(~(127<<0))

 /* WAVSEQ4 0x0A */
#define AW8623_BIT_WAVSEQ4_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ4_WAV_FRM_SEQ4_MASK		(~(127<<0))

 /* WAVSEQ5 0X0B */
#define AW8623_BIT_WAVSEQ5_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ5_WAV_FRM_SEQ5_MASK		(~(127<<0))

 /* WAVSEQ6 0X0C */
#define AW8623_BIT_WAVSEQ6_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ6_WAV_FRM_SEQ6_MASK		(~(127<<0))

 /* WAVSEQ7 */
#define AW8623_BIT_WAVSEQ7_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ7_WAV_FRM_SEQ7_MASK		(~(127<<0))

 /* WAVSEQ8 */
#define AW8623_BIT_WAVSEQ8_WAIT				(1<<7)
#define AW8623_BIT_WAVSEQ8_WAV_FRM_SEQ8_MASK		(~(127<<0))

 /* WAVLOOP */
#define AW8623_BIT_WAVLOOP_SEQN_MASK			(~(15<<4))
#define AW8623_BIT_WAVLOOP_SEQNP1_MASK			(~(15<<0))
#define AW8623_BIT_WAVLOOP_INIFINITELY			(15<<0)

 /* WAVLOOP1 */
#define AW8623_BIT_WAVLOOP1_SEQ1_MASK			(~(15<<4))
#define AW8623_BIT_WAVLOOP1_SEQ2_MASK			(~(15<<0))

 /* WAVLOOP2 */
#define AW8623_BIT_WAVLOOP2_SEQ3_MASK			(~(15<<4))
#define AW8623_BIT_WAVLOOP2_SEQ4_MASK			(~(15<<0))

 /* WAVLOOP3 */
#define AW8623_BIT_WAVLOOP3_SEQ5_MASK			(~(15<<4))
#define AW8623_BIT_WAVLOOP3_SEQ6_MASK			(~(15<<0))

 /* WAVLOOP4 */
#define AW8623_BIT_WAVLOOP4_SEQ7_MASK			(~(15<<4))
#define AW8623_BIT_WAVLOOP4_SEQ8_MASK			(~(15<<0))


 /* PLAYPRIO */
#define AW8623_BIT_PLAYPRIO_GO_MASK			(~(3<<6))
#define AW8623_BIT_PLAYPRIO_TRIG3_MASK			(~(3<<4))
#define AW8623_BIT_PLAYPRIO_TRIG2_MASK			(~(3<<2))
#define AW8623_BIT_PLAYPRIO_TRIG1_MASK			(~(3<<0))

 /* TRGCFG1 */
#define AW8623_BIT_TRGCFG1_TRG3_POLAR_MASK		(~(1<<5))
#define AW8623_BIT_TRGCFG1_TRG3_POLAR_NEG		(1<<5)
#define AW8623_BIT_TRGCFG1_TRG3_POLAR_POS		(0<<5)
#define AW8623_BIT_TRGCFG1_TRG3_EDGE_MASK		(~(1<<4))
#define AW8623_BIT_TRGCFG1_TRG3_EDGE_POS		(1<<4)
#define AW8623_BIT_TRGCFG1_TRG3_EDGE_POS_NEG		(0<<4)
#define AW8623_BIT_TRGCFG1_TRG2_POLAR_MASK		(~(1<<3))
#define AW8623_BIT_TRGCFG1_TRG2_POLAR_NEG		(1<<3)
#define AW8623_BIT_TRGCFG1_TRG2_POLAR_POS		(0<<3)
#define AW8623_BIT_TRGCFG1_TRG2_EDGE_MASK		(~(1<<2))
#define AW8623_BIT_TRGCFG1_TRG2_EDGE_POS		(1<<2)
#define AW8623_BIT_TRGCFG1_TRG2_EDGE_POS_NEG		(0<<2)
#define AW8623_BIT_TRGCFG1_TRG1_POLAR_MASK		(~(1<<1))
#define AW8623_BIT_TRGCFG1_TRG1_POLAR_NEG		(1<<1)
#define AW8623_BIT_TRGCFG1_TRG1_POLAR_POS		(0<<1)
#define AW8623_BIT_TRGCFG1_TRG1_EDGE_MASK		(~(1<<0))
#define AW8623_BIT_TRGCFG1_TRG1_EDGE_POS		(1<<0)
#define AW8623_BIT_TRGCFG1_TRG1_EDGE_POS_NEG		(0<<0)

 /* TRGCFG2 */
#define AW8623_BIT_TRGCFG2_TRG3_ENABLE_MASK		(~(1<<2))
#define AW8623_BIT_TRGCFG2_TRG3_ENABLE			(1<<2)
#define AW8623_BIT_TRGCFG2_TRG3_DISABLE			(0<<2)
#define AW8623_BIT_TRGCFG2_TRG2_ENABLE_MASK		(~(1<<1))
#define AW8623_BIT_TRGCFG2_TRG2_ENABLE			(1<<1)
#define AW8623_BIT_TRGCFG2_TRG2_DISABLE			(0<<1)
#define AW8623_BIT_TRGCFG2_TRG1_ENABLE_MASK		(~(1<<0))
#define AW8623_BIT_TRGCFG2_TRG1_ENABLE			(1<<0)
#define AW8623_BIT_TRGCFG2_TRG1_DISABLE			(0<<0)

 /*DBGCTRL 0X20 */
#define AW8623_BIT_DBGCTRL_INTN_TRG_SEL_MASK		(~(1<<5))
#define AW8623_BIT_DBGCTRL_INTN_SEL_ENABLE		(1<<5)
#define AW8623_BIT_DBGCTRL_TRG_SEL_ENABLE		(0<<5)
#define AW8623_BIT_DBGCTRL_INT_MODE_MASK		(~(3<<2))
#define AW8623_BIT_DBGCTRL_INTN_LEVEL_MODE		(0<<2)
#define AW8623_BIT_DBGCTRL_INT_MODE_EDGE		(1<<2)
#define AW8623_BIT_DBGCTRL_INTN_POSEDGE_MODE		(2<<2)
#define AW8623_BIT_DBGCTRL_INTN_BOTH_EDGE_MODE		(3<<2)

 /* DATCTRL */
#define AW8623_BIT_DATCTRL_FC_MASK			(~(1<<6))
#define AW8623_BIT_DATCTRL_FC_1000HZ			(3<<6)
#define AW8623_BIT_DATCTRL_FC_800HZ			(3<<6)
#define AW8623_BIT_DATCTRL_FC_600HZ			(1<<6)
#define AW8623_BIT_DATCTRL_FC_400HZ			(0<<6)
#define AW8623_BIT_DATCTRL_LPF_ENABLE_MASK		(~(1<<5))
#define AW8623_BIT_DATCTRL_LPF_ENABLE			(1<<5)
#define AW8623_BIT_DATCTRL_LPF_DISABLE			(0<<5)

 /*PWMPRC 0X2D */
#define AW8623_BIT_PWMPRC_PRC_EN_MASK			(~(1<<7))
#define AW8623_BIT_PWMPRC_PRC_ENABLE			(1<<7)
#define AW8623_BIT_PWMPRC_PRC_DISABLE			(0<<7)
#define AW8623_BIT_PWMPRC_PRCTIME_MASK			(~(0x7f<<0))

 /* PWMDBG */
#define AW8623_BIT_PWMDBG_PWM_MODE_MASK			(~(3<<5))
#define AW8623_BIT_PWMDBG_PWM_12K			(3<<5)
#define AW8623_BIT_PWMDBG_PWM_24K			(2<<5)
#define AW8623_BIT_PWMDBG_PWM_48K			(0<<5)

 /* WAVECTRL */
#define AW8623_BIT_WAVECTRL_NUM_OV_DRIVER_MASK		(~(0xF<<4))
#define AW8623_BIT_WAVECTRL_NUM_OV_DRIVER		(0<<4)

 /* BST_AUTO */
#define AW8623_BIT_BST_AUTO_BST_AUTOSW_MASK		(~(1<<2))
#define AW8623_BIT_BST_AUTO_BST_AUTOMATIC_BOOST		(1<<2)
#define AW8623_BIT_BST_AUTO_BST_MANUAL_BOOST		(0<<2)

 /* CONT_CTRL */
#define AW8623_BIT_CONT_CTRL_ZC_DETEC_MASK		(~(1<<7))
#define AW8623_BIT_CONT_CTRL_ZC_DETEC_ENABLE		(1<<7)
#define AW8623_BIT_CONT_CTRL_ZC_DETEC_DISABLE		(0<<7)
#define AW8623_BIT_CONT_CTRL_WAIT_PERIOD_MASK		(~(3<<5))
#define AW8623_BIT_CONT_CTRL_WAIT_8PERIOD		(3<<5)
#define AW8623_BIT_CONT_CTRL_WAIT_4PERIOD		(2<<5)
#define AW8623_BIT_CONT_CTRL_WAIT_2PERIOD		(1<<5)
#define AW8623_BIT_CONT_CTRL_WAIT_1PERIOD		(0<<5)
#define AW8623_BIT_CONT_CTRL_MODE_MASK			(~(1<<4))
#define AW8623_BIT_CONT_CTRL_BY_DRV_TIME		(1<<4)
#define AW8623_BIT_CONT_CTRL_BY_GO_SIGNAL		(0<<4)
#define AW8623_BIT_CONT_CTRL_EN_CLOSE_MASK		(~(1<<3))
#define AW8623_BIT_CONT_CTRL_CLOSE_PLAYBACK		(1<<3)
#define AW8623_BIT_CONT_CTRL_OPEN_PLAYBACK		(0<<3)
#define AW8623_BIT_CONT_CTRL_F0_DETECT_MASK		(~(1<<2))
#define AW8623_BIT_CONT_CTRL_F0_DETECT_ENABLE		(1<<2)
#define AW8623_BIT_CONT_CTRL_F0_DETECT_DISABLE		(0<<2)
#define AW8623_BIT_CONT_CTRL_O2C_MASK			(~(1<<1))
#define AW8623_BIT_CONT_CTRL_O2C_ENABLE			(1<<1)
#define AW8623_BIT_CONT_CTRL_O2C_DISABLE		(0<<1)
#define AW8623_BIT_CONT_CTRL_AUTO_BRK_MASK		(~(1<<0))
#define AW8623_BIT_CONT_CTRL_AUTO_BRK_ENABLE		(1<<0)
#define AW8623_BIT_CONT_CTRL_AUTO_BRK_DISABLE		(0<<0)

#define AW8623_BIT_D2SCFG_CLK_ADC_MASK			(~(7<<5))
#define AW8623_BIT_D2SCFG_CLK_ASC_1P5MHZ		(3<<5)

 /* DETCTRL */
#define AW8623_BIT_DETCTRL_RL_OS_MASK			(~(1<<6))
#define AW8623_BIT_DETCTRL_RL_DETECT			(1<<6)
#define AW8623_BIT_DETCTRL_OS_DETECT			(0<<6)
#define AW8623_BIT_DETCTRL_PROTECT_MASK			(~(1<<5))
#define AW8623_BIT_DETCTRL_PROTECT_NO_ACTION		(1<<5)
#define AW8623_BIT_DETCTRL_PROTECT_SHUTDOWN		(0<<5)
#define AW8623_BIT_DETCTRL_VBAT_GO_MASK			(~(1<<1))
#define AW8623_BIT_DETCTRL_VABT_GO_ENABLE		(1<<1)
#define AW8623_BIT_DETCTRL_VBAT_GO_DISBALE		(0<<1)
#define AW8623_BIT_DETCTRL_DIAG_GO_MASK			(~(1<<0))
#define AW8623_BIT_DETCTRL_DIAG_GO_ENABLE		(1<<0)
#define AW8623_BIT_DETCTRL_DIAG_GO_DISABLE		(0<<0)


 /* VBAT MODE */
#define AW8623_BIT_DETCTRL_VBAT_MODE_MASK		(~(1<<6))
#define AW8623_BIT_DETCTRL_VBAT_HW_COMP			(1<<6)
#define AW8623_BIT_DETCTRL_VBAT_SW_COMP			(0<<6)


 /* ANACTRL */
#define AW8623_BIT_ANACTRL_LRA_SRC_MASK			(~(1<<5))
#define AW8623_BIT_ANACTRL_LRA_SRC_REG			(1<<5)
#define AW8623_BIT_ANACTRL_LRA_SRC_EFUSE		(0<<5)
#define AW8623_BIT_ANACTRL_EN_IO_PD1_MASK		(~(1<<0))
#define AW8623_BIT_ANACTRL_EN_IO_PD1_HIGH		(1<<0)
#define AW8623_BIT_ANACTRL_EN_IO_PD1_LOW		(0<<0)

/* SW_BRAKE */
#define AW8623_BIT_EN_BRAKE_CONT_MASK			(~(1<<3))
#define AW8623_BIT_EN_BRAKE_CONT_ENABLE			(1<<3)
#define AW8623_BIT_EN_BRAKE_CONT_DISABLE		(0<<3)
#define AW8623_BIT_EN_BRAKE_RAM_MASK			(~(1<<2))
#define AW8623_BIT_EN_BRAKE_RAM_ENABLE			(1<<2)
#define AW8623_BIT_EN_BRAKE_RAM_DISABLE			(0<<2)
#define AW8623_BIT_EN_BRAKE_RTP_MASK			(~(1<<1))
#define AW8623_BIT_EN_BRAKE_RTP_ENABLE			(1<<1)
#define AW8623_BIT_EN_BRAKE_RTP_DISABLE			(0<<1)
#define AW8623_BIT_EN_BRAKE_TRIG_MASK			(~(1<<0))
#define AW8623_BIT_EN_BRAKE_TRIG_ENABLE			(1<<0)
#define AW8623_BIT_EN_BRAKE_TRIG_DISABLE		(0<<0)

/* PRLVL */
#define AW8623_BIT_PRLVL_PR_EN_MASK			(~(1<<7))
#define AW8623_BIT_PRLVL_PR_ENABLE			(1<<7)
#define AW8623_BIT_PRLVL_PR_DISABLE			(0<<7)
#define AW8623_BIT_PRLVL_PRLVL_MASK			(~(0x7f<<0))

/* PRTIME */
#define AW8623_BIT_PRTIME_PRTIME_MASK			(~(0xff<<0))

#define AW8623_BIT_BEMF_NUM_BRK_MASK			(~(0xf<<0))

/* TD_H 0x4b TD_brake */
#define AW8623_BIT_TDH_TD_BRAKE_MASK			(~(0xF<<4))
#define AW8623_BIT_R_SPARE_MASK				(~(1<<7))
#define AW8623_BIT_R_SPARE_ENABLE			(1<<7)

#define REG_SYSINT         0x02
#define REG_SYSINTM        0x03

#define REG_SYSCTRL        0x04

#define REG_GO            0x05

#define REG_RAMADDRH       0x40
#define REG_RAMADDRL       0x41
#define REG_RAMDATA        0x42

#define AW8623_RAM_BASE_ADDRH       0x08
#define AW8623_RAM_BASE_ADDRL       0x00
#define AW8623_RAM_MAX              0x2000

#define REG_WAVSEQ(x)      (0x07 + (x - 1))
#define REG_WAVLOOP(x)     (0x0F + (x - 1))
#define REG_MAINLOOP       0x13

#define REG_DATDBG        0x3B
#define REG_PRLVL         0x3F

#define AW8623_BIT_R_SPARE_MASK				(~(1<<7))
#define AW8623_BIT_R_SPARE_ENABLE			(1<<7)

#define REG_ID           0x00

enum aw8623_flags {
    AW8623_FLAG_NONR = 0,
    AW8623_FLAG_SKIP_INTERRUPTS = 1,
};

enum aw8623_chipids {
    AW8623_ID = 1,
};

enum aw8623_haptic_read_write {
    AW8623_HAPTIC_CMD_READ_REG = 0,
    AW8623_HAPTIC_CMD_WRITE_REG = 1,
};


enum aw8623_haptic_work_mode {
    AW8623_HAPTIC_STANDBY_MODE = 0,
    AW8623_HAPTIC_RAM_MODE = 1,
    AW8623_HAPTIC_RTP_MODE = 2,
    AW8623_HAPTIC_TRIG_MODE = 3,
    AW8623_HAPTIC_CONT_MODE = 4,
    AW8623_HAPTIC_RAM_LOOP_MODE = 5,
};

enum aw8623_haptic_bst_mode {
    AW8623_HAPTIC_BYPASS_MODE = 0,
    AW8623_HAPTIC_BOOST_MODE = 1,
};

enum aw8623_haptic_activate_mode {
  AW8623_HAPTIC_ACTIVATE_RAM_MODE = 0,
  AW8623_HAPTIC_ACTIVATE_CONT_MODE = 1,
};

enum aw8623_haptic_vbat_comp_mode {
    AW8623_HAPTIC_VBAT_SW_COMP_MODE = 0,
    AW8623_HAPTIC_VBAT_HW_COMP_MODE = 1,
};
enum aw8623_haptic_pwm_mode {
    AW8623_PWM_48K = 0,
    AW8623_PWM_24K = 1,
    AW8623_PWM_12K = 2,
};

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */
 
/*@{*/

extern const unsigned char aw8623_haptic_235Hz_220104_0x071d[1821];
#define aw8623_haptic      aw8623_haptic_235Hz_220104_0x071d

static const uint32_t f0_pre = 1700;
static const uint32_t f0_coeff = 260;
static const uint8_t f0_cali_percent = 7;

/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/

#if CONFIG_DRIVER_USING_CONST == 0
static void *gDevAw8623Interface = NULL;
static void * gDevAw8623PinRst = NULL;
static void * gDevAw8623PinInt = NULL;
#else
extern const kdi2c_t * const gDevAw8623Interface;
extern const kdgpio_t * const gDevAw8623PinRst;
extern const kdgpio_t * const gDevAw8623PinInt;
#endif
static uint8_t mIsInterfaceInit = 0;

static uint8_t f0_pre_num = 0x05;
static uint8_t f0_wait_num = 0x03;
static uint8_t f0_repeat_num = 0x02;
static uint8_t f0_trace_num = 0x0f;
static uint8_t cont_drv_lvl = 125;
static uint8_t cont_drv_lvl_ov = 155;
static uint8_t lra_f0 = 0x00;
static uint8_t chip_flag = 0;   
static int8_t cali_lra = 0;

static enum aw8623_haptic_work_mode mCurrentWorkMode;
static uint32_t mCurrentF0;
static uint8_t mPosBeme;
static uint8_t mNegBeme;
static uint8_t mF0CaliLra;
static uint8_t mEnableIrqDones;

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/

static inline int32_t _regRead(uint8_t reg, uint8_t *data, uint16_t length) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regRead((void *) gDevAw8623Interface, ADDRESS, reg, 1, data, length);
}


static inline int32_t _regWrite(uint8_t reg, uint8_t *data, uint16_t length) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regWrite((void *) gDevAw8623Interface, ADDRESS, reg, 1, data, length); 
}


static inline int32_t _writeReg(uint8_t reg, uint8_t data) {
    if (!mIsInterfaceInit) {
        return -1;
    }
    return kdi2c_regWrite((void *) gDevAw8623Interface, ADDRESS, reg, 1, &data, 1); 
}


static inline int32_t _writeBits(uint8_t reg, unsigned int mask, uint8_t regData) {
    uint8_t regVal = 0;

    if (_regRead(reg, &regVal, 1) != 0) {   
        return -1;
    }
    
    regVal &= mask;
    regVal |= regData;
    
    if (_regWrite(reg, &regVal, 1) != 0) {
        return -1;
    }

    return 0;
}


static inline void _forecePowerDown(void) {
    kdgpio_output((void *) gDevAw8623PinRst, 0);
}


static inline void _forecePowerUp(void) {
    kdgpio_output((void *) gDevAw8623PinRst, 1);
}


static int32_t _enableSramMode(void) {
    uint8_t data = 0x61;

    if (_regWrite(REG_SYSCTRL, &data, 1) != 0) {
        return -1;
    }
    
    return 0;
}


static int32_t _writeWaveformLibrary(void) {
    int32_t rc = -1;
    
    _writeBits(AW8623_REG_SYSCTRL, AW8623_BIT_SYSCTRL_RAMINIT_MASK, AW8623_BIT_SYSCTRL_RAMINIT_EN);
    _writeReg(AW8623_REG_BASE_ADDRH, AW8623_RAM_BASE_ADDRH);
    _writeReg(AW8623_REG_BASE_ADDRL, AW8623_RAM_BASE_ADDRL);

    _writeReg(AW8623_REG_RAMADDRH, AW8623_RAM_BASE_ADDRH);
    _writeReg(AW8623_REG_RAMADDRL, AW8623_RAM_BASE_ADDRL);
    
    return _regWrite(AW8623_REG_RAMDATA, (uint8_t *) aw8623_haptic, sizeof(aw8623_haptic));
    
//    for (uint32_t i = 0; i < sizeof(aw8623_haptic); i++) {
//        rc = _writeReg(AW8623_REG_RAMDATA, aw8623_haptic[i]);
//        if (rc < 0) {
//            return -1;
//        }
//    }
//    
//    rc = 0;
}


static int32_t _setPlayModeRamMode(void) {
    uint8_t data = 0;
    
    if (_regRead(REG_SYSCTRL, &data, 1) != 0) {
        return -1;
    }
    
    data &= ~(0x03 << 2);

    if (_regWrite(REG_SYSCTRL, &data, 1) != 0) {
        return -1;
    }
    
    return 0;
}


static int32_t _readId(void) {
    uint8_t id = 0;
        
    if (_regRead(REG_ID, &id, 1) != 0) {
        return -1;
    }
    
    if (id != 0x23) {
        return -1;
    }
    
    return 0;
}


static int32_t _interruptClear(void) {
    uint8_t data = 0;
    
    if (_regRead(AW8623_REG_SYSINT, &data, 1) != 0) {
        return -1;
    }
    
    return data;
}


static void _hapticPlayGo(uint8_t flag) {
    if (flag) {
        _writeBits(AW8623_REG_GO,
            AW8623_BIT_GO_MASK, AW8623_BIT_GO_ENABLE);
    } else {
        _writeBits(AW8623_REG_GO,
            AW8623_BIT_GO_MASK, AW8623_BIT_GO_DISABLE);
    }
}


static uint8_t _hapticIsGo(void) {
    uint8_t tmp = 0;
    
    if (_regRead(AW8623_REG_GO, &tmp, 1) != 0) {
        return -1;
    }
    
    return tmp & 0x01;
}


static void _hapticSetPwm(uint8_t mode) {
    switch(mode) {
        case AW8623_PWM_48K:
            _writeBits(AW8623_REG_PWMDBG,
                AW8623_BIT_PWMDBG_PWM_MODE_MASK, AW8623_BIT_PWMDBG_PWM_48K);
            break;
        case AW8623_PWM_24K:
            _writeBits(AW8623_REG_PWMDBG,
                AW8623_BIT_PWMDBG_PWM_MODE_MASK, AW8623_BIT_PWMDBG_PWM_24K);
            break;
        case AW8623_PWM_12K:
            _writeBits(AW8623_REG_PWMDBG,
                AW8623_BIT_PWMDBG_PWM_MODE_MASK, AW8623_BIT_PWMDBG_PWM_12K);
            break;
        default:
            break;
    }
}


static void _hapticSwicthMotorprotectConfig(uint8_t addr, uint8_t val) {
	if (addr == 1) {
		_writeBits(AW8623_REG_DETCTRL,
            AW8623_BIT_DETCTRL_PROTECT_MASK, AW8623_BIT_DETCTRL_PROTECT_SHUTDOWN);
        _writeBits(AW8623_REG_PWMPRC,
            AW8623_BIT_PWMPRC_PRC_EN_MASK, AW8623_BIT_PWMPRC_PRC_ENABLE);
        _writeBits(AW8623_REG_PRLVL,
            AW8623_BIT_PRLVL_PR_EN_MASK, AW8623_BIT_PRLVL_PR_ENABLE);     
	} else if (addr == 0) {
		_writeBits(AW8623_REG_DETCTRL,
            AW8623_BIT_DETCTRL_PROTECT_MASK, AW8623_BIT_DETCTRL_PROTECT_NO_ACTION);	
        _writeBits(AW8623_REG_PWMPRC,
            AW8623_BIT_PWMPRC_PRC_EN_MASK, AW8623_BIT_PWMPRC_PRC_DISABLE);  
        _writeBits(AW8623_REG_PRLVL,
            AW8623_BIT_PRLVL_PR_EN_MASK, AW8623_BIT_PRLVL_PR_DISABLE);   
   	 } else if (addr == 0x2d) {
        _writeBits(AW8623_REG_PWMPRC,
            AW8623_BIT_PWMPRC_PRCTIME_MASK, val);
	 } else if (addr == 0x3e) {
        _writeBits(AW8623_REG_PRLVL,
            AW8623_BIT_PRLVL_PRLVL_MASK, val);
	 } else if (addr == 0x3f) {
        _writeBits(AW8623_REG_PRTIME,
            AW8623_BIT_PRTIME_PRTIME_MASK, val);
	 }
}


static void _hapticSetVbatMode(unsigned char flag) {
    if (flag == AW8623_HAPTIC_VBAT_HW_COMP_MODE) {
        _writeBits(AW8623_REG_ADCTEST,
                AW8623_BIT_DETCTRL_VBAT_MODE_MASK, AW8623_BIT_DETCTRL_VBAT_HW_COMP);
    } else {
        _writeBits(AW8623_REG_ADCTEST,
                AW8623_BIT_DETCTRL_VBAT_MODE_MASK, AW8623_BIT_DETCTRL_VBAT_SW_COMP);
    }
}


static void _hapticActive(void) {
    _writeBits(AW8623_REG_SYSCTRL,
            AW8623_BIT_SYSCTRL_WORK_MODE_MASK, AW8623_BIT_SYSCTRL_ACTIVE);
    
    _interruptClear();
}


static void _hapticSetPlayMode(unsigned char playMode) {
    switch (playMode) {
        case AW8623_HAPTIC_STANDBY_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_STANDBY_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_WORK_MODE_MASK, AW8623_BIT_SYSCTRL_STANDBY);
            break;
        case AW8623_HAPTIC_RAM_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_RAM_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_PLAY_MODE_MASK, AW8623_BIT_SYSCTRL_PLAY_MODE_RAM);
            break;
        case AW8623_HAPTIC_RAM_LOOP_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_RAM_LOOP_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_PLAY_MODE_MASK, AW8623_BIT_SYSCTRL_PLAY_MODE_RAM);
            break;
        case AW8623_HAPTIC_RTP_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_RTP_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_PLAY_MODE_MASK, AW8623_BIT_SYSCTRL_PLAY_MODE_RTP);
            break;
        case AW8623_HAPTIC_TRIG_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_TRIG_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_PLAY_MODE_MASK, AW8623_BIT_SYSCTRL_PLAY_MODE_RAM);
            break;
        case AW8623_HAPTIC_CONT_MODE:
            mCurrentWorkMode = AW8623_HAPTIC_CONT_MODE;
            _writeBits(AW8623_REG_SYSCTRL,
                    AW8623_BIT_SYSCTRL_PLAY_MODE_MASK, AW8623_BIT_SYSCTRL_PLAY_MODE_CONT);
            break;
        default:
            break;
    }
}


static int32_t _hapticF0Preset(void) {
    uint16_t f0_reg = 0;
    
    f0_reg = 1000000000 / (f0_pre * f0_coeff);

    uint8_t tmp = (uint8_t) ((f0_reg >> 8) & 0xff);
    _regWrite(AW8623_REG_F_PRE_H, &tmp, 1);
    
    tmp = (uint8_t) ((f0_reg >> 0) & 0xff);
    _regWrite(AW8623_REG_F_PRE_L, &tmp, 1);
    
    return 0;
}


static int32_t _hapticReadF0(void) {
    uint16_t f0;
    uint8_t regVal;
    uint32_t tmp = 0;

    _regRead(AW8623_REG_F_LRA_F0_H, &regVal, 1);
    f0 = (regVal << 8);
    
    _regRead(AW8623_REG_F_LRA_F0_L, &regVal, 1);
    f0 |= (regVal << 0);
    
    if (f0 == 0) {
        return -1;
    }
    
    tmp = 1000000000 / (f0 * f0_coeff);
    lra_f0 = (uint16_t) tmp;
    
    return 0;
}


static void _hapticStopDelay(void) {
    uint8_t regVal = 0;
    unsigned int cnt = 100;

    while (cnt--) {
		_regRead(AW8623_REG_GLB_STATE, &regVal, 1);
		if ((regVal & 0x0f) == 0x00) {
			return;
        }
        
		osDelay(2);
    }
}


static void _hapticStop(void) {
    _hapticPlayGo(0);
    _hapticStopDelay();
    _hapticSetPlayMode(AW8623_HAPTIC_STANDBY_MODE);
}


static void _hapticStart(void) {
    _hapticActive();
    _hapticPlayGo(1);
}


static void _hapticGetBeme(void) {
    unsigned char regVal = 0;
    
    _regRead(AW8623_REG_WAIT_VOL_MP, &regVal, 1);
    mPosBeme = (regVal << 0);

    _regRead(AW8623_REG_WAIT_VOL_MN, &regVal, 1);
    mNegBeme = (regVal << 0);
}


static int32_t _hapticGetF0(void) {
    int ret = 0;
    uint8_t i = 0;
    uint8_t reg_val = 0;
    uint8_t t_f0_ms = 0;
    uint8_t t_f0_trace_ms = 0;
    uint8_t f0_cali_cnt = 50;

    uint32_t lra_f0 = f0_pre;

    /* f0 calibrate work mode */
    _hapticStop();
    _hapticSetPlayMode(AW8623_HAPTIC_CONT_MODE);

    //aw8624_i2c_write(AW8624_REG_TRIM_LRA, g_aw8624.cali_lra);
    _writeReg(AW8623_REG_TRIM_LRA, 0x00);
    
    _writeBits(AW8623_REG_CONT_CTRL, AW8623_BIT_CONT_CTRL_EN_CLOSE_MASK, AW8623_BIT_CONT_CTRL_OPEN_PLAYBACK);
    _writeBits(AW8623_REG_CONT_CTRL, AW8623_BIT_CONT_CTRL_F0_DETECT_MASK, AW8623_BIT_CONT_CTRL_F0_DETECT_ENABLE);

    /* LPF */
    _writeBits(AW8623_REG_DATCTRL, AW8623_BIT_DATCTRL_FC_MASK, AW8623_BIT_DATCTRL_FC_1000HZ);
    _writeBits(AW8623_REG_DATCTRL, AW8623_BIT_DATCTRL_LPF_ENABLE_MASK, AW8623_BIT_DATCTRL_LPF_ENABLE);

    /* LRA OSC Source */
//    if (g_aw8624.f0_flag == HAPTIC_CALI_F0) {
    _writeBits(AW8623_REG_ANACTRL, AW8623_BIT_ANACTRL_LRA_SRC_MASK, AW8623_BIT_ANACTRL_LRA_SRC_REG);
//    } else {
//      aw8624_i2c_write_bits(AW8624_REG_ANACTRL,
//      AW8624_BIT_ANACTRL_LRA_SRC_MASK,
//      AW8624_BIT_ANACTRL_LRA_SRC_EFUSE);
//    }

    /* preset f0 */
    _hapticF0Preset();

    /* f0 driver level */
    _writeReg(AW8623_REG_DRV_LVL, cont_drv_lvl);

    /* f0 trace parameter */
    //AW_LOGI(AW_TAG,"%s pre_num:0x%02x,wait_num:0x%02x,\r\n",__func__,
    //g_aw8624_dts_params.f0_pre_num, g_aw8624_dts_params.f0_wait_num);
    //AW_LOGI(AW_TAG,"%s repeat_num:0x%02x,trace_num:0x%02x,\r\n", __func__,
    //g_aw8624_dts_params.f0_repeat_num, g_aw8624_dts_params.f0_trace_num);
    _writeReg(AW8623_REG_NUM_F0_1, (f0_pre_num << 4) | (f0_wait_num << 0));
    _writeReg(AW8623_REG_NUM_F0_2, f0_repeat_num);
    _writeReg(AW8623_REG_NUM_F0_3, f0_trace_num);

    /* clear aw8624 interrupt */
    ret = _regRead(AW8623_REG_SYSINT, &reg_val, 1);

    /* play go and start f0 calibration */
    _hapticStart();

    /* f0 trace time */
    t_f0_ms = 1000 * 10 / f0_pre;
    t_f0_trace_ms = t_f0_ms * (f0_pre_num +
    f0_wait_num + (f0_trace_num +
    f0_wait_num) * (f0_repeat_num - 1));
    
    // AW_LOGI(AW_TAG,"%s t_f0_trace_ms: %d\r\n", __func__, t_f0_trace_ms);
    osDelay(t_f0_trace_ms);

    for (i = 0; i < f0_cali_cnt; i++) {
        ret = _regRead(AW8623_REG_GLB_STATE, &reg_val, 1);
        
        /* f0 calibrate done */
        if ((reg_val & 0x0f) == 0x00) {
            _hapticReadF0();
            _hapticGetBeme();
            break;
        }
        
        osDelay(2);
        // AW_LOGI(AW_TAG,"%s f0 cali sleep 10ms\n", __func__);
    }

    if (i == f0_cali_cnt) {
        ret = -1;
    } else {
        ret = 0;
    }

    /* restore default config */
    _writeBits(AW8623_REG_CONT_CTRL, AW8623_BIT_CONT_CTRL_EN_CLOSE_MASK, AW8623_BIT_CONT_CTRL_CLOSE_PLAYBACK);
    _writeBits(AW8623_REG_CONT_CTRL, AW8623_BIT_CONT_CTRL_F0_DETECT_MASK, AW8623_BIT_CONT_CTRL_F0_DETECT_DISABLE);
    return ret;
}


static uint8_t _hapticF0Cali(void) {
    int ret = 0;
    uint8_t reg_val = 0;
    uint16_t f0_limit = 0;
    char f0_cali_lra = 0;
    int f0_cali_step = 0;

    // AW_LOGI(AW_TAG,"%s enter\r\n", __func__);

    // g_aw8624.f0_flag = HAPTIC_CALI_F0;
    _writeReg(AW8623_REG_TRIM_LRA, 0x00);

    if (_hapticGetF0()) {
        // AW_LOGE(AW_TAG,"%s get f0 error, user defafult f0\n", __func__);
    } else {
        /* max and min limit */
        f0_limit = lra_f0;
        if (lra_f0 * 100 < f0_pre * (100 - f0_cali_percent)) {
            f0_limit = f0_pre;
        }
        if (lra_f0 * 100 > f0_pre * (100 + f0_cali_percent)) {
            f0_limit = f0_pre;
        }

        /* calculate cali step */
        f0_cali_step = 100000 * ((int) f0_limit - (int) f0_pre) / ((int) f0_limit * 25);
//        AW_LOGI(AW_TAG,"%s  line=%d f0_cali_step=%d\n",
//        __func__, __LINE__, f0_cali_step);
//        AW_LOGI(AW_TAG,"%s line=%d  f0_limit=%d\n",
//        __func__, __LINE__, (int)f0_limit);
//        AW_LOGI(AW_TAG,"%s line=%d  f0_pre=%d\n",
//        __func__, __LINE__, (int)g_aw8624_dts_params.f0_pre);

        if (f0_cali_step >= 0) {/*f0_cali_step >= 0 */
            if (f0_cali_step % 10 >= 5) {
                f0_cali_step = f0_cali_step / 10 + 1 + (chip_flag == 1 ? 32 : 16);
            } else {
                f0_cali_step = f0_cali_step / 10 + (chip_flag == 1 ? 32 : 16);
            }
        } else {	/*f0_cali_step < 0 */
            if (f0_cali_step % 10 <= -5) {
                f0_cali_step = (chip_flag == 1 ? 32 : 16) + (f0_cali_step / 10 - 1);
            } else {
                f0_cali_step = (chip_flag == 1 ? 32 : 16) + f0_cali_step / 10;
            }
        }

        if (chip_flag == 1){
          if (f0_cali_step > 31) {
              f0_cali_lra = (char) f0_cali_step - 32;
          } else {
              f0_cali_lra = (char) f0_cali_step + 32;
          }
        } else {
          if (f0_cali_step < 16 || (f0_cali_step > 31 && f0_cali_step < 48)) {
              f0_cali_lra = (char) f0_cali_step + 16;
          } else {
              f0_cali_lra = (char) f0_cali_step - 16;
          }
        }

#ifndef AW_F0_BRINGUP_CALI
        /*save cli_lar to nvram */
        //aw8624->cali_lra =  (char)f0_cali_lra;
        //aw8624_haptic_set_calilra_to_nvram(aw8624->cali_lra);
#endif

        /* update cali step */
        // AW_LOGI(AW_TAG,"%s f0_cali_lra=%d\n", __func__, (int)f0_cali_lra);
        cali_lra =  (char) f0_cali_lra;
        _writeReg(AW8623_REG_TRIM_LRA, (char) f0_cali_lra);
        _regRead(AW8623_REG_TRIM_LRA, &reg_val, 1);
        // AW_LOGI(AW_TAG,"%s final trim_lra=0x%02x\n", __func__, reg_val);
    }

    /* restore default work mode */
    _hapticSetPlayMode(AW8623_HAPTIC_STANDBY_MODE);
    _hapticStop();
    
    return f0_cali_lra;
}


static int32_t _hapticSetTrimLra(uint8_t v) {
    return _regWrite(AW8623_REG_TRIM_LRA, (uint8_t *) &v, 1);
}


static void _hapticConfigWaveSeq(uint8_t index, uint8_t enable, uint8_t waveNumber) {
    index = REG_WAVSEQ(index) + 1;
    uint8_t data = waveNumber;
    
    if (enable) {
    } else {
        data = 0x00;
    }
    
    _regWrite(index, &data, 1);
}


static void _hapticConfigWaveLoop(uint8_t index, uint8_t time) {
    uint8_t writeData;
    uint8_t readData;
    
    if (index % 2) {
        index = index / 2;

        _regRead(REG_WAVLOOP(1) + index, &readData, 1);
        
        writeData = (readData & 0xF0) | (time << 0);

        _regWrite(REG_WAVLOOP(1) + index, &writeData, 1);
    } else {
        index = index / 2;
        
        _regRead(REG_WAVLOOP(1) + index, &readData, 1);
        
        writeData = (readData & 0x0F) | (time << 4);
        
        _regWrite(REG_WAVLOOP(1) + index, &writeData, 1);
    }
}


static int32_t _enableIrqDones(uint8_t flag) {
    uint8_t tmp;
    
    if (_regRead(REG_SYSINTM, &tmp, 1) != 0) {
        return -1;
    }
    
    if (flag) {
        tmp |= 0x01;
    } else {
        tmp &= ~0x01;
    }
    
    if (_regWrite(REG_SYSINTM, &tmp, 1) != 0) {
        return -1;
    }
    
    return 0;
}


static int32_t _hapticSetGain(uint8_t gain) {
    uint8_t tmp = gain;
    return _regWrite(AW8623_REG_DATDBG, &tmp, 1);
}


static int32_t _hapticSetBreak(uint8_t flag) {
    uint8_t tmp;
    
    if (_regRead(AW8623_REG_SW_BRAKE, &tmp, 1) != 0) {
        return -1;
    }
    
    if (flag) {
        tmp |= 0x04;
    } else {
        tmp &= ~0x04;
    }
    
    if (_regWrite(AW8623_REG_SW_BRAKE, &tmp, 1) != 0) {
        return -1;
    }
    
    return 0;
}


static int32_t _hapticLateInit(uint8_t caliValue) {
    uint8_t ret = 0;
    uint8_t reg_val = 0;

    ret = _regRead(AW8623_REG_EF_RDATAH, &reg_val, 1);
    if ((ret == 0) && ((reg_val & 0x1) == 1)) {
        chip_flag = 1;
    } else {
        chip_flag = 0;
//        LOG_E(AW_TAG, "%s: to read register AW8623_REG_EF_RDATAH: %d\n",
//        __func__, ret);
    }
    
    _hapticSetPlayMode(AW8623_HAPTIC_STANDBY_MODE);
    _hapticSetPwm(AW8623_PWM_24K);
    _hapticSwicthMotorprotectConfig(0x00, 0x00);
    _hapticSetVbatMode(AW8623_HAPTIC_VBAT_HW_COMP_MODE);

    _writeBits(AW8623_REG_R_SPARE, (uint8_t) AW8623_BIT_R_SPARE_MASK, AW8623_BIT_R_SPARE_ENABLE);
    
    if (caliValue != 0xFF) {
        mF0CaliLra = caliValue;
        _hapticSetTrimLra(caliValue);
    } else {
        mF0CaliLra = _hapticF0Cali();
    }

    _hapticSetBreak(1);
    reg_val = 0x00;
    _regWrite(AW8623_REG_THRS_BRA_END, &reg_val, 1);
    _writeBits(AW8623_REG_WAVECTRL, (uint8_t) AW8623_BIT_WAVECTRL_NUM_OV_DRIVER_MASK, AW8623_BIT_WAVECTRL_NUM_OV_DRIVER);

    uint16_t cont_zc_thr = 0x08f8;
    uint8_t tset = 0x11;
    
    reg_val = (uint8_t) ((cont_zc_thr >> 8) & 0xff);
    _regWrite(AW8623_REG_ZC_THRSH_H, &reg_val, 1);
    reg_val = (uint8_t) (cont_zc_thr & 0x00ff);
    _regWrite(AW8623_REG_ZC_THRSH_L, &reg_val, 1);
    
    _regWrite(AW8623_REG_TSET, &tset, 1);
    
    uint8_t bemf_config[4] = {
        [0] = 0x10,
        [1] = 0x08,
        [2] = 0x23,
        [3] = 0xf8,
    };
    _regWrite(AW8623_REG_BEMF_VTHH_H, &bemf_config[0], 1);
    _regWrite(AW8623_REG_BEMF_VTHH_L, &bemf_config[1], 1);
    _regWrite(AW8623_REG_BEMF_VTHL_H, &bemf_config[2], 1);
    _regWrite(AW8623_REG_BEMF_VTHL_L, &bemf_config[3], 1);

    return 0;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

void aw8623_initBus(void *devIf, void *pinRst, void *pinInt) {
    if (mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 1;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevAw8623Interface != NULL) {
        return -1;
    }
#endif
    
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623Interface = devIf;
    ASSERT(gDevAw8623Interface != NULL);
#endif
    kdi2c_init((void *) gDevAw8623Interface);
    kdi2c_powerUp((void *) gDevAw8623Interface);

#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623PinInt = pinInt;
    ASSERT(gDevAw8623PinInt != NULL);
#endif
    kdgpio_init((void *) gDevAw8623PinInt);
    kdgpio_powerUp((void *) gDevAw8623PinInt, KDGPIO_MODE_INPUT, KDGPIO_PULL_NONE);
    
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623PinRst = pinRst;
    ASSERT(gDevAw8623PinRst != NULL);
#endif
    kdgpio_init((void *) gDevAw8623PinRst);
    kdgpio_powerUp((void *) gDevAw8623PinRst, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output((void *) gDevAw8623PinRst, 1);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osDelay(10);
#else
    qSTimer_t wait;
    qSTimer_Set(&wait, 10);
    while (!qSTimer_Expired(&wait));
#endif
}

int32_t aw8623_initSoft(uint8_t caliValue) {
    if (_readId() != 0) {
        return -1;
    }
    
    //// haptic init
    _hapticSetGain(128);
    _hapticSetPwm(AW8623_PWM_24K);
    _hapticSwicthMotorprotectConfig(0x00, 0x00);
    _hapticSetVbatMode(AW8623_HAPTIC_VBAT_HW_COMP_MODE);
    _hapticLateInit(caliValue);
    _interruptClear();

    mEnableIrqDones = 0;
    
    return 0;
}

void aw8623_finalizeBus(void) {
    if (!mIsInterfaceInit) {
        return;
    }
    mIsInterfaceInit = 0;
    
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevAw8623Interface == NULL) {
        return -1;
    }
#endif
    
    kdgpio_powerDown((void *)gDevAw8623PinInt);
    kdgpio_finalize((void *)gDevAw8623PinInt);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623PinInt = NULL;
#endif
    
    kdgpio_output((void *) gDevAw8623PinRst, 0);
    kdgpio_powerDown((void *) gDevAw8623PinRst);
    kdgpio_finalize((void *) gDevAw8623PinRst);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623PinRst = NULL;
#endif
    
    kdi2c_powerDown((void *) gDevAw8623Interface);
    kdi2c_finalize((void *) gDevAw8623Interface);
#if CONFIG_DRIVER_USING_CONST == 0
    gDevAw8623Interface = NULL;
#endif
}

void aw8623_finalizeSoft(void) {
}

void aw8623_powerDown(void) {
    _forecePowerDown();
}

void aw8623_powerUp(void) {
    _forecePowerUp();
}

void aw8623_updateLib(void) {
    _enableSramMode();
    _writeWaveformLibrary();
    _hapticSetPlayMode(AW8623_HAPTIC_RAM_MODE);
}

int32_t aw8623_enableIrqDones(uint8_t enable) {
    if (_enableIrqDones(enable) < 0) {
        mEnableIrqDones = 0;
        return -1;
    } else if (enable) {
        mEnableIrqDones = 1;
    } else {
        mEnableIrqDones = 0;
    }
    
    return 0;
}

int32_t aw8623_vibrationSequence(uint8_t *seqIndex, uint8_t indexCount) {
    if (indexCount > 0x07) {
        return -1;
    }
    
    for (uint8_t i = 0; i < indexCount; i++) {
        if (seqIndex[i] > 0x0F) {
            seqIndex[i] = 0x00;
        }
    }
    
#if 0
    if (mEnableIrqDones) {
        if (mPinInt->method.input(mPinInt)) {
            return -1;
        }
        
        _clearInt();
    } else {
        int32_t status = _isGoing();
        
        if (status < 0 || status >= 1) {
            return -1;
        }
    }
#else
    int32_t status = _hapticIsGo();
    if (status < 0 || status >= 1) {
        aw8623_stop();
    }
#endif
    
    for (uint8_t i = 0; i < indexCount; i++) {
        _hapticConfigWaveSeq(i, 
            seqIndex[i] == 0x00 ? 0 : 1, 
            seqIndex[i]);
        _hapticConfigWaveLoop(i, 
            0);
    }

    return 0;
}

int32_t aw8623_vibrationLoop(uint8_t *seqIndex, uint8_t *seqLoopCount, uint8_t indexCount) {
    if (indexCount > 0x07) {
        return -1;
    }
    
    for (uint8_t i = 0; i < indexCount; i++) {
        if (seqIndex[i] > 0x0F) {
            seqIndex[i] = 0xFF;
        }
        
        if (seqLoopCount[i] > 0x0E) {
            seqLoopCount[i] = 0x0F;
        }
    }
    
#if 0
    if (mEnableIrqDones) {
        if (mPinInt->method.input(mPinInt)) {
            return -1;
        }
        
        _clearInt();
    } else {
        int32_t status = _isGoing();
        
        if (status < 0 || status >= 1) {
            return -1;
        }
    }
#else
    int32_t status = _hapticIsGo();
    if (status < 0 || status >= 1) {
        aw8623_stop();
    }
#endif
    
    for (uint8_t i = 0; i < indexCount; i++) {
        _hapticConfigWaveSeq(i, seqIndex[i] == 0xFF ? 0 : 1, seqIndex[i]);
        
        if (seqIndex[i] == 0xFF) {
            continue;
        }
        
        _hapticConfigWaveLoop(i, seqLoopCount[i]);
    }

    return 0;
}

void aw8623_start(void) {
    _hapticActive();
    _hapticPlayGo(1);
}

void aw8623_stop(void) {
    _hapticPlayGo(0);
    _hapticStop();
}

uint8_t aw8623_isDones(void) {
    if (mEnableIrqDones) {
        if (kdgpio_input((void *) gDevAw8623PinInt)) {
            return 0;
        }

        return 1;
    } else {
        int32_t status = _hapticIsGo();
        
        if (status < 0 || status >= 1) {
            return 0;
        }
        
        return 1;
    }
}

uint8_t aw8623_getTrimLra(void) {
    return mF0CaliLra;
}

/*@}*/
