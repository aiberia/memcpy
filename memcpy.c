#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void *alloc(size_t length) {
    const size_t alignment = 2 * 1024 * 1024;
    uint8_t *data;

    if ((data = aligned_alloc(alignment, length)) == NULL) {
        perror("aligned_alloc");
        return NULL;
    }

    if (madvise(data, length, MADV_HUGEPAGE) != 0)
    {
        perror("madvise");
        return NULL;
    }

    for (size_t i = 0; i < length; i += 4096)
        data[i] = i;

    return data;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("syntax error\n");
        return 1;
    }

    size_t length_mbytes = (size_t) atoi(argv[1]);
    size_t length_bytes = length_mbytes * 1024 * 1024;

    printf("allocating memory...\n");

    uint8_t *data0;
    uint8_t *data1;

    if ((data0 = alloc(length_bytes)) == NULL)
        return 1;

    if ((data1 = alloc(length_bytes)) == NULL)
        return 1;

    printf("running...\n");

    struct timespec start, stop;

    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start) != 0) {
        perror("clock_gettime");
        return 1;
    }

    const int count = 1000;

    for (int i = 0; i < count; i++)
        memcpy(data0, data1, length_bytes);


    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop) != 0) {
        perror("clock_gettime");
        return 1;
    }

    uint64_t elapsed_nsec = (stop.tv_sec * 1000000000ll + stop.tv_nsec) - (start.tv_sec * 1000000000ll + start.tv_nsec);
    double elapsed_sec = elapsed_nsec / 1000000000.0;
    uint64_t transferrred_mbytes = length_mbytes * count;

    printf("test complete in: %0.03fs\n", elapsed_sec);
    printf("transfer chunk: %" PRIu64 " B\n", (uint64_t) length_bytes);
    printf("transferrred: %" PRIu64 " MB\n", (uint64_t) transferrred_mbytes);
    printf("copy rate: %0.03f MB/s\n", transferrred_mbytes / elapsed_sec);
    printf("bus usage: %0.03f MB/s\n", transferrred_mbytes * 2 / elapsed_sec);


    return 0;
}