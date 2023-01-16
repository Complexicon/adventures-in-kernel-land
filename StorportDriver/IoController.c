#include "all.h"
#include "structs.h"

void HandleServiceRequest(void* context, void* irp) {
	IRP* pIrp = (IRP*)irp;
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(pIrp);

	if (irpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL && irpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_MINIPORT_PROCESS_SERVICE_IRP) {
		CommunicationTest* t = (CommunicationTest*)pIrp->AssociatedIrp.SystemBuffer;

		DbgPrintEx(0, 0, "recvd message from usermode: %s\n", t->message);

		CommunicationTest response = { 0 };
		strcpy(response.message, "pingusdingus_from_kernelmode");

		memcpy(pIrp->AssociatedIrp.SystemBuffer, &response, sizeof(CommunicationTest));

		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(CommunicationTest);
	}

	StorPortCompleteServiceIrp(context, irp);
}