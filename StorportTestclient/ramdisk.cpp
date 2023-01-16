#include <Windows.h>
#include <SetupAPI.h>
#include <string>
#include "structs.h"
#include <ntddscsi.h>

using std::string;

bool FindPhysDriverPath(string TargetHWID, const GUID* DriverClass, string& PhysPath) {

	SP_DEVICE_INTERFACE_DATA devInterfaceData = { 0 };
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	auto diHandle = SetupDiGetClassDevsA(DriverClass, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	SP_DEVINFO_DATA deviceinfo;
	deviceinfo.cbSize = sizeof(deviceinfo);

	int i = 0;
	while (SetupDiEnumDeviceInfo(diHandle, i++, &deviceinfo)) {
		DWORD bufSz;
		if (!SetupDiGetDeviceRegistryPropertyA(diHandle, &deviceinfo, SPDRP_HARDWAREID, 0, 0, 0, &bufSz) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) continue;

		char* buffer = new char[bufSz];
		SetupDiGetDeviceRegistryPropertyA(diHandle, &deviceinfo, SPDRP_HARDWAREID, 0, (PBYTE)buffer, bufSz, 0);
		string hwid(buffer);
		delete[] buffer;

		if (TargetHWID.compare(hwid) == 0) {
			SetupDiGetDeviceRegistryPropertyA(diHandle, &deviceinfo, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, 0, 0, 0, &bufSz);
			string filepath(bufSz - 1, '\0');
			SetupDiGetDeviceRegistryPropertyA(diHandle, &deviceinfo, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME, 0, (PBYTE)filepath.c_str(), bufSz, 0);
			PhysPath = "\\\\?\\GLOBALROOT";
			PhysPath += filepath;
			return true;
		}
	}
	return false;
}

HANDLE hDriver = INVALID_HANDLE_VALUE;

int main() {

	string physPath;
	if (!FindPhysDriverPath("Root\\StorportDriver", &GUID_DEVINTERFACE_STORAGEPORT, physPath)) {
		printf("not found!\n");
		return 0;
	}

	hDriver = CreateFileA(physPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

	if (hDriver == INVALID_HANDLE_VALUE) printf("invalid handle\n");

	CommunicationTest tIn = { 0 };
	CommunicationTest tOut = { 0 };

	strcpy_s(tIn.message, "dinguspingus_from_usermode");

	DWORD recvd;
	bool success = DeviceIoControl(hDriver, IOCTL_MINIPORT_PROCESS_SERVICE_IRP, &tIn, sizeof(tIn), &tOut, sizeof(tOut), &recvd, 0);

	printf("driver returned: '%s' ok: %d, last error: %d\n", tOut.message, success, GetLastError());

	system("pause");

	return 0;
}