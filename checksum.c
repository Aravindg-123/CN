#include <stdio.h>

unsigned short checksum(unsigned short *buf, int size) {
    unsigned long sum = 0;

    while (size > 1) {
        sum += *buf++;
        if (sum & 0xFFFF0000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        size -= 2;
    }

    if (size)
        sum += *(unsigned char*)buf;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;
}

int main() {
    unsigned short data[] = {0x1234, 0xABCD, 0x0F0F};
    int size = sizeof(data);

    unsigned short result = checksum(data, size);
    printf("Checksum: 0x%X\n", result);

    return 0;
}
