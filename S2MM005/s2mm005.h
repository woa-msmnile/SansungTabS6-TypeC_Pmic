/*++

Module Name:

    s2mm005.h

--*/

//---------------------------------------------------------------------- Pragmas
#pragma once
//--------------------------------------------------------------------- Includes

#define TypeC_OTG             0x00
#define TypeC_Charging        0x01
#define TypeC_Blank           0x02

NTSTATUS
S2mm005_Get_TypeC_Status(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG TypeC_Status
);

NTSTATUS
S2mm005_Set_TypeC_Mode(
	PSURFACE_BATTERY_FDO_DATA DevExt
);