#include <windows.h>
#include <strsafe.h>
#include "log.h"

// Implementation for INCALESCENT_LogRawW
HRESULT INCALESCENT_LogRawW(PWSTR message, const SIZE_T messageSize) {
    HRESULT result = S_OK;

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    DWORD written;
    BOOL writeResult = WriteConsoleW(console, message, messageSize, &written, NULL);
    if (!writeResult) {
        result = HRESULT_FROM_WIN32(GetLastError());
    }

    cleanup:
    return result;
}

// Implementation for INCALESCENT_LogFormattedW
HRESULT INCALESCENT_LogFormattedW(PWSTR levelString, PWSTR format, ...) {
    va_list parameters;
    va_start(parameters, format);
    HRESULT result = S_OK;

    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console == INVALID_HANDLE_VALUE) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    WCHAR suppliedMessage[INCALESCENT_LOG_MAX_MESSAGE_LENGTH];
    result = StringCchVPrintfW(suppliedMessage, INCALESCENT_LOG_MAX_MESSAGE_LENGTH, format, parameters);
    va_end(parameters);
    if (FAILED(result)) {
        goto cleanup;
    }

    SYSTEMTIME time;
    GetLocalTime(&time);

    WCHAR logMessage[INCALESCENT_LOG_MAX_MESSAGE_LENGTH + INCALESCENT_LOG_MAX_HEADER_LENGTH];
    result = StringCchPrintfW(
            logMessage,
            INCALESCENT_LOG_MAX_MESSAGE_LENGTH,
            INCALESCENT_LOG_FORMAT_W,
            time.wMonth,
            time.wDay,
            time.wYear,
            time.wHour,
            time.wMinute,
            time.wSecond,
            levelString,
            suppliedMessage
    );
    if (FAILED(result)) {
        goto cleanup;
    }

    SIZE_T logMessageSize;
    result = StringCchLengthW(logMessage, INCALESCENT_LOG_MAX_MESSAGE_LENGTH, &logMessageSize);
    if (FAILED(result)) {
        goto cleanup;
    }

    DWORD charactersWritten;
    BOOL writeResult = WriteConsoleW(console, logMessage, logMessageSize, &charactersWritten, NULL);
    if (!writeResult) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    cleanup:
    return result;
}

// Implementation for INCALESCENT_LogFormattedErrorResultW
HRESULT INCALESCENT_LogFormattedErrorResultW(HRESULT failedResult) {
    HRESULT result = S_OK;
    PWSTR errorMessage = NULL;
    DWORD formatResult = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            failedResult,
            0,
            (LPWSTR) &errorMessage,
            0,
            NULL
    );
    if (formatResult == 0) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    result = INCALESCENT_LogFormattedW(L"Error", L"Encountered error (0x%x): %s\n", failedResult, errorMessage);

    cleanup:
    return result;
}
