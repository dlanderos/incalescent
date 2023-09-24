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
#include <shlwapi.h>
#include <shobjidl.h>
#include "dialog.h"

typedef struct INCALESCENT_FileDialogEvents {
    IFileDialogEventsVtbl* table;
    LONG referenceCount;
} INCALESCENT_FileDialogEvents;

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_QueryInterface(IFileDialogEvents *this, const IID *identifier, LPVOID *object) {
    static const QITAB table[] = {
            QITABENT(INCALESCENT_FileDialogEvents, IFileDialogEvents),
            QITABENT(INCALESCENT_FileDialogEvents, IFileDialogControlEvents),
            {0},
    };
    return QISearch(this, table, identifier, object);
}

ULONG STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_AddRef(IFileDialogEvents *this) {
    INCALESCENT_FileDialogEvents *cast = (INCALESCENT_FileDialogEvents *) this;
    return InterlockedIncrement(&cast->referenceCount);
}

ULONG STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_Release(IFileDialogEvents* this) {
    INCALESCENT_FileDialogEvents *cast = (INCALESCENT_FileDialogEvents *) this;
    LONG referenceCount = InterlockedDecrement(&cast->referenceCount);
    if (referenceCount == 0) {
        HANDLE heap = GetProcessHeap();
        HeapFree(heap, 0, this);
    }
    return referenceCount;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnFileOk(IFileDialogEvents* this, IFileDialog *dialog) {
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnFolderChanging(IFileDialogEvents* this, IFileDialog *dialog, IShellItem *folder) {
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    UNREFERENCED_PARAMETER(folder);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnFolderChange(IFileDialogEvents* this, IFileDialog *dialog) {
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnSelectionChange(IFileDialogEvents* this, IFileDialog *dialog) {
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnShareViolation(IFileDialogEvents* this, IFileDialog *dialog, IShellItem *item, FDE_SHAREVIOLATION_RESPONSE *response) { // NOLINT(*-non-const-parameter)
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(response);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnTypeChange(IFileDialogEvents* this, IFileDialog *dialog) {
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE INCALESCENT_FileDialogEvents_OnOverwrite(IFileDialogEvents* this, IFileDialog *dialog, IShellItem *item, FDE_OVERWRITE_RESPONSE *response) { // NOLINT(*-non-const-parameter)
    UNREFERENCED_PARAMETER(this);
    UNREFERENCED_PARAMETER(dialog);
    UNREFERENCED_PARAMETER(item);
    UNREFERENCED_PARAMETER(response);
    return S_OK;
}

static IFileDialogEventsVtbl INCALESCENT_FileDialogEvents_Table = {
        INCALESCENT_FileDialogEvents_QueryInterface,
        INCALESCENT_FileDialogEvents_AddRef,
        INCALESCENT_FileDialogEvents_Release,
        INCALESCENT_FileDialogEvents_OnFileOk,
        INCALESCENT_FileDialogEvents_OnFolderChanging,
        INCALESCENT_FileDialogEvents_OnFolderChange,
        INCALESCENT_FileDialogEvents_OnSelectionChange,
        INCALESCENT_FileDialogEvents_OnShareViolation,
        INCALESCENT_FileDialogEvents_OnTypeChange,
        INCALESCENT_FileDialogEvents_OnOverwrite,
};

HRESULT INCALESCENT_FileDialogEvents_CreateInstance(INCALESCENT_FileDialogEvents **object) {
    HRESULT result = S_OK;
    HANDLE heap = GetProcessHeap();

    INCALESCENT_FileDialogEvents *intermediate = HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(INCALESCENT_FileDialogEvents));
    if (intermediate == NULL) {
        result = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }
    intermediate->table = &INCALESCENT_FileDialogEvents_Table;
    intermediate->referenceCount = 1;

    result = intermediate->table->QueryInterface((IFileDialogEvents *) intermediate, &IID_IFileDialogEvents, (LPVOID *) object);
    if (FAILED(result)) {
        HeapFree(heap, 0, intermediate);
        goto cleanup;
    }

    intermediate->table->Release((IFileDialogEvents *) intermediate);

    cleanup:
    return result;
}

HRESULT INCALESCENT_Dialog_PresentChooseSource(WCHAR source[INCALESCENT_DIALOG_FILE_MAX_PATH]) {
    HRESULT result = S_OK;
    BOOL comInitialized = FALSE;
    IFileDialog *fileDialog = NULL;
    INCALESCENT_FileDialogEvents *fileDialogEvents = NULL;
    IShellItem *chosenResult = NULL;
    DWORD cookie = 0;
    DWORD flags = 0;
    PWSTR filePath = NULL;

    result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(result)) {
        goto cleanup;
    }
    comInitialized = TRUE;

    result = CoCreateInstance(
            &CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IFileDialog,
            (LPVOID *) &fileDialog);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = INCALESCENT_FileDialogEvents_CreateInstance(&fileDialogEvents);
    if (FAILED(result)) {
        goto cleanup;
    }
    result = fileDialog->lpVtbl->Advise(fileDialog, (IFileDialogEvents *) fileDialogEvents, &cookie);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->GetOptions(fileDialog, &flags);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetOptions(fileDialog, flags | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetTitle(fileDialog, INCALESCENT_DIALOG_SOURCE_TITLE);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->Show(fileDialog, NULL);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->GetResult(fileDialog, &chosenResult);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = chosenResult->lpVtbl->GetDisplayName(chosenResult, SIGDN_FILESYSPATH, &filePath);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = StringCchCopyW(source, INCALESCENT_DIALOG_FILE_MAX_PATH, filePath);

    cleanup:
    if (filePath != NULL) {
        CoTaskMemFree(filePath);
    }
    if (chosenResult != NULL) {
        chosenResult->lpVtbl->Release(chosenResult);
    }
    if (fileDialogEvents != NULL) {
        fileDialog->lpVtbl->Unadvise(fileDialog, cookie);
        fileDialogEvents->table->Release((IFileDialogEvents *) fileDialogEvents);
    }
    if (fileDialog != NULL) {
        fileDialog->lpVtbl->Release(fileDialog);
    }
    if (comInitialized) {
        CoUninitialize();
    }

    return result;
}

HRESULT INCALESCENT_Dialog_PresentChooseDestination(WCHAR destination[INCALESCENT_DIALOG_FILE_MAX_PATH]) {
    HRESULT result = S_OK;
    BOOL comInitialized = FALSE;
    IFileDialog *fileDialog = NULL;
    INCALESCENT_FileDialogEvents *fileDialogEvents = NULL;
    IShellItem *chosenResult = NULL;
    DWORD cookie = 0;
    DWORD flags = 0;
    PWSTR filePath = NULL;

    result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(result)) {
        goto cleanup;
    }
    comInitialized = TRUE;

    result = CoCreateInstance(
            &CLSID_FileSaveDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IFileDialog,
            (LPVOID *) &fileDialog);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = INCALESCENT_FileDialogEvents_CreateInstance(&fileDialogEvents);
    if (FAILED(result)) {
        goto cleanup;
    }
    result = fileDialog->lpVtbl->Advise(fileDialog, (IFileDialogEvents *) fileDialogEvents, &cookie);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->GetOptions(fileDialog, &flags);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetOptions(fileDialog, flags | FOS_OVERWRITEPROMPT | FOS_FORCEFILESYSTEM);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetTitle(fileDialog, INCALESCENT_DIALOG_DESTINATION_TITLE);
    if (FAILED(result)) {
        goto cleanup;
    }

    SYSTEMTIME time;
    GetLocalTime(&time);

    // Create a string buffer to store the formatted date and time as the file name
    WCHAR dateTimeString[32];
    result = StringCchPrintfW(dateTimeString, 32, L"data_%04d_%02d_%02d_%02d_%02d_%02d",
                              time.wYear, time.wMonth, time.wDay,
                              time.wHour, time.wMinute, time.wSecond);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetFileName(fileDialog, dateTimeString);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->SetDefaultExtension(fileDialog, L"csv");
    if (FAILED(result)) {
        goto cleanup;
    }

    COMDLG_FILTERSPEC filter = { 0};
    filter.pszName = L"Text Files";
    filter.pszSpec = L"*.csv;*.txt";

    result = fileDialog->lpVtbl->SetFileTypes(fileDialog, 1, &filter);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->Show(fileDialog, NULL);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = fileDialog->lpVtbl->GetResult(fileDialog, &chosenResult);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = chosenResult->lpVtbl->GetDisplayName(chosenResult, SIGDN_FILESYSPATH, &filePath);
    if (FAILED(result)) {
        goto cleanup;
    }

    result = StringCchCopyW(destination, INCALESCENT_DIALOG_FILE_MAX_PATH, filePath);

    cleanup:
    if (filePath != NULL) {
        CoTaskMemFree(filePath);
    }
    if (chosenResult != NULL) {
        chosenResult->lpVtbl->Release(chosenResult);
    }
    if (fileDialogEvents != NULL) {
        fileDialog->lpVtbl->Unadvise(fileDialog, cookie);
        fileDialogEvents->table->Release((IFileDialogEvents *) fileDialogEvents);
    }
    if (fileDialog != NULL) {
        fileDialog->lpVtbl->Release(fileDialog);
    }
    if (comInitialized) {
        CoUninitialize();
    }

    return result;
}
