/*++

Module Name:

	s2mm005.c

--*/

//--------------------------------------------------------------------- Includes
#include "../SamsungEC/SamsungEC.h"
#include "../SamsungEC/Spb.h"
#include "../S2MM005/s2mm005.h"
#include "../PTN36502/ptn36502.h"
#include "s2mm005.tmh"

//--------------------------------------------------------------------- Includes


NTSTATUS
S2mm005_Set_TypeC_Mode(
	PSURFACE_BATTERY_FDO_DATA DevExt
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG            TypeC_Status;
	S2mm005_Get_TypeC_Status(DevExt, &TypeC_Status);

	switch (TypeC_Status) {
	case 0x00:
	case 0x03:
	case 0x07:
	case 0x11:
	case 0x1d:
		PTN36502_Config(DevExt, USB3_ONLY_MODE, 0);
		break;
	case 0x0e:
		PTN36502_Config(DevExt, USB3_ONLY_MODE, 1);
		break;
	default:
		break;
	}

}