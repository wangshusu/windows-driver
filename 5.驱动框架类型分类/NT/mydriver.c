#include <ntddk.h>

typedef struct _DEVICE_EXTENSION {
    UNICODE_STRING DeviceName;  // 存储设备名
    UNICODE_STRING SymbolicLinkName;    // 存储符号链接名
    ULONG DeviceData;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

PDEVICE_OBJECT deviceObject;

NTSTATUS DeviceCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
    // 处理设备创建和关闭请求

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DeviceIoControl(_In_ PDEVICE_OBJECT pDeviceObject, _In_ PIRP Irp) {
    // 处理设备IO控制请求

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID Dump(_In_ PDRIVER_OBJECT pDeviceObject) {
    KdPrint(("-----------------------------------------------------------\n"));
    KdPrint(("Begin Dump..........\n"));

    // 打印调试信息
    KdPrint(("Driver Address:%p\n", pDeviceObject));
    KdPrint(("Driver Name   :%S\n", pDeviceObject->DriverName.Buffer));
    KdPrint(("Driver Name   :%S\n", pDeviceObject->HardwareDatabase->Buffer));
    KdPrint(("Driver First pDevice:%p\n", pDeviceObject->DeviceObject));

    // 得到设备对象
    int i = 1;
    PDEVICE_OBJECT pDevice = pDeviceObject->DeviceObject;
    for (; pDevice != NULL; pDevice = pDevice->NextDevice)
    {
        KdPrint(("Driver %d device\n", i++));
        KdPrint(("Driver AttachedDevice:%p\n", pDevice->AttachedDevice));
        KdPrint(("Driver NextDevice    :%p\n", pDevice->NextDevice));
        KdPrint(("Driver StackSize     :%d\n", pDevice->StackSize));
        KdPrint(("Driver DriverObject  :%p\n", pDevice->DriverObject));
    }

    KdPrint(("Begin Over..........\n"));
    KdPrint(("-----------------------------------------------------------\n"));
}

VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject) {
    // 执行清理和资源释放操作

    // 从设备拓展信息中取回设备名称和符号链接名称
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;
    UNICODE_STRING deviceName = deviceExtension->DeviceName;
    UNICODE_STRING symbolicLinkName = deviceExtension->SymbolicLinkName;

    // 删除符号链接
    IoDeleteSymbolicLink(&symbolicLinkName);

    // 删除设备对象
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    // 释放设备名称和符号链接名称内存
    ExFreePool(deviceName.Buffer);
    ExFreePool(symbolicLinkName.Buffer);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    // 创建设备拓展信息并分配内存
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_EXTENSION), 'MyEx');
    if (deviceExtension == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 初始化设备名称
    RtlInitUnicodeString(&deviceExtension->DeviceName, L"\\Device\\MyDevice");
    RtlInitUnicodeString(&DriverObject->DriverName, L"\\Device\\MyDevice");

    // 初始化符号链接名称
    RtlInitUnicodeString(&deviceExtension->SymbolicLinkName, L"\\DosDevices\\MyDevice");

    // 创建设备对象
    status = IoCreateDevice(DriverObject, 
        sizeof(DEVICE_EXTENSION), 
        NULL, 
        FILE_DEVICE_UNKNOWN, 
        FILE_DEVICE_SECURE_OPEN, 
        FALSE, 
        &deviceObject);
    if (!NT_SUCCESS(status)) {
        ExFreePool(deviceExtension);
        return status;
    }

    // 将设备拓展信息存储在设备对象的拓展存储信息中
    DriverObject->DeviceObject->DeviceExtension = deviceExtension;

    // 设置设备对象处理函数
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;

    // 设置卸载函数
    DriverObject->DriverUnload = UnloadDriver;

    // 创建符号链接
    status = IoCreateSymbolicLink(&deviceExtension->SymbolicLinkName, &deviceExtension->DeviceName);

    // 其他初始化和处理逻辑

    Dump(DriverObject)

    return status;
}
