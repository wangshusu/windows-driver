#include <ntddk.h>

typedef struct _DEVICE_EXTENSION {
    UNICODE_STRING DeviceName;  // �洢�豸��
    UNICODE_STRING SymbolicLinkName;    // �洢����������
    ULONG DeviceData;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

PDEVICE_OBJECT deviceObject;

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

VOID Dump(_In_ PDRIVER_OBJECT pDeviceObject) {
    KdPrint(("-----------------------------------------------------------\n"));
    KdPrint(("Begin Dump..........\n"));

    // ��ӡ������Ϣ
    KdPrint(("Driver Address:%p\n", pDeviceObject));
    KdPrint(("Driver Name   :%S\n", pDeviceObject->DriverName.Buffer));
    KdPrint(("Driver Name   :%S\n", pDeviceObject->HardwareDatabase->Buffer));
    KdPrint(("Driver First pDevice:%p\n", pDeviceObject->DeviceObject));

    // �õ��豸����
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
    ExFreePool(deviceName.Buffer);
    ExFreePool(symbolicLinkName.Buffer);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    // �����豸��չ��Ϣ�������ڴ�
    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)ExAllocatePoolWithTag(NonPagedPool, sizeof(DEVICE_EXTENSION), 'MyEx');
    if (deviceExtension == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // ��ʼ���豸����
    RtlInitUnicodeString(&deviceExtension->DeviceName, L"\\Device\\MyDevice");
    RtlInitUnicodeString(&DriverObject->DriverName, L"\\Device\\MyDevice");

    // ��ʼ��������������
    RtlInitUnicodeString(&deviceExtension->SymbolicLinkName, L"\\DosDevices\\MyDevice");

    // �����豸����
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

    // ���豸��չ��Ϣ�洢���豸�������չ�洢��Ϣ��
    DriverObject->DeviceObject->DeviceExtension = deviceExtension;

    // �����豸��������
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DeviceCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControl;

    // ����ж�غ���
    DriverObject->DriverUnload = UnloadDriver;

    // ������������
    status = IoCreateSymbolicLink(&deviceExtension->SymbolicLinkName, &deviceExtension->DeviceName);

    // ������ʼ���ʹ����߼�

    Dump(DriverObject)

    return status;
}
