#include <stdio.h>
#include <string.h>

void xorOperation(char *temp, char *poly) {
    for (int i = 1; i < strlen(poly); i++)
        temp[i] = ((temp[i] == poly[i]) ? '0' : '1');
}

int main() {
    char data[100], poly[20];

    printf("Enter data bits: ");
    scanf("%s", data);
    printf("Enter generator polynomial bits: ");
    scanf("%s", poly);

    int dataLen = strlen(data);
    int polyLen = strlen(poly);

    // Append zeros = degree of polynomial
    char temp[150];
    strcpy(temp, data);
    for (int i = 0; i < polyLen - 1; i++)
        temp[dataLen + i] = '0';
    temp[dataLen + polyLen - 1] = '\0';

    char remainder[150];
    strcpy(remainder, temp);

    for (int i = 0; i <= dataLen - 1; i++) {
        if (remainder[i] == '1')
            xorOperation(&remainder[i], poly);
    }

    printf("CRC remainder: ");
    for (int i = dataLen; i < dataLen + polyLen - 1; i++)
        printf("%c", remainder[i]);

    printf("\n");
    return 0;
}
