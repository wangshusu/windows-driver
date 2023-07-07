#include "wdm-driver.h"


// ����������ں���
extern "C"
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, 
    _In_ PUNICODE_STRING RegistryPath) 
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    UNICODE_STRING deviceName, symLinkName;
    PDEVICE_EXTENSION deviceExtension;

    // �����豸����
    RtlInitUnicodeString(&deviceName, L"\\Device\\MyDriver");

    // �����豸����
    status = IoCreateDevice(DriverObject, 
        sizeof(DEVICE_EXTENSION), 
        &deviceName, 
        FILE_DEVICE_UNKNOWN, 
        0, 
        FALSE, 
        &deviceObject
    );
    if (!NT_SUCCESS(status)) {
        // �����豸���󴴽�ʧ�ܵ����
        return status;
    }

    // ����������������
    RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\MyDriver");

    // ������������
    status = IoCreateSymbolicLink(&symLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        // ����������Ӵ���ʧ�ܵ����
        IoDeleteDevice(deviceObject);
        return status;
    }
    
    // ��ʼ���豸��չ�ṹ
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->DeviceName = deviceName;
    deviceExtension->SymLinkName = symLinkName;

    // ע����������ж�ػص�����
    DriverObject->DriverUnload = UnloadDriver;

    // ע���豸I/O��������
    DriverObject->MajorFunction[IRP_MJ_CREATE] = HandleCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = HandleCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleDeviceControl;

    // ���һ����Ϣ��������
    DbgPrint("WDM���������Ѽ���\n");

    return STATUS_SUCCESS;
}

// ��������ж�ػص�����
VOID UnloadDriver(_In_ PDRIVER_OBJECT DriverObject) 
{

    PDEVICE_EXTENSION deviceExtension = (PDEVICE_EXTENSION)DriverObject->DeviceObject->DeviceExtension;

    // ɾ����������
    IoDeleteSymbolicLink(&deviceExtension->SymLinkName);

    // ɾ���豸����
    IoDeleteDevice(deviceExtension->DeviceObject);

    // ���һ����Ϣ��������
    DbgPrint("WDM����������ж��\n");
}

// �������͹رղ���
NTSTATUS HandleCreateClose(_In_ PDEVICE_OBJECT DeviceObject, 
    _In_ PIRP Irp) 
{
    UNREFERENCED_PARAMETER(DeviceObject);

    // ���IRP����
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

// �����豸���Ʋ���
NTSTATUS HandleDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, 
    _In_ PIRP Irp) 
{
    UNREFERENCED_PARAMETER(DeviceObject);

    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stackLocation;
    ULONG controlCode;

    stackLocation = IoGetCurrentIrpStackLocation(Irp);
    controlCode = stackLocation->Parameters.DeviceIoControl.IoControlCode;

    // ���ݿ�����ִ����Ӧ����
    switch (controlCode) {
        // �����Զ��������

    default:
        // δ֪�����룬���ش���
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    // ���IRP����
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}
