/*++

Module Name:

	sm5705_fuelgauge.c

--*/

//--------------------------------------------------------------------- Includes
#include "../SamsungEC/SamsungEC.h"
#include "../SamsungEC/Spb.h"
#include "sm5705_fuelgauge.h"
#include "sm5705_fuelgauge.tmh"

NTSTATUS
sm5705_Get_CycleCount(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG CycleCount
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int  ret_cycle = 0;
	int  cycle = 0;

	Status = SpbReadDataSynchronously(&DevExt->I2CContext, SM5705_REG_SOC_CYCLE, &ret_cycle, 2);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}
	if (ret_cycle < 0) {
		cycle = 0;
	}
	else {
		cycle = ret_cycle & 0x03FF;
	}

	*CycleCount = cycle;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
sm5705_Get_Temperature(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG Temperature
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int  ret_Temperature = 0;
	int  temp = 0;

	Status = SpbReadDataSynchronously(&DevExt->I2CContext, SM5705_REG_TEMPERATURE, &ret_Temperature, 2);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}
	if (ret_Temperature < 0) {
		temp = 0;
	}
	else {
		temp = ((ret_Temperature & 0x7F00) >> 8) * 10;                  //integer bit
		temp = temp + (((ret_Temperature & 0x00f0) * 10) / 256); // integer + fractional bit
		if (ret_Temperature & 0x8000)
			temp *= -1;
	}

	*Temperature = temp;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
sm5705_Get_Capacity(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG Capacity
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int  ret_Capacity = 0;
	unsigned int soc = 0;

	Status = SpbReadDataSynchronously(&DevExt->I2CContext, SM5705_REG_SOC, &ret_Capacity, 2);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}
	if (ret_Capacity < 0) {
		soc = 500;
	}
	else {
		soc = ((ret_Capacity & 0xff00) >> 8) * 10; //integer bit;
		soc = soc + (((ret_Capacity & 0x00ff) * 10) / 256); // integer + fractional bit
	}

	*Capacity = soc;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
sm5705_Get_Voltage(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG Voltage
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int  ret_Voltage = 0;
	unsigned int vbat = 0;

	Status = SpbReadDataSynchronously(&DevExt->I2CContext, SM5705_REG_VOLTAGE, &ret_Voltage, 2);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}
	if (ret_Voltage < 0) {
		vbat = 4000;
	}
	else {
		vbat = ((ret_Voltage & 0x3800) >> 11) * 1000;         //integer;
		vbat = vbat + (((ret_Voltage & 0x07ff) * 1000) / 2048); // integer + fractional
	}

	*Voltage = vbat;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}

NTSTATUS
sm5705_Get_Current(
	PSURFACE_BATTERY_FDO_DATA DevExt,
	PULONG Current
)
{
	NTSTATUS Status = STATUS_SUCCESS;
	int  ret_Current = 0;
	int  curr = 0;

	Status = SpbReadDataSynchronously(&DevExt->I2CContext, SM5705_REG_CURRENT, &ret_Current, 2);
	if (!NT_SUCCESS(Status))
	{
		Trace(TRACE_LEVEL_ERROR, SURFACE_BATTERY_TRACE, "SpbReadDataSynchronously failed with Status = 0x%08lX\n", Status);
		goto Exit;
	}
	if (ret_Current < 0)
	{
		curr = 0;
	}
	else {
		curr = ((ret_Current & 0x1800) >> 11) * 1000; //integer;
		curr = curr + (((ret_Current & 0x07ff) * 1000) / 2048); // integer + fractional
		if (ret_Current & 0x8000)
			curr *= -1;
	}

	*Current = curr;

Exit:
	Trace(TRACE_LEVEL_INFORMATION, SURFACE_OTHER_TRACE,
		"Leaving %!FUNC!: Status = 0x%08lX\n",
		Status);
	return Status;
}