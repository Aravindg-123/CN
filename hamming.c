#include <stdio.h>
#include <math.h>

// Function to calculate number of parity bits
int calcParityBits(int m) {
    int r = 0;
    while ((1 << r) < (m + r + 1))
        r++;
    return r;
}

// Sender – generate Hamming code
void generateHamming() {
    int data[50], hamming[50];
    int m, r, totalBits;

    printf("\n--- SENDER SIDE ---\n");
    printf("Enter number of data bits: ");
    scanf("%d", &m);

    printf("Enter data bits (LSB first):\n");
    for (int i = m, j = 1; i >= 1; i--, j++)
        scanf("%d", &data[j]);

    r = calcParityBits(m);
    totalBits = m + r;

    printf("Number of parity bits required = %d\n", r);

    int j = 1;
    for (int i = 1; i <= totalBits; i++) {
        if ((i & (i - 1)) == 0)
            hamming[i] = 0;  // parity bit
        else
            hamming[i] = data[j++];
    }

    // Calculate parity bits
    for (int i = 0; i < r; i++) {
        int p = 1 << i;
        int sum = 0;

        for (int k = p; k <= totalBits; k += 2 * p)
            for (int x = k; x < k + p && x <= totalBits; x++)
                sum ^= hamming[x];

        hamming[p] = sum;
    }

    printf("\nGenerated Hamming Code: ");
    for (int i = totalBits; i >= 1; i--)
        printf("%d", hamming[i]);
    printf("\n");
}

// Receiver – detect and correct error
void receiveHamming() {
    int received[50], totalBits;

    printf("\n--- RECEIVER SIDE ---\n");
    printf("Enter number of received bits: ");
    scanf("%d", &totalBits);

    printf("Enter received bits (LSB first):\n");
    for (int i = totalBits; i >= 1; i--)
        scanf("%d", &received[i]);

    int r = calcParityBits(totalBits - calcParityBits(totalBits));
    int errorPos = 0;

    for (int i = 0; i < r; i++) {
        int p = 1 << i;
        int sum = 0;

        for (int k = p; k <= totalBits; k += 2 * p)
            for (int x = k; x < k + p && x <= totalBits; x++)
                sum ^= received[x];

        if (sum)
            errorPos += p;
    }

    if (errorPos == 0)
        printf("No error in received data.\n");
    else {
        printf("Error detected at bit position: %d\n", errorPos);
        received[errorPos] ^= 1;  // flip the bit
        printf("Corrected data: ");
        for (int i = totalBits; i >= 1; i--)
            printf("%d", received[i]);
        printf("\n");
    }
}

int main() {
    int choice;

    while (1) {
        printf("\n--- Hamming Code Menu ---\n");
        printf("1. Sender (Generate Hamming Code)\n");
        printf("2. Receiver (Detect and Correct Error)\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1)
            generateHamming();
        else if (choice == 2)
            receiveHamming();
        else
            break;
    }

    return 0;
}
