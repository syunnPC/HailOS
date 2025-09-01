#include "status.h"
#include "basetype.h"
#include "common.h"

const char* StatusToString(HOSstatus Status)
{
    switch(Status)
    {
        case STATUS_SUCCESS:
            return "STATUS_SUCCESS";
        case STATUS_ALREADY_SATISFIED:
            return "STATUS_ALREADY_SATISFIED";
        case STATUS_ERROR:
            return "STATUS_ERROR";
        case STATUS_UNSUPPORTED:
            return "STATUS_UNSUPPORTED";
        case STATUS_NOT_IMPLEMENTED:
            return "STATUS_NOT_IMPLEMENTED";
        case STATUS_OUT_OF_RANGE:
            return "STATUS_OUT_OF_RANGE";
        case STATUS_NOT_INITIALIZED:
            return "STATUS_NOT_INITIALIZED";
        case STATUS_NOT_READY:
            return "STATUS_NOT_READY";
        case STATUS_NOT_FOUND:
            return "STATUS_NOT_FOUND";
        case STATUS_INVALID_PARAMETER:
            return "STATUS_INVALID_PARAMETER";
        case STATUS_MEMORY_ALLOCATION_FAILED:
            return "STATUS_MEMORY_ALLOCATION_FAILED";
        case STATUS_BUFFER_TOO_SMALL:
            return "STATUS_BUFFER_TOO_SMALL";
        case STATUS_NOT_AVAILABLE:
            return "STATUS_NOT_AVAILABLE";
        case STATUS_ASSERTION_FAILED:
            return "STATUS_ASSERTION_FAILED";
        case STATUS_NO_MEMORY_AVAILABLE:
            return "STATUS_NO_MEMORY_AVAILABLE";
        case STAUTS_HARDWARE_ERROR:
            return "STATUS_HARDWARE_ERROR";
        case STATUS_DISK_IO_ERROR:
            return "STATUS_DISK_IO_ERROR";
        case STATUS_IO_ERROR:
            return "STATUS_IO_ERROR";
        case STATUS_GENERAL_PROTECTION_FAULT:
            return "STATUS_GENERAL_PROTECTION_FAULT";
        case STATUS_HAL_ERROR:
            return "STATUS_HAL_ERROR";
        case STATUS_FAT32_FILESYSTEM:
            return "STATUS_FAT32_FILESYSTEM";
        default: 
            return NULL;
    }
}