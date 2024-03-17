#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define PATH_TO_IMAGE "../img.bmp"
#define NUM_PROCESSES 10
#define FIFO_PATH "/tmp/fifo"
#define CHILD_PROCESS_EXECUTABLE "./ChildProcess"

struct ColorCount {
    long red;
    long green;
    long blue;
};

void CreateChildProcess(ColorCount& totalColorCount, int column, const char* fifoPath) {
    int fd;
    ColorCount colorCount;

    if (mkfifo(fifoPath, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if ((fd = open(fifoPath, O_WRONLY)) == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // Execute the ChildProcess executable
        execlp(CHILD_PROCESS_EXECUTABLE, CHILD_PROCESS_EXECUTABLE, std::to_string(column).c_str(), nullptr);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        if ((fd = open(fifoPath, O_RDONLY)) == -1) {
            perror("open");
            exit(EXIT_FAILURE);
        }

        ssize_t bytesRead = read(fd, &colorCount, sizeof(colorCount));

        if (bytesRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        close(fd);

        totalColorCount.red += colorCount.red;
        totalColorCount.green += colorCount.green;
        totalColorCount.blue += colorCount.blue;

        int status;
        waitpid(pid, &status, 0);

        unlink(fifoPath);
    }
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
        char fifoPath[256];
        snprintf(fifoPath, sizeof(fifoPath), FIFO_PATH, i);

        CreateChildProcess(totalColorCount, width - i - 1, fifoPath);
    }

    std::cout << "Red: " << totalColorCount.red << std::endl;
    std::cout << "Green: " << totalColorCount.green << std::endl;
    std::cout << "Blue: " << totalColorCount.blue << std::endl;

    stbi_image_free(image_data);

    return 0;
}
