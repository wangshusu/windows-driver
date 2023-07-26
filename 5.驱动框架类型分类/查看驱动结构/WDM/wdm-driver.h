#include <ntddk.h>

// �豸��չ�ṹ
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT DeviceObject;    // �豸����
	UNICODE_STRING DeviceName;      // �豸����
	UNICODE_STRING SymLinkName;     // ������������
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

// ��������ж�ػص�����
VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject);

// �������͹رղ���
NTSTATUS HandleCreateClose(_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp);

// �����豸���Ʋ���
NTSTATUS HandleDeviceControl(_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp);

// ����������ں���
extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath);
