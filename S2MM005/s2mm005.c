/*++

Module Name:

	s2mm005.c

--*/

//--------------------------------------------------------------------- Includes
#include "../SamsungEC/SamsungEC.h"
#include "../SamsungEC/Spb.h"
#include "../S2MM005/s2mm005.h"
#include "s2mm005.tmh"

BYTE s2mm005_Read_Status[2] = { 0x00,0x20 };

NTSTATUS
S2mm005_Get_TypeC_Status(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG TypeC_Status
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int USB_CC_Status = 0;

	Status = SpbReadDataSynchronouslyFromAnyAddr(&DevExt->I2CContextCCIC, s2mm005_Read_Status, &USB_CC_Status, sizeof(s2mm005_Read_Status), 1);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_OTHER_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE, "S2mm005: Read USB_CC_Status: %d \n", USB_CC_Status);

	*TypeC_Status = USB_CC_Status;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}