#include "all.h"

ULONG VirtFindAdapter(void* DeviceExtension, void* HwContext, void* BusInformation, void* LowerDevice, char* ArgumentString, PORT_CONFIGURATION_INFORMATION* ConfigInfo, BOOLEAN* Again) {
	UNREFERENCED_PARAMETER(DeviceExtension);
	UNREFERENCED_PARAMETER(HwContext);
	UNREFERENCED_PARAMETER(BusInformation);
	UNREFERENCED_PARAMETER(ArgumentString);
	UNREFERENCED_PARAMETER(Again);
	
	DriverDeviceExtension* driverCtx = (DriverDeviceExtension*) DeviceExtension;

	KeInitializeSpinLock(&driverCtx->DeviceListLock);
	InitializeListHead(&driverCtx->DeviceList);

	ConfigInfo->VirtualDevice = TRUE;
	ConfigInfo->ScatterGather = TRUE;
	ConfigInfo->Master = TRUE;
	ConfigInfo->CachesData = FALSE;
	ConfigInfo->NumberOfBuses = 1;
	ConfigInfo->MaximumNumberOfTargets = 8;
	ConfigInfo->MapBuffers = TRUE;
	ConfigInfo->MaximumTransferLength = SP_UNINITIALIZED_VALUE;
	ConfigInfo->AlignmentMask = FILE_BYTE_ALIGNMENT;

	PsCreateSystemThread(&driverCtx->WorkerThread, 0, 0, 0, 0, KeWorkerThread, DeviceExtension);

	return SP_RETURN_FOUND;
}

BOOLEAN VirtInitialize(void* DeviceExtension) {
	UNREFERENCED_PARAMETER(DeviceExtension);
	return TRUE;
}

SCSI_ADAPTER_CONTROL_STATUS VirtAdapterControl(void* DeviceExtension, SCSI_ADAPTER_CONTROL_TYPE ControlType, void* Parameters) {
	UNREFERENCED_PARAMETER(DeviceExtension);
	UNREFERENCED_PARAMETER(ControlType);
	UNREFERENCED_PARAMETER(Parameters);

	return ScsiAdapterControlSuccess;
}

BOOLEAN VirtResetBus(void* DeviceExtension, ULONG PathID) {
	StorPortCompleteRequest(DeviceExtension, (UCHAR)PathID, SP_UNTAGGED, SP_UNTAGGED, (UCHAR)SRB_STATUS_BUS_RESET);
	return TRUE;
}

//uint8_t HandleIO(DriverDeviceExtension* driverCtx, SCSI_REQUEST_BLOCK* Srb, BOOLEAN* complete) {
//
//	UNREFERENCED_PARAMETER(driverCtx);
//	UNREFERENCED_PARAMETER(Srb);
//	UNREFERENCED_PARAMETER(complete);
//
//	return SRB_STATUS_INVALID_REQUEST;
//}

void VirtFreeResources(void* DeviceExtension) {

}

BOOLEAN VirtStartIo(void* DeviceExtension, SCSI_REQUEST_BLOCK* Srb) {

	DriverDeviceExtension* driverCtx = (DriverDeviceExtension*)DeviceExtension;

	BOOLEAN complete = TRUE;
	uint8_t srbStatus = SRB_STATUS_INVALID_REQUEST;

	switch (Srb->Function) {
	case SRB_FUNCTION_EXECUTE_SCSI:
		srbStatus = HandleScsi(driverCtx, Srb, &complete);
		break;
	/* STUB
	case SRB_FUNCTION_IO_CONTROL:
		srbStatus = HandleIO(driverCtx, Srb, &complete);
		break;
		*/
	case SRB_FUNCTION_RESET_LOGICAL_UNIT:
		StorPortCompleteRequest(DeviceExtension, Srb->PathId, Srb->TargetId, Srb->Lun, SRB_STATUS_BUSY);
		srbStatus = SRB_STATUS_SUCCESS;
		break;

	case SRB_FUNCTION_RESET_DEVICE:
		StorPortCompleteRequest(DeviceExtension, Srb->PathId, Srb->TargetId, SP_UNTAGGED, SRB_STATUS_TIMEOUT);
		srbStatus = SRB_STATUS_SUCCESS;
		break;
	}

	if (complete) {
		Srb->SrbStatus = srbStatus;
		StorPortNotification(RequestComplete, DeviceExtension, Srb);
	}

	return complete;

}

NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	VIRTUAL_HW_INITIALIZATION_DATA hwInitData = { 0 };

	hwInitData.HwInitializationDataSize = sizeof(VIRTUAL_HW_INITIALIZATION_DATA);

	// Required.
	hwInitData.HwFindAdapter = VirtFindAdapter;
	hwInitData.HwInitialize = VirtInitialize;
	hwInitData.HwAdapterControl = VirtAdapterControl;
	hwInitData.HwResetBus = VirtResetBus;
	hwInitData.HwStartIo = VirtStartIo;
	
	// Usermode Communication
	hwInitData.HwProcessServiceRequest = HandleServiceRequest;
	hwInitData.HwFreeAdapterResources = VirtFreeResources;
	
	hwInitData.AdapterInterfaceType = Internal;
	hwInitData.MultipleRequestPerLu = TRUE;
	hwInitData.PortVersionFlags = 0;

	hwInitData.DeviceExtensionSize = sizeof(DriverDeviceExtension);
	hwInitData.SpecificLuExtensionSize = sizeof(VirtScsiDevice);
	hwInitData.SrbExtensionSize = sizeof(DriverSrbExtension);

	DbgPrintEx(0,0,"%s\n", "Storport Driver Running v2");
	return StorPortInitialize(DriverObject, RegistryPath, (HW_INITIALIZATION_DATA*)&hwInitData, NULL);
}