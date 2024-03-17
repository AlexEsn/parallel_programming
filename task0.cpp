#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

#define PATH_TO_IMAGE "../img.bmp"

using namespace std;

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

    unsigned long count_red = 0, count_green = 0, count_blue = 0;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            auto index = (i * width +  j) * channels;

            auto red = image_data[index];
            auto green = image_data[++index];
            auto blue = image_data[++index];

            if (red >= green && red >= blue) {
                count_red++;
            } else if (green >= red && green >= blue) {
                count_green++;
            } else {
                count_blue++;
            }
        }
    }

    cout << "Red: " << count_red << endl;
    cout << "Green: " << count_green << endl;
    cout << "Blue: " << count_blue << endl;

    stbi_image_free(image_data);

    return 0;
}
