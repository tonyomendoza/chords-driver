/*
 * ================================================================
 *  File: chords_driver.c
 *  Author: Tony0M
 *  Date: May 11, 2025
 *  Description: A test for chords.ko device driver
 *
 *  License: GPLv2
 *  Copyright (c) 2025 by TonyOM. All rights reserved.
 * ================================================================
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() 
{
    int fd;
    char *s = "Em7S2S4A11/B"; // E Minor 7th Sus2 Sus4 Add11 / B...

    // Open
    fd = open("/dev/chords210", O_RDWR);
    if (fd < 0) {
        perror("open");
        return fd;
    }

    // Test write
    int length = write(fd, s, strlen(s));
    if (length < 0) {
        perror("write");
    }

    // Test read
    char result[32] = {0};
    read(fd, result, sizeof(result));
    printf("Result from driver: %s\n", result);

    // Close
    close(fd);
    return 0;
}