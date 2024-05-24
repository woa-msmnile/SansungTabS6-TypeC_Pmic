/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

	miniclass.c

Abstract:

	This module implements battery miniclass functionality specific to the
	Surface battery driver.

	N.B. This code is provided "AS IS" without any expressed or implied warranty.

--*/

//--------------------------------------------------------------------- Includes

#include "../SamsungEC/SamsungEC.h"
#include "../SamsungEC/Spb.h"
#include "usbfnbase.h"
#include "miniclass.tmh"

#include "sm5705_fuelgauge.h"

#include "../S2MM005/s2mm005.h"

//------------------------------------------------------------------- Prototypes

_IRQL_requires_same_
VOID
SM5705FGUpdateTag(
	_Inout_ PSURFACE_BATTERY_FDO_DATA DevExt
);

BCLASS_QUERY_TAG_CALLBACK SM5705FGQueryTag;
BCLASS_QUERY_INFORMATION_CALLBACK SM5705FGQueryInformation;
BCLASS_SET_INFORMATION_CALLBACK SM5705FGSetInformation;
BCLASS_QUERY_STATUS_CALLBACK SM5705FGQueryStatus;
BCLASS_SET_STATUS_NOTIFY_CALLBACK SM5705FGSetStatusNotify;
BCLASS_DISABLE_STATUS_NOTIFY_CALLBACK SM5705FGDisableStatusNotify;

//---------------------------------------------------------------------- Pragmas

#pragma alloc_text(PAGE, SM5705FGPrepareHardware)
#pragma alloc_text(PAGE, SM5705FGUpdateTag)
#pragma alloc_text(PAGE, SM5705FGQueryTag)
#pragma alloc_text(PAGE, SM5705FGQueryInformation)
#pragma alloc_text(PAGE, SM5705FGQueryStatus)
#pragma alloc_text(PAGE, SM5705FGSetStatusNotify)
#pragma alloc_text(PAGE, SM5705FGDisableStatusNotify)
#pragma alloc_text(PAGE, SM5705FGSetInformation)
//------------------------------------------------------------ Battery Interface

_Use_decl_annotations_
VOID
SM5705FGPrepareHardware(
	WDFDEVICE Device
)

/*++

Routine Description:

	This routine is called to initialize battery data to sane values.

Arguments:

	Device - Supplies the device to initialize.

Return Value:

	NTSTATUS

--*/

{

	PSURFACE_BATTERY_FDO_DATA DevExt;
	NTSTATUS Status = STATUS_SUCCESS;

	PAGED_CODE();
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");

	DevExt = GetDeviceExtension(Device);

	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	SM5705FGUpdateTag(DevExt);
	WdfWaitLockRelease(DevExt->StateLock);

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return;
}

_Use_decl_annotations_
VOID
SM5705FGUpdateTag(
	PSURFACE_BATTERY_FDO_DATA DevExt
)

/*++

Routine Description:

	This routine is called when static battery properties have changed to
	update the battery tag.

Arguments:

	DevExt - Supplies a pointer to the device extension  of the battery to
		update.

Return Value:

	None

--*/

{

	PAGED_CODE();

	DevExt->BatteryTag += 1;
	if (DevExt->BatteryTag == BATTERY_TAG_INVALID) {
		DevExt->BatteryTag += 1;
	}

	return;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGQueryTag(
	PVOID Context,
	PULONG BatteryTag
)

/*++

Routine Description:

	This routine is called to get the value of the current battery tag.

Arguments:

	Context - Supplies the miniport context value for battery

	BatteryTag - Supplies a pointer to a ULONG to receive the battery tag.

Return Value:

	NTSTATUS

--*/

{
	PSURFACE_BATTERY_FDO_DATA DevExt;
	NTSTATUS Status;

	PAGED_CODE();
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");

	DevExt = (PSURFACE_BATTERY_FDO_DATA)Context;
	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	*BatteryTag = DevExt->BatteryTag;
	WdfWaitLockRelease(DevExt->StateLock);
	if (*BatteryTag == BATTERY_TAG_INVALID) {
		Status = STATUS_NO_SUCH_DEVICE;
	}
	else {
		Status = STATUS_SUCCESS;
	}

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
SM5705FGQueryBatteryInformation(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PBATTERY_INFORMATION BatteryInformationResult
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG            CycleCount;

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");

	BatteryInformationResult->Capabilities =
		BATTERY_SYSTEM_BATTERY |
		BATTERY_SET_CHARGE_SUPPORTED |
		BATTERY_SET_DISCHARGE_SUPPORTED;

	BatteryInformationResult->Technology = 1;

	BYTE LION[4] = {'L','I','O','N'};
	RtlCopyMemory(BatteryInformationResult->Chemistry, LION, 4);

	// mWh (7040mAh * 4.4V)
	BatteryInformationResult->DesignedCapacity = 30976;

	// mWh (6840mAh * 4.4V)
	BatteryInformationResult->FullChargedCapacity = 30096;

	BatteryInformationResult->DefaultAlert1 = BatteryInformationResult->FullChargedCapacity * 7 / 100; // 7% of total capacity for error
	BatteryInformationResult->DefaultAlert2 = BatteryInformationResult->FullChargedCapacity * 9 / 100; // 9% of total capacity for warning
	BatteryInformationResult->CriticalBias = 0;


	Status = sm5705_Get_CycleCount(DevExt, &CycleCount);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "sm5705 Get CycleCount failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}

	BatteryInformationResult->CycleCount = CycleCount;

	Trace(
		TRACE_LEVEL_INFORMATION,
		SURFACE_BATTERY_TRACE,
		"BATTERY_INFORMATION: \n"
		"Capabilities: %d \n"
		"Technology: %d \n"
		"DesignedCapacity: %d \n"
		"FullChargedCapacity: %d \n"
		"DefaultAlert1: %d \n"
		"DefaultAlert2: %d \n"
		"CriticalBias: %d \n"
		"CycleCount: %d\n",
		BatteryInformationResult->Capabilities,
		BatteryInformationResult->Technology,
		BatteryInformationResult->DesignedCapacity,
		BatteryInformationResult->FullChargedCapacity,
		BatteryInformationResult->DefaultAlert1,
		BatteryInformationResult->DefaultAlert2,
		BatteryInformationResult->CriticalBias,
		BatteryInformationResult->CycleCount);

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
SM5705FGQueryBatteryEstimatedTime(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG ResultValue
)
{
	NTSTATUS Status = STATUS_SUCCESS;

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");

	*ResultValue = BATTERY_UNKNOWN_TIME;

	Trace(
		TRACE_LEVEL_INFORMATION,
		SURFACE_BATTERY_TRACE,
		"BatteryEstimatedTime: BATTERY_UNKNOWN_TIME \n");

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGQueryInformation(
	PVOID Context,
	ULONG BatteryTag,
	BATTERY_QUERY_INFORMATION_LEVEL Level,
	LONG AtRate,
	PVOID Buffer,
	ULONG BufferLength,
	PULONG ReturnedLength
)

/*++

Routine Description:

	Called by the class driver to retrieve battery information

	The battery class driver will serialize all requests it issues to
	the miniport for a given battery.

	Return invalid parameter when a request for a specific level of information
	can't be handled. This is defined in the battery class spec.

Arguments:

	Context - Supplies the miniport context value for battery

	BatteryTag - Supplies the tag of current battery

	Level - Supplies the type of information required

	AtRate - Supplies the rate of drain for the BatteryEstimatedTime level

	Buffer - Supplies a pointer to a buffer to place the information

	BufferLength - Supplies the length in bytes of the buffer

	ReturnedLength - Supplies the length in bytes of the returned data

Return Value:

	Success if there is a battery currently installed, else no such device.

--*/

{
	PSURFACE_BATTERY_FDO_DATA DevExt;
	PVOID ReturnBuffer;
	size_t ReturnBufferLength;
	NTSTATUS Status;

	BATTERY_REPORTING_SCALE ReportingScale = { 0 };
	BATTERY_INFORMATION BatteryInformationResult = { 0 };
	WCHAR StringResult[MAX_BATTERY_STRING_SIZE] = { 0 };
	BATTERY_MANUFACTURE_DATE ManufactureDate = { 0 };

	ULONG  Temperature;
	USHORT DateData = 0;
	ULONG ResultValue;

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	PAGED_CODE();

	DevExt = (PSURFACE_BATTERY_FDO_DATA)Context;
	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	if (BatteryTag != DevExt->BatteryTag) {
		Status = STATUS_NO_SUCH_DEVICE;
		goto QueryInformationEnd;
	}

	//
	// Determine the value of the information being queried for and return it.
	//

	ReturnBuffer = NULL;
	ReturnBufferLength = 0;
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO, "Query for information level 0x%x\n", Level);
	Status = STATUS_INVALID_DEVICE_REQUEST;
	switch (Level) {
	case BatteryInformation:
		Status = SM5705FGQueryBatteryInformation(DevExt, &BatteryInformationResult);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SM5705FGQueryBatteryInformation failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = &BatteryInformationResult;
		ReturnBufferLength = sizeof(BATTERY_INFORMATION);
		Status = STATUS_SUCCESS;
		break;

	case BatteryEstimatedTime:
		Status = SM5705FGQueryBatteryEstimatedTime(DevExt, &ResultValue);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SM5705FGQueryBatteryEstimatedTime failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = &ResultValue;
		ReturnBufferLength = sizeof(ResultValue);
		Status = STATUS_SUCCESS;
		break;

	case BatteryUniqueID:

		swprintf_s(StringResult, sizeof(StringResult) / sizeof(WCHAR), L"%c%c%c%c%c%c%c%c",
			'S',
			'M',
			'5',
			'7',
			'0',
			'5',
			'F',
			'G');

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BatteryUniqueID: %S\n",
			StringResult);

		Status = RtlStringCbLengthW(StringResult,
			sizeof(StringResult),
			&ReturnBufferLength);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "RtlStringCbLengthW failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = StringResult;
		ReturnBufferLength += sizeof(WCHAR);
		Status = STATUS_SUCCESS;
		break;


	case BatteryManufactureName:

		swprintf_s(StringResult, sizeof(StringResult) / sizeof(WCHAR), L"%c%c",
			0x53,  // S
			0x53   // S
		);

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BatteryManufactureName: %S\n",
			StringResult);

		Status = RtlStringCbLengthW(StringResult,
			sizeof(StringResult),
			&ReturnBufferLength);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "RtlStringCbLengthW failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = StringResult;
		ReturnBufferLength += sizeof(WCHAR);
		Status = STATUS_SUCCESS;
		break;

	case BatteryDeviceName:

		swprintf_s(StringResult, sizeof(StringResult) / sizeof(WCHAR), L"%c%c%c%c%c%c",
			0x53,  //S
			0x4D,  //M
			0x35,  //5
			0x37,  //7
			0x30,  //0
			0x35   //5
		);

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BatteryDeviceName: %S\n",
			StringResult);

		Status = RtlStringCbLengthW(StringResult,
			sizeof(StringResult),
			&ReturnBufferLength);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "RtlStringCbLengthW failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = StringResult;
		ReturnBufferLength += sizeof(WCHAR);
		Status = STATUS_SUCCESS;
		break;

		case BatterySerialNumber:

		swprintf_s(StringResult, sizeof(StringResult) / sizeof(WCHAR), L"%u", (UINT32)5705);

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BatterySerialNumber: %S\n",
			StringResult);

		Status = RtlStringCbLengthW(StringResult,
			sizeof(StringResult),
			&ReturnBufferLength);
		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "RtlStringCbLengthW failed with Status = 0x%08lX\n", Status);
			goto Exit;
		}

		ReturnBuffer = StringResult;
		ReturnBufferLength += sizeof(WCHAR);
		Status = STATUS_SUCCESS;
		break;

	case BatteryManufactureDate:

		ManufactureDate.Day = 1;
		ManufactureDate.Month = 1;
		ManufactureDate.Year = 2019;

		ReturnBuffer = &ManufactureDate;
		ReturnBufferLength = sizeof(BATTERY_MANUFACTURE_DATE);
		Status = STATUS_SUCCESS;
		break;

	case BatteryGranularityInformation:

		ReportingScale.Capacity = 7040;
		ReportingScale.Granularity = 1;

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BATTERY_REPORTING_SCALE: Capacity: %d, Granularity: %d\n",
			ReportingScale.Capacity,
			ReportingScale.Granularity);

		ReturnBuffer = &ReportingScale;
		ReturnBufferLength = sizeof(BATTERY_REPORTING_SCALE);
		Status = STATUS_SUCCESS;
		break;

	case BatteryTemperature:

		Status = sm5705_Get_Temperature(DevExt, &Temperature);

		if (!NT_SUCCESS(Status))
		{
			Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "sm5705 Get Temperature failed with Status = 0x%08lX\n", Status);
		}

		Temperature = (ULONG)Temperature / (ULONG)10;

		Trace(
			TRACE_LEVEL_INFORMATION,
			SURFACE_BATTERY_TRACE,
			"BatteryTemperature: %d\n",
			Temperature);

		ReturnBuffer = &Temperature;
		ReturnBufferLength = sizeof(ULONG);
		Status = STATUS_SUCCESS;
		break;

	default:
		Status = STATUS_INVALID_PARAMETER;
		break;
	}

Exit:
	NT_ASSERT(((ReturnBufferLength == 0) && (ReturnBuffer == NULL)) ||
		((ReturnBufferLength > 0) && (ReturnBuffer != NULL)));

	if (NT_SUCCESS(Status)) {
		*ReturnedLength = (ULONG)ReturnBufferLength;
		if (ReturnBuffer != NULL) {
			if ((Buffer == NULL) || (BufferLength < ReturnBufferLength)) {
				Status = STATUS_BUFFER_TOO_SMALL;

			}
			else {
				memcpy(Buffer, ReturnBuffer, ReturnBufferLength);
			}
		}

	}
	else {
		*ReturnedLength = 0;
	}

QueryInformationEnd:
	WdfWaitLockRelease(DevExt->StateLock);
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGQueryStatus(
	PVOID Context,
	ULONG BatteryTag,
	PBATTERY_STATUS BatteryStatus
)

/*++

Routine Description:

	Called by the class driver to retrieve the batteries current status

	The battery class driver will serialize all requests it issues to
	the miniport for a given battery.

Arguments:

	Context - Supplies the miniport context value for battery

	BatteryTag - Supplies the tag of current battery

	BatteryStatus - Supplies a pointer to the structure to return the current
		battery status in

Return Value:

	Success if there is a battery currently installed, else no such device.

--*/

{
	PSURFACE_BATTERY_FDO_DATA DevExt;
	NTSTATUS Status;

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	PAGED_CODE();

	DevExt = (PSURFACE_BATTERY_FDO_DATA)Context;
	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	if (BatteryTag != DevExt->BatteryTag) {
		Status = STATUS_NO_SUCH_DEVICE;
		goto QueryStatusEnd;
	}

	ULONG            TypeC_Status;
	ULONG            Capacity;
	ULONG            Voltage;
	ULONG            Current;

	S2mm005_Get_TypeC_Status(DevExt, &TypeC_Status);

	switch (TypeC_Status) {
	case 0x11:
	case 0x1d:
		BatteryStatus->PowerState = BATTERY_POWER_ON_LINE;
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Battery: Charging \n");
		break;
	case 0x0e:
	default:
		BatteryStatus->PowerState = BATTERY_DISCHARGING;
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Battery: DisCharging \n");
		break;
	}

	Status = sm5705_Get_Capacity(DevExt, &Capacity);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "sm5705 Get Capacity failed with Status = 0x%08lX\n", Status);
		goto QueryStatusEnd;
	}

	// Voltage(mV)
	Status = sm5705_Get_Voltage(DevExt, &Voltage);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "sm5705 Get Voltage failed with Status = 0x%08lX\n", Status);
		goto QueryStatusEnd;
	}

	// Current (mA)
	Status = sm5705_Get_Current(DevExt, &Current);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "sm5705 Get Current failed with Status = 0x%08lX\n", Status);
		goto QueryStatusEnd;
	}

	/*
     * BatteryStatus only accepts battery Capacity data in units of mWh,
	 * So we need to perform some conversions.
	*/

	// mWh
	// (6840mAh * 4.4V = 30096mWh)
	BatteryStatus->Capacity = (ULONG)Capacity * 30096 / (ULONG)1000;
	// mV
	BatteryStatus->Voltage = (ULONG)Voltage;
	// mW (Signed)
	BatteryStatus->Rate = (((LONG)Current * (LONG)Voltage) / (LONG)1000);

	Trace(
		TRACE_LEVEL_INFORMATION,
		SURFACE_BATTERY_TRACE,
		"BATTERY_STATUS: \n"
		"PowerState: %d \n"
		"Capacity: %d \n"
		"Voltage: %d \n"
		"Rate: %d\n",
		BatteryStatus->PowerState,
		BatteryStatus->Capacity,
		BatteryStatus->Voltage,
		BatteryStatus->Rate);

	Status = STATUS_SUCCESS;

QueryStatusEnd:
	WdfWaitLockRelease(DevExt->StateLock);
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGSetStatusNotify(
	PVOID Context,
	ULONG BatteryTag,
	PBATTERY_NOTIFY BatteryNotify
)

/*++

Routine Description:

	Called by the class driver to set the capacity and power state levels
	at which the class driver requires notification.

	The battery class driver will serialize all requests it issues to
	the miniport for a given battery.

Arguments:

	Context - Supplies the miniport context value for battery

	BatteryTag - Supplies the tag of current battery

	BatteryNotify - Supplies a pointer to a structure containing the
		notification critera.

Return Value:

	Success if there is a battery currently installed, else no such device.

--*/

{
	PSURFACE_BATTERY_FDO_DATA DevExt;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(BatteryNotify);

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	PAGED_CODE();

	DevExt = (PSURFACE_BATTERY_FDO_DATA)Context;
	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	if (BatteryTag != DevExt->BatteryTag) {
		Status = STATUS_NO_SUCH_DEVICE;
		goto SetStatusNotifyEnd;
	}

	Status = STATUS_NOT_SUPPORTED;

SetStatusNotifyEnd:
	WdfWaitLockRelease(DevExt->StateLock);
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGDisableStatusNotify(
	PVOID Context
)

/*++

Routine Description:

	Called by the class driver to disable notification.

	The battery class driver will serialize all requests it issues to
	the miniport for a given battery.

Arguments:

	Context - Supplies the miniport context value for battery

Return Value:

	Success if there is a battery currently installed, else no such device.

--*/

{
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(Context);

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	PAGED_CODE();

	Status = STATUS_NOT_SUPPORTED;
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

_Use_decl_annotations_
NTSTATUS
SM5705FGSetInformation(
	PVOID Context,
	ULONG BatteryTag,
	BATTERY_SET_INFORMATION_LEVEL Level,
	PVOID Buffer
)

/*
 Routine Description:

	Called by the class driver to set the battery's charge/discharge state,
	critical bias, or charge current.

Arguments:

	Context - Supplies the miniport context value for battery

	BatteryTag - Supplies the tag of current battery

	Level - Supplies action requested

	Buffer - Supplies a critical bias value if level is BatteryCriticalBias.

Return Value:

	NTSTATUS

--*/

{
	PBATTERY_CHARGING_SOURCE ChargingSource;
	PULONG CriticalBias;
	PBATTERY_CHARGER_ID ChargerId;
	PBATTERY_CHARGER_STATUS ChargerStatus;
	PBATTERY_USB_CHARGER_STATUS UsbChargerStatus;
	USBFN_PORT_TYPE UsbFnPortType;
	PSURFACE_BATTERY_FDO_DATA DevExt;
	NTSTATUS Status;

	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE, "Entering %!FUNC!\n");
	PAGED_CODE();

	DevExt = (PSURFACE_BATTERY_FDO_DATA)Context;
	WdfWaitLockAcquire(DevExt->StateLock, NULL);
	if (BatteryTag != DevExt->BatteryTag) {
		Status = STATUS_NO_SUCH_DEVICE;
		goto SetInformationEnd;
	}

	if (Level == BatteryCharge)
	{
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : BatteryCharge\n");

		Status = STATUS_SUCCESS;
	}
	else if (Level == BatteryDischarge)
	{
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : BatteryDischarge\n");

		Status = STATUS_SUCCESS;
	}
	else if (Buffer == NULL)
	{
		Status = STATUS_INVALID_PARAMETER_4;
	}
	else if (Level == BatteryChargingSource)
	{
		ChargingSource = (PBATTERY_CHARGING_SOURCE)Buffer;

		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : BatteryChargingSource Type = %d\n",
			ChargingSource->Type);

		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : Set MaxCurrentDraw = %u mA\n",
			ChargingSource->MaxCurrent);

		Status = STATUS_SUCCESS;
	}
	else if (Level == BatteryCriticalBias)
	{
		CriticalBias = (PULONG)Buffer;
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : Set CriticalBias = %u mW\n",
			*CriticalBias);

		Status = STATUS_SUCCESS;
	}
	else if (Level == BatteryChargerId)
	{
		ChargerId = (PBATTERY_CHARGER_ID)Buffer;
		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : BatteryChargerId = %!GUID!\n",
			ChargerId);

		Status = STATUS_SUCCESS;
	}
	else if (Level == BatteryChargerStatus)
	{
		ChargerStatus = (PBATTERY_CHARGER_STATUS)Buffer;

		Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
			"SM5705FG : BatteryChargingSource Type = %d\n",
			ChargerStatus->Type);

		if (ChargerStatus->Type == BatteryChargingSourceType_USB)
		{
			UsbChargerStatus = (PBATTERY_USB_CHARGER_STATUS)Buffer;

			Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
				"SM5705FG : BatteryChargingSourceType_USB: Flags = %d, MaxCurrent = %d, Voltage = %d, PortType = %d, PortId = %llu, OemCharger = %!GUID!\n",
				UsbChargerStatus->Flags, UsbChargerStatus->MaxCurrent, UsbChargerStatus->Voltage, UsbChargerStatus->PortType, UsbChargerStatus->PortId, &UsbChargerStatus->OemCharger);

			UsbFnPortType = (USBFN_PORT_TYPE)(UINT64)UsbChargerStatus->PowerSourceInformation;

			Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_INFO,
				"SM5705FG : UsbFnPortType = %d\n",
				UsbFnPortType);
		}

		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_NOT_SUPPORTED;
	}

SetInformationEnd:
	WdfWaitLockRelease(DevExt->StateLock);
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_BATTERY_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}