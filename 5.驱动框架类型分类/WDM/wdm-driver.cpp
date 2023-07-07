#include "wdm-driver.h"


// 驱动程序入口函数
extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, 
    _In_ PUNICODE_STRING RegistryPath) 
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    UNICODE_STRING deviceName, symLinkName;
    PDEVICE_EXTENSION deviceExtension;

    // 创建设备名称
    RtlInitUnicodeString(&deviceName, L"\\Device\\MyDriver");

    // 创建设备对象
    status = IoCreateDevice(DriverObject, 
        sizeof(DEVICE_EXTENSION), 
        &deviceName, 
        FILE_DEVICE_UNKNOWN, 
        0, 
        FALSE, 
        &deviceObject
    );
    if (!NT_SUCCESS(status)) {
        // 处理设备对象创建失败的情况
        return status;
    }

    // 创建符号链接名称
    RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\MyDriver");

    // 创建符号链接
    status = IoCreateSymbolicLink(&symLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        // 处理符号链接创建失败的情况
        IoDeleteDevice(deviceObject);
        return status;
    }
    
    // 初始化设备扩展结构
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->DeviceName = deviceName;
    deviceExtension->SymLinkName = symLinkName;

    // 注册驱动程序卸载回调函数
    DriverObject->DriverUnload = UnloadDriver;

    // 注册设备I/O操作函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = HandleCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = HandleCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleDeviceControl;

    // 输出一条消息到调试器
    DbgPrint("WDM驱动程序已加载\n");

    return STATUS_SUCCESS;
}

// 驱动程序卸载回调函数
VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject) 
{

    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

    // 删除符号链接
    IoDeleteSymbolicLink(&deviceExtension->SymLinkName);

    // 删除设备对象
    IoDeleteDevice(deviceExtension->DeviceObject);

    // 输出一条消息到调试器
    DbgPrint("WDM驱动程序已卸载\n");
}

// 处理创建和关闭操作
NTSTATUS HandleCreateClose(_In_ PDEVICE_OBJECT DeviceObject, 
    _In_ PIRP Irp) 
{
    UNREFERENCED_PARAMETER(DeviceObject);

    // 完成IRP请求
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// 处理设备控制操作
NTSTATUS HandleDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, 
    _In_ PIRP Irp) 
{
    UNREFERENCED_PARAMETER(DeviceObject);

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stackLocation;
    ULONG controlCode;

    stackLocation = IoGetCurrentIrpStackLocation(Irp);
    controlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;

    // 根据控制码执行相应操作
    switch (controlCode) {
        // 处理自定义控制码

    default:
        // 未知控制码，返回错误
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    // 完成IRP请求
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}
