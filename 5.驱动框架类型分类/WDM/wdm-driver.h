#include <ntddk.h>

// 设备扩展结构
typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT DeviceObject;    // 设备对象
	UNICODE_STRING DeviceName;      // 设备名称
	UNICODE_STRING SymLinkName;     // 符号链接名称
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

// 驱动程序卸载回调函数
VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject);

// 处理创建和关闭操作
NTSTATUS HandleCreateClose(_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp);

// 处理设备控制操作
NTSTATUS HandleDeviceControl(_In_ PDEVICE_OBJECT DeviceObject,
	_In_ PIRP Irp);

// 驱动程序入口函数
extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath);
