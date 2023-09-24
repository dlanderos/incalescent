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
#ifndef INCALESCENT_DIALOG_H
#define INCALESCENT_DIALOG_H

// Forward declarations from <windows.h>
typedef long HRESULT;
typedef unsigned short WCHAR;

#define INCALESCENT_DIALOG_FILE_MAX_PATH 260
#define INCALESCENT_DIALOG_SOURCE_TITLE L"Select Data Folder:"
#define INCALESCENT_DIALOG_DESTINATION_TITLE L"Select where to save consolidated data:"

HRESULT INCALESCENT_Dialog_PresentChooseSource(WCHAR source[INCALESCENT_DIALOG_FILE_MAX_PATH]);
HRESULT INCALESCENT_Dialog_PresentChooseDestination(WCHAR destination[INCALESCENT_DIALOG_FILE_MAX_PATH]);

#endif //INCALESCENT_DIALOG_H
