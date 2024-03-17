#include <iostream>
#include <windows.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

#define PATH_TO_IMAGE "../img.bmp"
#define NUM_PROCESSES 10

struct ColorCount {
    long red;
    long green;
    long blue;
};

void ProcessColumn(const unsigned char *image_data, int column, int width, int height, int channels, ColorCount &result) {
    result = {0, 0, 0};

    for (size_t i = 0; i < height; i++) {
        for (int j = column; j >= 0; j -= NUM_PROCESSES) {
            auto index = (i * width + j) * channels;

            auto red = image_data[index];
            auto green = image_data[++index];
            auto blue = image_data[++index];

            if (red >= green && red >= blue) {
                result.red++;
            } else if (green >= red && green >= blue) {
                result.green++;
            } else {
                result.blue++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ChildProcess.exe <column>" << std::endl;
        return -1;
    }

    int column = std::stoi(argv[1]);
    int width;
    int height;
    int channels;

    unsigned char *image_data = stbi_load(PATH_TO_IMAGE, &width, &height, &channels, 0);

    if (!image_data) {
        std::cerr << "Error loading image data." << std::endl;
        return -1;
    }

    ColorCount result = {0, 0, 0};
    ProcessColumn(image_data, column, width, height, channels, result);

    HANDLE pipe = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytesWritten;
    WriteFile(pipe, &result, sizeof(ColorCount), &bytesWritten, nullptr);

    CloseHandle(pipe);

    // Имя файла для записи
    std::string num = std::to_string(column);
    std::string fileName = "example" + num + ".txt";

    // Создание объекта ofstream (output file stream) для записи данных в файл
    std::ofstream outFile(fileName);

    if (outFile.is_open()) {
        // Запись данных в файл
        outFile << result.red << std::endl;
        outFile << result.green << std::endl;
        outFile << result.blue << std::endl;

        // Закрытие файла
        outFile.close();
    } else {
        std::cerr << "Не удалось открыть файл для записи." << std::endl;
    }

    return 0;
}
