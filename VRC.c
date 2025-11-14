#include <stdio.h>
#include <string.h>

int main() {
    char data[50];
    printf("Enter a binary string: ");
    scanf("%s", data);

    int len = strlen(data);
    int count = 0;

    for (int i = 0; i < len; i++)
        if (data[i] == '1') count++;

    int parity = count % 2;  // even parity

    printf("Data with parity bit: %s%d\n", data, parity);

    return 0;
}
