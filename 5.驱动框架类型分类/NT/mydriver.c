#include <ntddk.h>

typedef struct _DEVICE_EXTENSION {
    UNICODE_STRING DeviceName;          // 存储设备名
    UNICODE_STRING SymbolicLinkName;    // 存储符号链接名
    ULONG DeviceData;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

PDEVICE_OBJECT deviceObject = NULL;

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
    RtlFreeUnicodeString(&deviceName);
    RtlFreeUnicodeString(&symbolicLinkName);
    ExFreePool(deviceExtension);
}

VOID Dump(_In_ PDRIVER_OBJECT DriverObject) {
    KdPrint(("-----------------------------------------------------------\n"));
    KdPrint(("Begin Dump..........\n"));

    // 打印驱动程序信息
    KdPrint(("Driver Address:%p\n", DriverObject));
    KdPrint(("Driver Name   :%S\n", DriverObject->DriverName.Buffer));

    // 打印设备对象信息
    int i = 1;
    PDEVICE_OBJECT pDevice = DriverObject->DeviceObject;
    for (; pDevice != NULL; pDevice = pDevice->NextDevice) {
        KdPrint(("Device %d:\n", i++));
        KdPrint(("    Device Address        : %p\n", pDevice));
        KdPrint(("    AttachedDevice        : %p\n", pDevice->AttachedDevice));
        KdPrint(("    NextDevice            : %p\n", pDevice->NextDevice));
        KdPrint(("    StackSize             : %d\n", pDevice->StackSize));
        KdPrint(("    DeviceExtension       : %p\n", pDevice->DeviceExtension));
        // 打印其他设备对象信息...
    }

    KdPrint(("Begin Over..........\n"));
    KdPrint(("-----------------------------------------------------------\n"));
}


NTSTATUS DeepCopyUnicodeString(PUNICODE_STRING dest, PCUNICODE_STRING src) {
    dest->Length = src->Length;
    dest->MaximumLength = src->Length + sizeof(WCHAR); // 加上null终止符的空间
    dest->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool, dest->MaximumLength, 'MyEx');
    if (dest->Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(dest->Buffer, src->Buffer, src->Length);
    dest->Buffer[src->Length / sizeof(WCHAR)] = L'\0'; // 添加null终止符

    return STATUS_SUCCESS;
}


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    // 创建设备拓展信息并分配内存
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_EXTENSION), 'MyEx');
    if (deviceExtension == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // 初始化设备名称
    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName, L"\\Device\\MyDevice");
    status = DeepCopyUnicodeString(&deviceExtension->DeviceName, &deviceName);
    if (!NT_SUCCESS(status)) {
        // 处理内存分配失败的情况
        ExFreePool(deviceExtension);
        return status;
    }

    // 初始化符号链接名称
    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\MyDevice");
    status = DeepCopyUnicodeString(&deviceExtension->SymbolicLinkName, &symbolicLinkName);
    if (!NT_SUCCESS(status)) {
        // 处理内存分配失败的情况
        RtlFreeUnicodeString(&deviceExtension->DeviceName);
        ExFreePool(deviceExtension);
        return status;
    }

    // 创建设备对象
    status = IoCreateDevice(DriverObject,
        sizeof(DEVICE_EXTENSION),
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject);
    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(&deviceExtension->SymbolicLinkName);
        RtlFreeUnicodeString(&deviceExtension->DeviceName);
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
    status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        RtlFreeUnicodeString(&deviceExtension->SymbolicLinkName);
        RtlFreeUnicodeString(&deviceExtension->DeviceName);
        ExFreePool(deviceExtension);
        return status;
    }

    // 其他初始化和处理逻辑

    Dump(DriverObject);

    return STATUS_SUCCESS;
}
