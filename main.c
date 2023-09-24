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
#include "main.h"
#include "log.h"
#include "file.h"
#include "dialog.h"
#include "generated_error.h"

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, PSTR commandLineArguments, INT showCommand) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLineArguments);
    UNREFERENCED_PARAMETER(instance);

    HRESULT result = S_OK;

    // Print the license to the console
    result = INCALESCENT_LOG_RAW_W(INCALESCENT_LICENSE);
    if (FAILED(result)) {
        goto cleanup;
    }

    presentFileChoices:
    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"Presenting file prompt for input directory...");
    if (FAILED(result)) {
        goto cleanup;
    }

    WCHAR sourcePath[INCALESCENT_DIALOG_FILE_MAX_PATH];
    result = INCALESCENT_Dialog_PresentChooseSource(sourcePath);
    if (FAILED(result)) {
        // If the user cancelled the file prompt, then it's not an error
        // we're concerned about.
        if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            result = S_OK;
        }
        goto cleanup;
    }

    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"... chose input directory \"%s\".", sourcePath);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"Presenting file prompt for output file...");
    if (FAILED(result)) {
        goto cleanup;
    }

    WCHAR destinationPath[INCALESCENT_DIALOG_FILE_MAX_PATH];
    result = INCALESCENT_Dialog_PresentChooseDestination(destinationPath);
    if (FAILED(result)) {
        // If the user cancelled the file prompt, then it's not an error
        // we're concerned about.
        if (result == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
            result = S_OK;
        }
        goto cleanup;
    }

    // Log the chosen output (destination) file path.
    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"... chose output file \"%s\".", destinationPath);
    if (FAILED(result)) {
        goto cleanup;
    }

    // Read the files and write the data to the file.
    DWORD startTime = GetTickCount();
    result = INCALESCENT_File_ReadAndWrite(sourcePath, destinationPath);
    if (FAILED(result)) {
        if (result != INCALESCENT_ERROR_NO_DATA_FILES_FOUND) {
            goto cleanup;
        }
        INT modalResult = MessageBoxW(
                NULL,
                L"Could not find any data files in the folder selected. Retry different folder?",
                L"No Data Files Found",
                MB_RETRYCANCEL | MB_ICONERROR | MB_APPLMODAL
        );
        if (modalResult == IDCANCEL) {
            result = S_OK;
            goto cleanup;
        }
        goto presentFileChoices;
    }
    DWORD endTime = GetTickCount();
    DWORD deltaTime = endTime - startTime;
    DOUBLE converted = ((DOUBLE) deltaTime) / 1000.0;

    // Log the time it took to perform the operation.
    result = INCALESCENT_LOG_INFO_FORMATTED_W(L"Finished in %.2f seconds. Program will exit in 2 seconds...",
                                              converted);
    if (FAILED(result)) {
        goto cleanup;
    }

    Sleep(2000);

    cleanup:

    // Attempt to print information about an error if it was encountered.
    if (FAILED(result)) {
        // Print the error. If this fails somehow, the program will make
        // an attempt to exit with the failure result code for the log
        // function.
        result = INCALESCENT_LOG_FAILED_RESULT_W(result);
    }

    return result;
}
