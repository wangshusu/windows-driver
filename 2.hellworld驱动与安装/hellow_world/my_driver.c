#include <ntddk.h>

NTSTATUS DriverUnload(
	_In_ PDRIVER_OBJECT pDriverObject
)
{
	UNREFERENCED_PARAMETER(pDriverObject);

	DbgPrint("Bybe, world!!\n");
}


NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT pDriverObject,
	_In_ PUNICODE_STRING pRegistryPath
)
{
	// ��֪���������������ں������ڲ�δʹ��
	UNREFERENCED_PARAMETER(pRegistryPath);

	DbgPrint("Hello, world!!\n");


	pDriverObject->DriverUnload = DriverUnload;

	// ���÷���״̬
	return STATUS_SUCCESS;
}