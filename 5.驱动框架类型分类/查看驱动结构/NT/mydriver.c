#include <ntddk.h>

typedef struct _DEVICE_EXTENSION {
    UNICODE_STRING DeviceName;          // �洢�豸��
    UNICODE_STRING SymbolicLinkName;    // �洢����������
    ULONG DeviceData;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

PDEVICE_OBJECT deviceObject = NULL;

NTSTATUS DeviceCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
    // �����豸�����͹ر�����

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DeviceIoControl(_In_ PDEVICE_OBJECT pDeviceObject, _In_ PIRP Irp) {
    // �����豸IO��������

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject) {
    // ִ���������Դ�ͷŲ���

    // ���豸��չ��Ϣ��ȡ���豸���ƺͷ�����������
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;
    UNICODE_STRING deviceName = deviceExtension->DeviceName;
    UNICODE_STRING symbolicLinkName = deviceExtension->SymbolicLinkName;

    // ɾ����������
    IoDeleteSymbolicLink(&symbolicLinkName);

    // ɾ���豸����
    if (DriverObject->DeviceObject) {
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    // �ͷ��豸���ƺͷ������������ڴ�
    RtlFreeUnicodeString(&deviceName);
    RtlFreeUnicodeString(&symbolicLinkName);
    ExFreePool(deviceExtension);
}

VOID Dump(_In_ PDRIVER_OBJECT DriverObject) {
    KdPrint(("-----------------------------------------------------------\n"));
    KdPrint(("Begin Dump..........\n"));

    // ��ӡ����������Ϣ
    KdPrint(("Driver Address:%p\n", DriverObject));
    KdPrint(("Driver Name   :%S\n", DriverObject->DriverName.Buffer));

    // ��ӡ�豸������Ϣ
    int i = 1;
    PDEVICE_OBJECT pDevice = DriverObject->DeviceObject;
    for (; pDevice != NULL; pDevice = pDevice->NextDevice) {
        KdPrint(("Device %d:\n", i++));
        KdPrint(("    Device Address        : %p\n", pDevice));
        KdPrint(("    AttachedDevice        : %p\n", pDevice->AttachedDevice));
        KdPrint(("    NextDevice            : %p\n", pDevice->NextDevice));
        KdPrint(("    StackSize             : %d\n", pDevice->StackSize));
        KdPrint(("    DeviceExtension       : %p\n", pDevice->DeviceExtension));
        // ��ӡ�����豸������Ϣ...
    }

    KdPrint(("Begin Over..........\n"));
    KdPrint(("-----------------------------------------------------------\n"));
}


NTSTATUS DeepCopyUnicodeString(PUNICODE_STRING dest, PCUNICODE_STRING src) {
    dest->Length = src->Length;
    dest->MaximumLength = src->Length + sizeof(WCHAR); // ����null��ֹ���Ŀռ�
    dest->Buffer = (PWSTR)ExAllocatePoolWithTag(NonPagedPool, dest->MaximumLength, 'MyEx');
    if (dest->Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(dest->Buffer, src->Buffer, src->Length);
    dest->Buffer[src->Length / sizeof(WCHAR)] = L'\0'; // ���null��ֹ��

    return STATUS_SUCCESS;
}


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    // �����豸��չ��Ϣ�������ڴ�
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_EXTENSION), 'MyEx');
    if (deviceExtension == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // ��ʼ���豸����
    UNICODE_STRING deviceName;
    RtlInitUnicodeString(&deviceName, L"\\Device\\MyDevice");
    status = DeepCopyUnicodeString(&deviceExtension->DeviceName, &deviceName);
    if (!NT_SUCCESS(status)) {
        // �����ڴ����ʧ�ܵ����
        ExFreePool(deviceExtension);
        return status;
    }

    // ��ʼ��������������
    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\MyDevice");
    status = DeepCopyUnicodeString(&deviceExtension->SymbolicLinkName, &symbolicLinkName);
    if (!NT_SUCCESS(status)) {
        // �����ڴ����ʧ�ܵ����
        RtlFreeUnicodeString(&deviceExtension->DeviceName);
        ExFreePool(deviceExtension);
        return status;
    }

    // �����豸����
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

    // ���豸��չ��Ϣ�洢���豸�������չ�洢��Ϣ��
    DriverObject->DeviceObject->DeviceExtension = deviceExtension;

    // �����豸��������
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;

    // ����ж�غ���
    DriverObject->DriverUnload = UnloadDriver;

    // ������������
    status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        RtlFreeUnicodeString(&deviceExtension->SymbolicLinkName);
        RtlFreeUnicodeString(&deviceExtension->DeviceName);
        ExFreePool(deviceExtension);
        return status;
    }

    // ������ʼ���ʹ����߼�

    Dump(DriverObject);

    return STATUS_SUCCESS;
}
