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
	// 告知驱动这个输入参数在函数体内部未使用
	UNREFERENCED_PARAMETER(pRegistryPath);

	DbgPrint("Hello, world!!\n");


	pDriverObject->DriverUnload = DriverUnload;

	// 设置返回状态
	return STATUS_SUCCESS;
}