#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() 
{
    int fd;
    char *s = "A";

    fd = open("/dev/chords", O_RDWR);

    if (fd < 0) {
        perror("open");
        return fd;
    }

    // Send the actual string data instead of the address of s
    int length = write(fd, s, strlen(s));

    if (length < 0) {
        perror("write");
    }

    // Read back any data (if applicable)
    char result[16] = {0};
    read(fd, result, sizeof(result));
    printf("Result from driver: %s\n", result);

    close(fd);
    return 0;
}