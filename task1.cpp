#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"
#include <windows.h>

#define PATH_TO_IMAGE "../img.bmp"
#define NUM_THREADS 10

using namespace std;

struct ColorCount {
    unsigned long red = 0;
    unsigned long green = 0;
    unsigned long blue = 0;
};

struct ThreadData {
    unsigned long column;
    ColorCount *colorCount;
    unsigned char *image_data;
    size_t width;
    size_t height;
    size_t channels;
};

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    auto params = static_cast<ThreadData *>(lpParam);
    int height = params->height;
    int width = params->width;
    int channels = params->channels;
    int column = params->column;
    unsigned char *image_data = params->image_data;
    ColorCount *colorCount = params->colorCount;

    for (size_t i = 0; i < height; i++) {
        for (int j = column; j >= 0; j -= NUM_THREADS) {
            auto index = (i * width + j) * channels;

            auto red = image_data[index];
            auto green = image_data[++index];
            auto blue = image_data[++index];

            if (red >= green && red >= blue) {
                InterlockedIncrement(&colorCount->red);
            } else if (green >= red && green >= blue) {
                InterlockedIncrement(&colorCount->green);
            } else {
                InterlockedIncrement(&colorCount->blue);
            }
        }
    }

    return 0;
}


int main() {
    int width, height, channels;
    unsigned char *image_data = stbi_load(PATH_TO_IMAGE, &width, &height, &channels, 0);

    if (!image_data) {
        cerr << "The picture should be on the path : " << PATH_TO_IMAGE << endl;
        return -1;
    }

    if (channels != 3) {
        cerr << "The image does not have three channels (RGB)" << endl;
        return -1;
    }

    HANDLE threads[NUM_THREADS];
    DWORD threadIds[NUM_THREADS];
    ThreadData threadParams[NUM_THREADS];
    ColorCount colorCount = {0, 0, 0};

    for (int i = 0; i < NUM_THREADS; i++) {
        threadParams[i] = {
                static_cast<unsigned long>(width - i - 1),
                &colorCount,
                image_data,
                static_cast<size_t>(width),
                static_cast<size_t>(height),
                static_cast<size_t>(channels)
        };

        threads[i] = CreateThread(
                NULL,
                0,
                ThreadFunction,
                &threadParams[i],
                0,
                &threadIds[i]
        );
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    cout << "Red: " << colorCount.red << endl;
    cout << "Green: " << colorCount.green << endl;
    cout << "Blue: " << colorCount.blue << endl;

    for (auto &thread: threads) {
        CloseHandle(thread);
    }

    stbi_image_free(image_data);

    return 0;
}