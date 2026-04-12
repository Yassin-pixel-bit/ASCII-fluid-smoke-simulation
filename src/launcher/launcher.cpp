#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include <string>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
    int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
    #include <unistd.h>
    #include <limits.h>
    int main(int argc, char* argv[])
#endif
{
    std::string exe_dir = "";
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    exe_dir = std::filesystem::path(path).parent_path().string();
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        exe_dir = std::filesystem::path(path).parent_path().string();
    } else {
        exe_dir = "."; // Absolute worst-case fallback
    }
#endif

    for (char& c : exe_dir) {
        if (c == '\\') c = '/';
    }

    // --- 2. Boot Python ---
    Py_Initialize();

    std::string boot_script = 
        "import sys\n"
        "import os\n"
        "import runpy\n"
        "\n"
        "base_dir = '" + exe_dir + "'\n"
        "launcher_dir = os.path.join(base_dir, 'launcher_py')\n"
        "script_path = os.path.join(launcher_dir, 'main.py')\n"
        "\n"
        "sys.path.insert(0, launcher_dir)\n"
        "\n"
        "try:\n"
        "    runpy.run_path(script_path, run_name='__main__')\n"
        "except Exception as e:\n"
        "    try:\n"
        "        import tkinter.messagebox\n"
        "        tkinter.messagebox.showerror('Launcher Error', f'Fatal Error:\\n{e}')\n"
        "    except ImportError:\n"
        "        print(f'\\n[LAUNCHER CRASH] {e}')\n"
        "        print('[DEPENDENCY MISSING] Tkinter not found. Run: sudo apt install python3-tk\\n')\n";

    PyRun_SimpleString(boot_script.c_str());

    if (Py_FinalizeEx() < 0) {
        return 120;
    }
    return 0;
}