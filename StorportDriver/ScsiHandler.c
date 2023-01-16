#include "all.h"

uint8_t HandleScsi(DriverDeviceExtension* driverCtx, SCSI_REQUEST_BLOCK* Srb, BOOLEAN* complete) {

	UNREFERENCED_PARAMETER(driverCtx);
	UNREFERENCED_PARAMETER(Srb);
	UNREFERENCED_PARAMETER(complete);

	return SRB_STATUS_INVALID_REQUEST;
}