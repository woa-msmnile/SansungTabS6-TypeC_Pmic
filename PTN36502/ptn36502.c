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
	INT MODE = 0;
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	switch (Config)
	{
	case INIT_MODE:
		SpbWriteDataSynchronously(&DevExt->I2CContextPTN, Device_Control, (UINT8[]) { 0x81 }, 1);
		break;
	case USB3_ONLY_MODE:
		if (EnableDFP)
		{ 
			SpbWriteDataSynchronously(&DevExt->I2CContextPTN, Mode_Control, (UINT8[]) { 0x41 }, 1);
		}
		else
		{
			SpbWriteDataSynchronously(&DevExt->I2CContextPTN, Mode_Control, (UINT8[]) { 0xa1 }, 1);
		}
		SpbReadDataSynchronously(&DevExt->I2CContextPTN, Mode_Control, &MODE, 1);
		Trace(TRACE_LEVEL_ERROR, SURFACE_OTHER_INFO, "PTN36502: USB3_ONLY_MODE: Read 0x0b command as (%x)", MODE);
		break;
	default:
		break;
	}

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}