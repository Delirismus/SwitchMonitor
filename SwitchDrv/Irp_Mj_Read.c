
#include "DominateCommand.h"
#include "Irp_Mj_Read.h"
FLT_PREOP_CALLBACK_STATUS IrpMjReadPre(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (IrpMjReadDoRequestOperationStatus(Data))
	{
        status = FltRequestOperationStatusCallback(Data,IrpMjReadOperationStatusCallback,NULL);
        if (!NT_SUCCESS(status))
		{
            
        }
    }
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

VOID IrpMjReadOperationStatusCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_IO_PARAMETER_BLOCK ParameterSnapshot,
    _In_ NTSTATUS OperationStatus,
    _In_ PVOID RequesterContext)
{
    UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(ParameterSnapshot);
	UNREFERENCED_PARAMETER(OperationStatus);
	UNREFERENCED_PARAMETER(RequesterContext);
    //PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,("SwitchDrv!SwitchDrvOperationStatusCallback: Entered\n"));

    //PT_DBG_PRINT(PTDBG_TRACE_OPERATION_STATUS,
    //              ("SwitchDrv!SwitchDrvOperationStatusCallback: Status=%08x ctx=%p IrpMj=%02x.%02x \"%s\"\n",
    //               OperationStatus,
    //               RequesterContext,
    //               ParameterSnapshot->MajorFunction,
    //               ParameterSnapshot->MinorFunction,
    //               FltGetIrpName(ParameterSnapshot->MajorFunction)));
}


FLT_POSTOP_CALLBACK_STATUS IrpMjReadPost(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);
    //PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,("SwitchDrv!SwitchDrvPostOperation: Entered\n"));
    return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS IrpMjReadOperationNoPostOperation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( FltObjects );
    UNREFERENCED_PARAMETER( CompletionContext );
    //PT_DBG_PRINT(PTDBG_TRACE_ROUTINES,("SwitchDrv!SwitchDrvPreOperationNoPostOperation: Entered\n"));
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


BOOLEAN IrpMjReadDoRequestOperationStatus(_In_ PFLT_CALLBACK_DATA Data)
{
    PFLT_IO_PARAMETER_BLOCK iopb = Data->Iopb;
    return (BOOLEAN)

            //
            //  Check for oplock operations
            //

             (((iopb->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
               ((iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_FILTER_OPLOCK)  ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_BATCH_OPLOCK)   ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_1) ||
                (iopb->Parameters.FileSystemControl.Common.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2)))

              ||

              //
              //    Check for directy change notification
              //

              ((iopb->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
               (iopb->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY))
             );
}