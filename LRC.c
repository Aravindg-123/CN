#include <stdio.h>
#include <string.h>

int main() {
    char data[50];
    printf("Enter binary string (multiple of 8 bits): ");
    scanf("%s", data);

    int len = strlen(data);
    int columns = 8;
    int rows = len / columns;
    int lrc[8] = {0};

    // Compute LRC
    for (int c = 0; c < columns; c++) {
        int count = 0;
        for (int r = 0; r < rows; r++) {
            if (data[r * 8 + c] == '1')
                count++;
        }
        lrc[c] = count % 2;
    }

    printf("LRC: ");
    for (int i = 0; i < columns; i++)
        printf("%d", lrc[i]);

    printf("\n");
    return 0;
}
