/*
 * The MIT License
 *
 * Copyright (c) 2023 Daniel Landeros
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <windows.h>
#include <strsafe.h>
#include "file.h"
#include "log.h"
#include "generated_error.h"

#define CLAMP_POSITIVE(value, maximumValue) ((value) > (maximumValue) ? (maximumValue) : (value))

// Implementation for INCALESCENT_File_ReadTemperature
HRESULT INCALESCENT_File_ReadTemperature(PWSTR path, WCHAR value[INCALESCENT_FILE_TEMPERATURE_FIELD_VALUE_MAX_LENGTH]) {
    BYTE buffer[INCALESCENT_FILE_TEMPERATURE_RAW_BUFFER_SIZE];
    HRESULT result = S_OK;
    PWSTR string = NULL;
    BOOL foundValue = FALSE;

    HANDLE file = CreateFileW(
            path,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_READONLY,
            NULL
    );
    if (file == INVALID_HANDLE_VALUE) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    DWORD readCount = 0;
    BOOL readResult = ReadFile(file, buffer, INCALESCENT_FILE_TEMPERATURE_RAW_BUFFER_SIZE, &readCount, NULL);
    if (!readResult) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    INT wideCount = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) buffer, INCALESCENT_FILE_TEMPERATURE_RAW_BUFFER_SIZE, NULL,
                                        0);
    if (wideCount == 0) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }
    wideCount = CLAMP_POSITIVE(wideCount, INCALESCENT_FILE_TEMPERATURE_CHARACTER_BUFFER_MAX_LENGTH);

    string = VirtualAlloc(NULL, wideCount, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (string == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    wideCount = MultiByteToWideChar(CP_UTF8, 0, (LPCCH) buffer, INCALESCENT_FILE_TEMPERATURE_RAW_BUFFER_SIZE, string,
                                    wideCount);
    if (wideCount == 0) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    SIZE_T matchCount = 0;
    SIZE_T valueIndex = 0;

    for (SIZE_T index = 0; index < wideCount; index++) {
        WCHAR character = string[index];

        // If there was a perfect match of characters we can start adding them
        // to the value buffer.
        if (matchCount == INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING_LENGTH) {
            // The '\r' character denotes the end of the field's value.
            if (character == '\r') {
                value[valueIndex] = '\0';
                foundValue = TRUE;
                break;
            }

            // If length exceeds maximum this value is invalid. The "- 1" is for the null-terminating character.
            if (valueIndex == (INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING_LENGTH - 1)) {
                result = INCALESCENT_ERROR_FIELD_VALUE_TOO_LARGE;
                goto cleanup;
            }

            value[valueIndex] = character;
            valueIndex++;
            continue;
        }

        // Determine if the character is a match, if not reset and try again
        if (character == INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING[matchCount]) {
            matchCount++;
        } else {
            matchCount = 0;
        }
    }

    if (!foundValue) {
        result = INCALESCENT_ERROR_FIELD_VALUE_NOT_FOUND;
    }

    cleanup:
    if (string != NULL) {
        VirtualFree(string, 0, MEM_FREE);
    }
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
    }

    return result;
}

HRESULT INCALESCENT_File_FilteredNamesSorted(PWSTR directory, PBYTE nameAllocation, PSIZE_T nameAllocationSize, PSIZE_T fileCount) {
    HRESULT result;
    WCHAR buffer[INCALESCENT_FILE_FILTER_AGGREGATE_SIZE];
    SIZE_T filesFound = 0;

    result = StringCchCopyW(buffer, INCALESCENT_FILE_FILTER_AGGREGATE_SIZE, directory);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = StringCchCatW(buffer, INCALESCENT_FILE_FILTER_AGGREGATE_SIZE, INCALESCENT_FILE_FILTER);
    if (FAILED(result)) {
        goto cleanup;
    }

    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(buffer, &data);
    if (find == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        // If a file couldn't be found, that's okay. Just inform the user.
        if (lastError == ERROR_FILE_NOT_FOUND) {
            result = S_OK;
        } else {
            result = HRESULT_FROM_WIN32(lastError);
        }
        goto cleanup;
    }

    PWSTR *allocationStringPointer = (PWSTR *) nameAllocation;
    PBYTE allocationStringStart = nameAllocation + (sizeof(PWSTR) * *fileCount);

    do {
        // Obtain the length of the file name string.
        SIZE_T nameLength = 0;
        result = StringCchLengthW(data.cFileName, MAX_PATH, &nameLength);
        if (FAILED(result)) {
            goto cleanup;
        }

        // Increment the number of files that have been found
        filesFound += 1;

        // Increase the size of the allocation required to store the file names.
        // Each string needs an associated pointer that resides in the beginning of
        // the memory allocation along with the actual string stored somewhere within
        // the allocation.
        *nameAllocationSize += sizeof(PWSTR) + (sizeof(WCHAR) * (nameLength + 1));

        // If there is no names buffer to write to, then there is no need to write to it.
        if (nameAllocation == NULL) {
            continue;
        }

        // Copy the file's name into the allocation at its appropriate location.
        PWSTR stringStart = (PWSTR) allocationStringStart;
        result = StringCchCopyW(stringStart, nameLength + 1, data.cFileName);
        if (FAILED(result)) {
            goto cleanup;
        }
        *allocationStringPointer = (PWSTR) allocationStringStart;

        allocationStringPointer++;
        allocationStringStart += sizeof(WCHAR) * (nameLength + 1);
    } while (FindNextFileW(find, &data) != FALSE);

    // Catch any errors that FindNextFileW may have
    DWORD lastError = GetLastError();
    if (lastError != 0) {
        if (lastError != ERROR_NO_MORE_FILES) {
            result = HRESULT_FROM_WIN32(GetLastError());
            goto cleanup;
        }
    }

    // This function run is simply to determine the allocation size and file count.
    // There is no writing of file names to a buffer occurring yet.
    if (nameAllocation == NULL) {
        goto cleanup;
    }

    if (filesFound != *fileCount) {
        result = INCALESCENT_ERROR_DATA_FILE_COUNT_MISMATCH;
        goto cleanup;
    }

    // Sort all the file names alphanumerically.
    result = INCALESCENT_String_BubbleSort(nameAllocation, *fileCount);

    cleanup:

    *fileCount = filesFound;
    return result;
}

HRESULT INCALESCENT_File_ReadAndWrite(PWSTR dataDirectory, PWSTR consolidatedFile) {
    HRESULT result = S_OK;
    HANDLE file = NULL;
    PBYTE names = NULL;
    HANDLE heap = GetProcessHeap();

    file = CreateFileW(consolidatedFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    SIZE_T allocationSize = 0;
    SIZE_T fileCount = 0;
    result = INCALESCENT_File_FilteredNamesSorted(dataDirectory, NULL, &allocationSize, &fileCount);\
    if (FAILED(result)) {
        goto cleanup;
    }
    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"Found %d valid data files...", fileCount);
    if (FAILED(result)) {
        goto cleanup;
    }
    if (fileCount == 0) {
        result = INCALESCENT_ERROR_NO_DATA_FILES_FOUND;
        goto cleanup;
    }

    names = HeapAlloc(heap, HEAP_ZERO_MEMORY, allocationSize);
    if (names == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }
    result = INCALESCENT_File_FilteredNamesSorted(dataDirectory, names, &allocationSize, &fileCount);
    if (FAILED(result)) {
        goto cleanup;
    }

    WCHAR buffer[INCALESCENT_TABLE_ROW_LENGTH];
    result = StringCchCopyW(buffer, INCALESCENT_TABLE_ROW_LENGTH, INCALESCENT_TABLE_HEADER_STRING);
    if (FAILED(result)) {
        goto cleanup;
    }

    DWORD writeCount = 0;
    BOOL writeResult = WriteFile(file, buffer, sizeof(WCHAR) * (INCALESCENT_TABLE_HEADER_STRING_LENGTH), &writeCount, NULL);
    if (!writeResult) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    WCHAR filePathBuffer[INCALESCENT_FILE_FILTER_AGGREGATE_SIZE];

    PWSTR *mappedString = (PWSTR *) names;
    for (SIZE_T index = 0; index < fileCount; index++) {
        // Create a new string which contains the file's full path
        PWSTR fileName = *(mappedString + index);
        result = StringCchPrintfW(filePathBuffer, INCALESCENT_FILE_FILTER_AGGREGATE_SIZE, L"%s\\%s", dataDirectory, fileName);
        if (FAILED(result)) {
            goto cleanup;
        }
        result = INCALESCENT_LOG_INFO_FORMATTED_W(L"Reading temperature for %s ...", fileName);
        if (FAILED(result)) {
            goto cleanup;
        }

        // Attempt to retrieve the data value from the file
        WCHAR value[INCALESCENT_FILE_TEMPERATURE_FIELD_VALUE_MAX_LENGTH];
        result = INCALESCENT_File_ReadTemperature(filePathBuffer, value);
        if (FAILED(result)) {
            goto cleanup;
        }
        result = INCALESCENT_LOG_INFO_FORMATTED_W(L"... found, value discovered to be %s ...", value);
        if (FAILED(result)) {
            goto cleanup;
        }

        result = StringCchPrintfW(buffer, INCALESCENT_TABLE_ROW_LENGTH, L"%d,%s,%s\r\n", index, fileName, value);
        if (FAILED(result)) {
            goto cleanup;
        }

        SIZE_T charactersWritten;
        result = StringCchLengthW(buffer, INCALESCENT_TABLE_ROW_LENGTH, &charactersWritten);
        if (FAILED(result)) {
            goto cleanup;
        }

        writeResult = WriteFile(file, buffer, sizeof(WCHAR) * charactersWritten, &writeCount, NULL);
        if (!writeResult) {
            result = HRESULT_FROM_WIN32(GetLastError());
            goto cleanup;
        }
    }

    cleanup:
    if (names != NULL) {
        HeapFree(heap, 0, names);
    }
    if (file != NULL) {
        CloseHandle(file);
    }
    return result;
}
