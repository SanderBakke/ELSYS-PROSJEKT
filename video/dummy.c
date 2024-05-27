#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include <unistd.h>

int main() {
    srand(time(NULL)); // Seed the random number generator
    int result = rand() % 180; // Generate a random number between 0 and 99
    printf("%d\n", result); // Print the result to stdout
    fflush(stdout);   
    return 0;
}
