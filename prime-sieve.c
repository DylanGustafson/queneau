#include <stdio.h>
#include <stdlib.h>

//Loads the global int array "primes" with a list of prime numbers
#include "primes.h"

int main(int argc, char* argv[])
{
    int i, j, n, p, sum;
    int max = atoi(argv[argc - 1]);
    
    if(max < 2)
    {
        printf("Please specify an upper limit greater than 2\n");
        return 1;
    }
    
    char* skip = (char*) calloc(max, sizeof(char));
    
    FILE* outfile = fopen("primes.dat", "wb");
    if(outfile == NULL)
    {
        printf("Could not create output file primes.dat\n");
        return 1;
    }
    
    skip[0] = 1;
    skip[1] = 1;
    
    n = sizeof(primes)/sizeof(int);
    for(i = 0; i < n; i++)
    {
        p = primes[i];
        if(p * p > max)
            break;
        
        for(j = 2 * p; j < max; j += p)
            skip[j] = 1;
    }
    
    sum = 0;
    for(j = 0; j < max; j++)
        if(!skip[j])
            sum++;
    
    fwrite(&sum, sizeof(int), 1, outfile);
    for(j = 0; j < max; j++)
        if(!skip[j])
            fwrite(&j, sizeof(int), 1, outfile);
        
    free(skip);
    fclose(outfile);
    return 0;
}
