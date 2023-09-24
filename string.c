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
#include "string.h"


// Implementation of INCALESCENT_String_BubbleSort
HRESULT INCALESCENT_String_BubbleSort(PBYTE buffer, SIZE_T count) {
    HRESULT result = S_OK;

    PWSTR *mappedHeader = (PWSTR *) buffer;

    for (SIZE_T indexOuter = 0; indexOuter < (count - 1); indexOuter++) {
        BOOL swapPerformed = FALSE;

        for (SIZE_T indexInner = 0; indexInner < (count - indexOuter - 1); indexInner++) {
            PWSTR firstString = *(mappedHeader + indexInner);
            PWSTR secondString = *(mappedHeader + indexInner + 1);

            // Obtain the comparison result between the first and the second string.
            // The length fields for both strings are set to "-1" so that the comparison function
            // will use their terminating characters. It is expected that the strings were validated by
            // the callee of this function.
            INT comparison = CompareStringW(
                    LOCALE_USER_DEFAULT,
                    LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS,
                    firstString,
                    -1,
                    secondString,
                    -1
            );

            // If the comparison is less than zero, the strings should be swapped so that the one with greater
            // alphanumeric significance comes first.
            if (comparison == CSTR_GREATER_THAN) {
                *(mappedHeader + indexInner) = secondString;
                *(mappedHeader + indexInner + 1) = firstString;
                swapPerformed = TRUE;
                continue;
            }

            // If the comparison result is equal to zero, that means there was an error, and it
            // should consequently be returned.
            if (comparison == 0) {
                result = HRESULT_FROM_WIN32(GetLastError());
                goto cleanup;
            }
        }

        // If a swap wasn't performed, then the strings have already been sorted.
        if (!swapPerformed) {
            break;
        }
    }

    cleanup:

    return result;
}
