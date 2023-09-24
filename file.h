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
#ifndef INCALESCENT_FILE_H
#define INCALESCENT_FILE_H
#include "string.h"

// Forward declarations from <windows.h>
typedef long HRESULT;
typedef unsigned short WCHAR;
typedef WCHAR* PWSTR;
typedef unsigned char* PBYTE;
typedef unsigned __int64* PSIZE_T;

#define INCALESCENT_FILE_MAX_PATH 260
#define INCALESCENT_FILE_FILTER L"\\*.tif.metadata"
#define INCALESCENT_FILE_FILTER_AGGREGATE_SIZE ((INCALESCENT_FILE_MAX_PATH * 2) + 2)

#define INCALESCENT_FILE_TEMPERATURE_FIELD_VALUE_MAX_LENGTH 16
#define INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING L"userComment4="
#define INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING_LENGTH INCALESCENT_STRING_LENGTH(INCALESCENT_FILE_TEMPERATURE_FIELD_KEY_STRING)

#define INCALESCENT_FILE_TEMPERATURE_RAW_BUFFER_SIZE 1024
#define INCALESCENT_FILE_TEMPERATURE_CHARACTER_BUFFER_MAX_LENGTH 2048

// 2 commas, 2 new-line characters, 1 for null-terminating character
#define INCALESCENT_TABLE_CONTROL_CHARACTER_COUNT 5
#define INCALESCENT_TABLE_HEADER_STRING L"Index,File,Temperature\r\n"
#define INCALESCENT_TABLE_HEADER_STRING_LENGTH INCALESCENT_STRING_LENGTH(INCALESCENT_TABLE_HEADER_STRING)
#define INCALESCENT_TABLE_ROW_LENGTH (INCALESCENT_TABLE_CONTROL_CHARACTER_COUNT + INCALESCENT_FILE_TEMPERATURE_FIELD_VALUE_MAX_LENGTH + INCALESCENT_FILE_FILTER_AGGREGATE_SIZE)

HRESULT INCALESCENT_File_ReadTemperature(PWSTR path, WCHAR value[INCALESCENT_FILE_TEMPERATURE_FIELD_VALUE_MAX_LENGTH]);
HRESULT INCALESCENT_File_FilteredNamesSorted(PWSTR directory, PBYTE nameAllocation, PSIZE_T nameAllocationSize, PSIZE_T fileCount);
HRESULT INCALESCENT_File_ReadAndWrite(PWSTR dataDirectory, PWSTR consolidatedFile);

#endif //INCALESCENT_FILE_H
