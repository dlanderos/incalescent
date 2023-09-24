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
#ifndef INCALESCENT_LOG_H
#define INCALESCENT_LOG_H
#include "string.h"

// Forward declarations from <windows.h>
typedef long HRESULT;
typedef unsigned short* PWSTR;

#define INCALESCENT_LOG_FORMAT_W L"[%02d-%02d-%d %02d:%02d:%02d] [%s] %s\n"
#define INCALESCENT_LOG_MAX_HEADER_LENGTH 64
#define INCALESCENT_LOG_MAX_MESSAGE_LENGTH 512

HRESULT INCALESCENT_LogRawW(PWSTR message, SIZE_T messageSize);
HRESULT INCALESCENT_LogFormattedW(PWSTR levelString, PWSTR format, ...);
HRESULT INCALESCENT_LogFormattedErrorResultW(HRESULT failedResult);

#define INCALESCENT_LOG_RAW_W(message) INCALESCENT_LogRawW(message, INCALESCENT_STRING_LENGTH(message))
#define INCALESCENT_LOG_INFO_FORMATTED_W(format, ...) INCALESCENT_LogFormattedW(L"INFO", format, ##__VA_ARGS__)
#define INCALESCENT_LOG_FAILED_RESULT_W(result) INCALESCENT_LogFormattedErrorResultW(result)

#endif //INCALESCENT_LOG_H
