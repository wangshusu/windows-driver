#include <ntddk.h>

#define IOCTL_MYDRIVER_GET_VERSION CTL_CODE(FILE_DEVICE_UNKNOWN, \
											0x800, \
											METHOD_BUFFERED, \
											FILE_ANY_ACCESS)

#define DEVICR_NAME		L"\\Device\\MyDriver"
#define SYMBOLIC_NAME   L"\\DosDevices\\MyDriver"

PDEVICE_OBJECT g_DeviceObject = NULL;

NTSTATUS CreateCloseDispatch
(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	// 该参数未使用
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DeviceControlDispatch
(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG controlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	switch (controlCode)
	{
		// 处理设备控制码
		case IOCTL_MYDRIVER_GET_VERSION:
		{
			break;
		}
		default:
		{
			//未知的控制码
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symbolicLinkName;

	// 删除符号链接
	RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_NAME);
	IoDeleteSymbolicLink(&symbolicLinkName);

	// 删除设备对象
	if (g_DeviceObject != NULL)
	{
		IoDeleteDevice(g_DeviceObject);
		g_DeviceObject = NULL;
	}
}


NTSTATUS DriverEntry
(
	_In_ PDRIVER_OBJECT DriverObject, 
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName, symbolicLinkName;

	// 创建驱动对象
	status = IoCreateDevice(DriverObject, 
							0, 
							&deviceName, 
							FILE_DEVICE_UNKNOWN, 
							FILE_DEVICE_SECURE_OPEN, 
							FALSE, 
							&g_DeviceObject);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	// 创建符号链接
	RtlInitUnicodeString(&symbolicLinkName, SYMBOLIC_NAME);
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(g_DeviceObject);
		return status;
	}

	// 初始化驱动对象
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlDispatch;
	DriverObject->DriverUnload = UnloadDriver;

	return status;
}