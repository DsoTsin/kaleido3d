#include "metal_compiler.h"
#include <cstdio>
#include <string.h>

#if _WIN32
#include <Windows.h>
#pragma comment(lib, "advapi32.lib")
#endif

static char metal_root[4096] = {0};
static bool metal_found = false;

struct ProcessHandle {
  HANDLE process;
  HANDLE thread;
};

int metal_run_process(int is_ios, const char *program, const char *args,
                      mtl_process_t *process) {
  HKEY hkey = NULL;
  if (!metal_found) {
    auto status = ::RegOpenKeyA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\WOW6432Node\\Apple\\Metal Developer Tools", &hkey);
    if (hkey) {
      DWORD size = 0;
      ::RegQueryValueExA(hkey, NULL, NULL, NULL, NULL, &size);
      if (size > 0) {
        ::RegQueryValueExA(hkey, NULL, NULL, NULL, (LPBYTE)metal_root, &size);
        ::RegCloseKey(hkey);
        metal_found = true;
        goto RUN;
      } else {
        ::RegCloseKey(hkey);
        return -1;
      }
    } else {
      return -1;
    }
  } else {
  RUN:
    // metal.exe ...
    static thread_local char exe_path[4096] = {0};
    static thread_local char command_line[4096] = {0};
    if (is_ios == 1) {
      snprintf(exe_path, 4096, "%s\\ios\\bin\\%s.exe", metal_root, program);
      snprintf(command_line, 4096, "\"%s\\ios\\bin\\%s.exe\" %s", metal_root,
               program, args);
    } else {
      snprintf(exe_path, 4096, "%s\\macos\\bin\\%s.exe", metal_root, program);
      snprintf(command_line, 4096, "\"%s\\macos\\bin\\%s.exe\" %s",
               metal_root, program, args); 
    }
    STARTUPINFOA start_info = {};
    start_info.cb = sizeof(STARTUPINFO);
    start_info.dwFlags = STARTF_USESHOWWINDOW;
    start_info.wShowWindow = SW_HIDE;
    start_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    // start_info.dwFlags |= STARTF_USESTDHANDLES;
    PROCESS_INFORMATION process_info = {};
    if (TRUE == ::CreateProcessA(exe_path, command_line, NULL, NULL, FALSE, 0,
                                 NULL, NULL, &start_info, &process_info)) {
      auto handle = new ProcessHandle;
      handle->process = process_info.hProcess;
      handle->thread = process_info.hThread;
      if (process) {
        *process = handle;
      }
    }
    return 0;
  }
}

int metal_close_process(mtl_process_t process) {
  DWORD dw_exit_code = 0;
  if (process) {
    auto handle = (ProcessHandle *)process;
    ::WaitForSingleObject(handle->process, INFINITE);
    ::GetExitCodeProcess(handle->process, &dw_exit_code);
    CloseHandle(handle->process);
    CloseHandle(handle->thread);
    delete handle;
  }
  return dw_exit_code;
}
