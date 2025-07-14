#include <windows.h>
#include <taskschd.h>
#include <comdef.h>
#include <string>
#include <iostream>

#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "ole32.lib")

bool GetTaskBootStatus(const std::wstring& taskName) {
    bool success = false;
    HRESULT hr = S_OK;
    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;
    IRegisteredTask* pTask = nullptr;

    do {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) break;

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL);
        if (FAILED(hr)) break;

        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
            IID_ITaskService, (void**)&pService);
        if (FAILED(hr) || !pService) break;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) break;

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) break;

        hr = pRootFolder->GetTask(_bstr_t(taskName.c_str()), &pTask);
        if (SUCCEEDED(hr) && pTask) {
            success = true;
        }
    } while (false);

    if (pTask) pTask->Release();
    if (pRootFolder) pRootFolder->Release();
    if (pService) pService->Release();
    CoUninitialize();
    return success;
}
bool AddTaskBoot(const std::wstring& taskName, const std::wstring& exeFile) {
    bool success = false;
    HRESULT hr = S_OK;

    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;
    ITaskDefinition* pTask = nullptr;
    IPrincipal* pPrincipal = nullptr;
    ITriggerCollection* pTriggerCollection = nullptr;
    ITrigger* pTrigger = nullptr;
    IActionCollection* pActionCollection = nullptr;
    IAction* pAction = nullptr;
    IExecAction* pExecAction = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;

    do {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if (FAILED(hr)) break;

        hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, EOAC_NONE, NULL);
        if (FAILED(hr)) break;

        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
            IID_ITaskService, (void**)&pService);
        if (FAILED(hr) || !pService) break;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) break;

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) break;

        // 删除旧任务（忽略错误）
        pRootFolder->DeleteTask(_bstr_t(taskName.c_str()), 0);

        hr = pService->NewTask(0, &pTask);
        if (FAILED(hr)) break;

        hr = pTask->get_Principal(&pPrincipal);
        if (FAILED(hr)) break;
        pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->put_LogonType(TASK_LOGON_SERVICE_ACCOUNT);
        pPrincipal->put_UserId(_bstr_t(L"SYSTEM"));

        hr = pTask->get_Triggers(&pTriggerCollection);
        if (FAILED(hr)) break;

        hr = pTriggerCollection->Create(TASK_TRIGGER_BOOT, &pTrigger);
        if (FAILED(hr)) break;

        hr = pTask->get_Actions(&pActionCollection);
        if (FAILED(hr)) break;

        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        if (FAILED(hr)) break;

        hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
        if (FAILED(hr)) break;

        hr = pExecAction->put_Path(_bstr_t(exeFile.c_str()));
        if (FAILED(hr)) break;

        hr = pRootFolder->RegisterTaskDefinition(
            _bstr_t(taskName.c_str()),
            pTask,
            TASK_CREATE_OR_UPDATE,
            _variant_t(L"SYSTEM"),
            _variant_t(),
            TASK_LOGON_SERVICE_ACCOUNT,
            _variant_t(L""),
            &pRegisteredTask);
        if (FAILED(hr)) break;

        success = true;
    } while (false);

    if (pRegisteredTask) pRegisteredTask->Release();
    if (pExecAction) pExecAction->Release();
    if (pAction) pAction->Release();
    if (pActionCollection) pActionCollection->Release();
    if (pTrigger) pTrigger->Release();
    if (pTriggerCollection) pTriggerCollection->Release();
    if (pPrincipal) pPrincipal->Release();
    if (pTask) pTask->Release();
    if (pRootFolder) pRootFolder->Release();
    if (pService) pService->Release();
    CoUninitialize();
    return success;
}

#include "WinTool.h"
int wmain() {

    WinTool::SetAutoBoot(L"C:/Program Files/Ezboo/ezboo.exe", true);

    WinTool::GetAutoBootStatus(L"C:/Program Files/Ezboo/ezboo.exe");

    WinTool::SetAutoBoot(L"C:/Program Files/Ezboo/ezboo.exe", false);

    const std::wstring taskName = L"MyElevatedStartupTask";
    const std::wstring exePath = L"C:\\Program Files\\Ezboo\\ezboo.exe";

    if (GetTaskBootStatus(taskName)) {
        std::wcout << L"任务已存在：" << taskName << std::endl;
    }
    else {
        if (AddTaskBoot(taskName, exePath)) {
            std::wcout << L"已创建系统启动自启任务：" << taskName << std::endl;
        }
        else {
            std::wcerr << L"创建任务失败！" << std::endl;
        }
    }

    return 0;
}
