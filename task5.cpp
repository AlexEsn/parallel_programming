#include <iostream>
#include <windows.h>
#include <vector>
#include <tchar.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define PATH_TO_IMAGE "../img.bmp"
#define NUM_PROCESSES 10

struct ColorCount {
    long red;
    long green;
    long blue;
};

void CreateChildProcess(ColorCount& totalColorCount, int column) {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES saAttr;

    // Установка флага bInheritHandle, чтобы дескрипторы канала были унаследованы
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Создание канала
    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
        fprintf(stderr, "CreatePipe failed (%d)\n", GetLastError());
        return;
    }

    // Создание дочернего процесса
    STARTUPINFO siStartInfo;
    PROCESS_INFORMATION piProcInfo;

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hWritePipe;
    siStartInfo.hStdOutput = hWritePipe;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string num = std::to_string(column);
    std::string argument = "./ChildProcess.exe " + num;
    TCHAR commandLineArgs[256];
    _tcscpy_s(commandLineArgs, argument.c_str());

    if (!CreateProcess(
            nullptr,
            commandLineArgs,
            nullptr, // Process handle not inheritable
            nullptr, // Thread handle not inheritable
            TRUE, // Set handle inheritance to TRUE
            0, // No creation flags
            nullptr, // Use parent's environment block
            nullptr, // Use parent's starting directory
            &siStartInfo, // Pointer to STARTUPINFO structure
            &piProcInfo) // Pointer to PROCESS_INFORMATION structure
            ) {
        std::cerr << "Error creating child process." << std::endl;
        return;
    }

    // Закрытие записывающего конца канала, так как дочерний процесс будет в него записывать
    CloseHandle(hWritePipe);

    // Чтение данных из канала в родительском процессе
    ColorCount colorCount;
    DWORD bytesRead;

    while (ReadFile(hReadPipe, &colorCount, sizeof(colorCount), &bytesRead, NULL) && bytesRead != 0) {
        totalColorCount.red += colorCount.red;
        totalColorCount.green += colorCount.green;
        totalColorCount.blue += colorCount.blue;
    }

    // Закрытие читающего конца канала
    CloseHandle(hReadPipe);

    // Ожидание завершения дочернего процесса
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    // Закрытие дескрипторов процесса и потока.
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
}

int main() {
    int width, height, channels;
    unsigned char *image_data = stbi_load(PATH_TO_IMAGE, &width, &height, &channels, 0);

    if (!image_data) {
        std::cerr << "The picture should be on the path: " << PATH_TO_IMAGE << std::endl;
        return -1;
    }

    if (channels != 3) {
        std::cerr << "The image does not have three channels (RGB)" << std::endl;
        return -1;
    }

    ColorCount totalColorCount = {0, 0, 0};

    for (int i = 0; i < NUM_PROCESSES; i++) {
        CreateChildProcess(totalColorCount, width - i - 1);
    }

    std::cout << "Red: " << totalColorCount.red << std::endl;
    std::cout << "Green: " << totalColorCount.green << std::endl;
    std::cout << "Blue: " << totalColorCount.blue << std::endl;

    stbi_image_free(image_data);

    return 0;
}
