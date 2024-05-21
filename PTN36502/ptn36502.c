/*++

Module Name:

	ptn36502.c

--*/

//--------------------------------------------------------------------- Includes
#include "../SamsungEC/SamsungEC.h"
#include "../SamsungEC/Spb.h"
#include "../PTN36502/ptn36502.h"
#include "ptn36502.tmh"

NTSTATUS
PTN36502_Config(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	BYTE Config,
	BYTE EnableDFP)
{
	NTSTATUS Status = STATUS_SUCCESS;
	switch (Config)
	{
	case INIT_MODE:
		SpbWriteDataSynchronously(&DevExt->I2CContextPTN, Device_Control, (UINT8[]) { 0x81 }, 1);
		break;
	case USB3_ONLY_MODE:
		break;
	default:
		break;
	}
}