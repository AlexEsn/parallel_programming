#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define PATH_TO_IMAGE "../img.bmp"
#define NUM_THREADS 10

using namespace std;

struct ColorCount {
    atomic_ulong red = 0;
    atomic_ulong green = 0;
    atomic_ulong blue = 0;
};

struct ThreadData {
    unsigned long column;
    ColorCount *colorCount;
    unsigned char *image_data;
    size_t width;
    size_t height;
    size_t channels;
};

void ThreadFunction(ThreadData* params) {
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
                colorCount->red.fetch_add(1);
            } else if (green >= red && green >= blue) {
                colorCount->green.fetch_add(1);
            } else {
                colorCount->blue.fetch_add(1);
            }
        }
    }
}

int main() {
    int width, height, channels;
    unsigned char *image_data = stbi_load(PATH_TO_IMAGE, &width, &height, &channels, 0);

    if (!image_data) {
        cerr << "The picture should be on the path: " << PATH_TO_IMAGE << endl;
        return -1;
    }

    if (channels != 3) {
        cerr << "The image does not have three channels (RGB)" << endl;
        return -1;
    }

    vector<thread> threads;
    vector<ThreadData> threadParams(NUM_THREADS);
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

        threads.emplace_back(ThreadFunction, &threadParams[i]);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    cout << "Red: " << colorCount.red << endl;
    cout << "Green: " << colorCount.green << endl;
    cout << "Blue: " << colorCount.blue << endl;

    stbi_image_free(image_data);

    return 0;
}