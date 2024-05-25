/*++

Module Name:

    ptn36502.h

--*/

NTSTATUS
PTN36502_Config(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	BYTE Config,
	BYTE EnableDFP
);

#define Chip_ID						0x00
#define Chip_Rev					0x01
#define USB_TXRX_Control			0x04
#define DS_TXRX_Control				0x05
#define DP_Link_Control				0x06
#define DP_Lane0_Control			0x07
#define DP_Lane1_Control			0x08
#define DP_Lane2_Control			0x09
#define DP_Lane3_Control			0x0a
#define Mode_Control				0x0b
#define Squelch_Threshold			0x0c
#define Device_Control				0x0d

enum config_type {
	INIT_MODE = 0,
	USB3_ONLY_MODE = 1,
	DP4_LANE_MODE = 2,
	DP2_LANE_USB3_MODE = 3,
	AUX_THRU_MODE = 4,
	AUX_CROSS_MODE = 5,
	SAFE_STATE = 6,
	CHECK_EXIST = 7,
};
