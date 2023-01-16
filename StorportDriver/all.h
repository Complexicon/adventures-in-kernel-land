#pragma once
#include <ntifs.h>
#include <ntstrsafe.h>
#include <storport.h>
#include <ntddscsi.h>

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef struct {
	SCSI_ADAPTER_CONTROL_TYPE adapterState;
	UNICODE_STRING DeviceInterface;
	HANDLE WorkerThread;
	LIST_ENTRY DeviceList;
	KSPIN_LOCK DeviceListLock;
} DriverDeviceExtension;

typedef struct {
	uint8_t PathID;
	uint8_t TargetID;
	uint8_t Lun;
} VirtScsiDevice;

typedef struct {
	uint8_t UserData[1];
} DriverSrbExtension;

void KeWorkerThread(void* context);
uint8_t HandleScsi(DriverDeviceExtension* driverCtx, SCSI_REQUEST_BLOCK* Srb, BOOLEAN* complete);
void HandleServiceRequest(void* context, void* irp);