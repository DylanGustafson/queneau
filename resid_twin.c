#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define MAX_MOD 19

//Main entry point
int main(const int argc, const char** argv)
{
    long block_start, block_size, i;
    short* diffs;
    short diff;

    char mod;
    char resid[MAX_MOD - 1];
    long resid_count[MAX_MOD - 1][MAX_MOD] = {0};
    long twin_count = 0;

    //Open input file containing differences stored in binary as shorts
    FILE* i_file = fopen(argv[argc - 1], "rb");
    if(i_file == NULL)
    {
        printf("Could not find \"%s\"\n", argv[argc - 1]);
        return 1;
    }

    //Get file size for diffs list, minus start value (long) at beginning and final difference (short) at end
    fseek(i_file, 0, SEEK_END);
    block_size = (ftell(i_file) - sizeof(long) - sizeof(short)) / sizeof(short);
    diffs = (short*) malloc(block_size * sizeof(short));

    //Start at beginning and read in starting value
    fseek(i_file, 0, SEEK_SET);
    fread(&block_start, sizeof(long), 1, i_file);

    //Read in short diffs
    fread(&diffs[0], sizeof(short), block_size, i_file);
    fclose(i_file);

    //Get modulo residuals of block starting value
    for(mod = 0; mod < MAX_MOD - 1; mod++)
        resid[mod] = (char)(block_start % (mod + 2));

    for(long i = 0; i < block_size; i++)
    {
        diff = diffs[i];

        if(diff == 1)
            twin_count++;

        for(mod = 0; mod < MAX_MOD - 1; mod++)
        {
            resid[mod] = (resid[mod] + diff) % (mod + 2);
            resid_count[mod][resid[mod]]++;
        }
    }
    free(diffs);

    printf("%li, %li,\n", block_start, twin_count);
    for(mod = 0; mod < MAX_MOD - 1; mod++)
    {
        for(size_t j = 0; j < mod + 2; j++)
            printf("%7li,", resid_count[mod][j]);
        printf("\n");
    }
    return 0;
}
