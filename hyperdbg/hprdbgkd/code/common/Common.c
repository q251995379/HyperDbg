/**
 * @file Common.c
 * @author Sina Karvandi (sina@hyperdbg.org)
 * @brief Routines for common tasks in debugger
 * @details
 * @version 0.2
 * @date 2023-01-22
 *
 * @copyright This project is released under the GNU Public License v3.
 *
 */
#include "pch.h"

/**
 * @brief Checks whether the process with ProcId exists or not
 *
 * @details this function should NOT be called from vmx-root mode
 *
 * @param UINT32 ProcId
 * @return BOOLEAN Returns true if the process
 * exists and false if it the process doesn't exist
 */
BOOLEAN
IsProcessExist(UINT32 ProcId)
{
    PEPROCESS TargetEprocess;
    CR3_TYPE  CurrentProcessCr3 = {0};

    if (PsLookupProcessByProcessId(ProcId, &TargetEprocess) != STATUS_SUCCESS)
    {
        //
        // There was an error, probably the process id was not found
        //
        return FALSE;
    }
    else
    {
        ObDereferenceObject(TargetEprocess);

        return TRUE;
    }
}

/**
 * @brief Get process name by eprocess
 *
 * @param Eprocess Process eprocess
 * @return PCHAR Returns a pointer to the process name
 */
PCHAR
GetProcessNameFromEprocess(PEPROCESS Eprocess)
{
    PCHAR Result = 0;

    //
    // We can't use PsLookupProcessByProcessId as in pageable and not
    // work on vmx-root
    //
    Result = (CHAR *)PsGetProcessImageFileName(Eprocess);

    return Result;
}

/**
 * @brief Kill a user-mode process with different methods
 * @param ProcessId
 * @param KillingMethod
 *
 * @return BOOLEAN
 */
_Use_decl_annotations_
BOOLEAN
KillProcess(UINT32 ProcessId, PROCESS_KILL_METHODS KillingMethod)
{
    NTSTATUS  Status        = STATUS_SUCCESS;
    HANDLE    ProcessHandle = NULL;
    PEPROCESS Process       = NULL;

    if (ProcessId == NULL)
    {
        return FALSE;
    }

    switch (KillingMethod)
    {
    case PROCESS_KILL_METHOD_1:

        Status = GetHandleFromProcess(ProcessId, &ProcessHandle);

        if (!NT_SUCCESS(Status) || ProcessHandle == NULL)
        {
            return FALSE;
        }

        //
        // Call ZwTerminateProcess with NULL handle
        //
        Status = ZwTerminateProcess(ProcessHandle, 0);

        if (!NT_SUCCESS(Status))
        {
            return FALSE;
        }

        break;

    case PROCESS_KILL_METHOD_2:

        UndocumentedNtOpenProcess(
            &ProcessHandle,
            PROCESS_ALL_ACCESS,
            ProcessId,
            KernelMode);

        if (ProcessHandle == NULL)
        {
            return FALSE;
        }

        //
        // Call ZwTerminateProcess with NULL handle
        //
        Status = ZwTerminateProcess(ProcessHandle, 0);

        if (!NT_SUCCESS(Status))
        {
            return FALSE;
        }

        break;

    case PROCESS_KILL_METHOD_3:

        //
        // Get the base address of process's executable image and unmap it
        //
        Status = MmUnmapViewOfSection(Process, PsGetProcessSectionBaseAddress(Process));

        //
        // Dereference the target process
        //
        ObDereferenceObject(Process);

        break;

    default:

        //
        // Unknow killing method
        //
        return FALSE;
        break;
    }

    //
    // If we reached here, it means the functionality of
    // the above codes was successful
    //
    return TRUE;
}
