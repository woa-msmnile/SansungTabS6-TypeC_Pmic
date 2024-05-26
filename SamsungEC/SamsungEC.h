/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    SM5705FG.h

Abstract:

    This is the header file for the Surface battery driver.

    N.B. This code is provided "AS IS" without any expressed or implied warranty.

--*/

//---------------------------------------------------------------------- Pragmas

#pragma once

//--------------------------------------------------------------------- Includes

#include <wdm.h>
#include <wdf.h>
#include <batclass.h>
#include <wmistr.h>
#include <wmilib.h>
#include <ntstrsafe.h>
#include "..\SamsungEC\trace.h"
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
#include "..\SamsungEC\spb.h"

//--------------------------------------------------------------------- Literals

#define SURFACE_BATTERY_TAG                 'StaB'

/*
* Rob Green, a member of the NTDEV list, provides the
* following set of macros that'll keep you from having
* to scratch your head and count zeros ever again.
* Using these defintions, all you'll have to do is write:
*
* interval.QuadPart = RELATIVE(SECONDS(5));
*/

#ifndef ABSOLUTE
#define ABSOLUTE(wait) (wait)
#endif

#ifndef RELATIVE
#define RELATIVE(wait) (-(wait))
#endif

#ifndef NANOSECONDS
#define NANOSECONDS(nanos) \
	(((signed __int64)(nanos)) / 100L)
#endif

#ifndef MICROSECONDS
#define MICROSECONDS(micros) \
	(((signed __int64)(micros)) * NANOSECONDS(1000L))
#endif

#ifndef MILLISECONDS
#define MILLISECONDS(milli) \
	(((signed __int64)(milli)) * MICROSECONDS(1000L))
#endif

#ifndef SECONDS
#define SECONDS(seconds) \
	(((signed __int64)(seconds)) * MILLISECONDS(1000L))
#endif

//------------------------------------------------------------------ Definitions

typedef struct {
    UNICODE_STRING                  RegistryPath;
} SURFACE_BATTERY_GLOBAL_DATA, *PSURFACE_BATTERY_GLOBAL_DATA;

typedef struct {
    //
    // Device handle
    //
    WDFDEVICE Device;

    //
    // Battery class registration
    //

    PVOID                           ClassHandle;
    WDFWAITLOCK                     ClassInitLock;
    WMILIB_CONTEXT                  WmiLibContext;

    //
    // Spb (I2C) related members used for the lifetime of the device
    //

    // SM5705-FuelGauge
    SPB_CONTEXT I2CContext;
    // PTN36502 (Redriver)
    SPB_CONTEXT I2CContextPTN;
    // S2MM005 (USBPD)
    SPB_CONTEXT I2CContextCCIC;

    //
    // Battery state
    //

    WDFWAITLOCK                     StateLock;
    ULONG                           BatteryTag;
} SURFACE_BATTERY_FDO_DATA, *PSURFACE_BATTERY_FDO_DATA;

//------------------------------------------------------ WDF Context Declaration

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SURFACE_BATTERY_GLOBAL_DATA, GetGlobalData);
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SURFACE_BATTERY_FDO_DATA, GetDeviceExtension);

//----------------------------------------------------- Prototypes (miniclass.c)

_IRQL_requires_same_
VOID
SM5705FGPrepareHardware(
    _In_ WDFDEVICE Device
);

BCLASS_QUERY_TAG_CALLBACK SM5705FGQueryTag;
BCLASS_QUERY_INFORMATION_CALLBACK SM5705FGQueryInformation;
BCLASS_SET_INFORMATION_CALLBACK SM5705FGSetInformation;
BCLASS_QUERY_STATUS_CALLBACK SM5705FGQueryStatus;
BCLASS_SET_STATUS_NOTIFY_CALLBACK SM5705FGSetStatusNotify;
BCLASS_DISABLE_STATUS_NOTIFY_CALLBACK SM5705FGDisableStatusNotify;