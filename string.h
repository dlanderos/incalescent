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
#ifndef INCALESCENT_STRING_H
#define INCALESCENT_STRING_H

#define INCALESCENT_STRING_LENGTH(string) ((sizeof(string) / sizeof((string)[0])) - 1)

// Forward declarations from <windows.h>
typedef long HRESULT;
typedef unsigned char* PBYTE;
typedef unsigned __int64 SIZE_T;

/**
 * @brief Sorts a string buffer alphanumerically.
 *
 * This function will sort a specialized string buffer alphanumerically such that a smaller
 * number will take precedence over a larger one. This is to ensure that data file names are
 * written to the resultant table in the order that they were created.
 *
 * @param[in] buffer    The string buffer. The first count * sizeof(PWSTR) bytes contain pointers
 *                      to each string within the buffer. This allows for quick manipulation of
 *                      which string each pointer points to during the sort operation. All strings
 *                      must be NULL-terminated so the function knows when they end and the next
 *                      one begins.
 * @param[in] count     The number of strings in the buffer.
 *
 * @return The result of the sort operation (S_OK if successful).
 */
HRESULT INCALESCENT_String_BubbleSort(PBYTE buffer, SIZE_T count);

#endif //INCALESCENT_STRING_H
