/*++

Module Name:

    s2mm005.h

--*/

//---------------------------------------------------------------------- Pragmas
#pragma once
//--------------------------------------------------------------------- Includes

NTSTATUS
S2mm005_Get_TypeC_Status(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG TypeC_Status
);