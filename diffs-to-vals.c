#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    long length, val, j;
    short* list;

    //Open the output file beforehand in case there's an issue
    FILE* o_file = fopen("output.txt", "w");
    if(o_file == NULL)
    {
        printf("Could not create \"%s\"\n", "output.txt");
        return 1;
    }

    //Open input file containing differences stored in binary as shorts
    FILE* in_file = fopen(argv[argc - 1], "rb");
    if(in_file == NULL)
    {
        printf("Could not find \"%s\"\n", argv[argc - 1]);
        return 1;
    }

    //Get file size for list, minus start value (long) at beginning
    fseek(in_file, 0, SEEK_END);
    length = ftell(in_file) - sizeof(long);
    list = (short*) malloc(length);

    //Start at beginning and read in starting value
    fseek(in_file, 0, SEEK_SET);
    fread(&val, sizeof(long), 1, in_file);
    fprintf(o_file, "Block start: %li\n", val);

    //Read in short diffs
    fread(&list[0], sizeof(short), length, in_file);
    fclose(in_file);

    //Write values to file by adding diffs
    for(j = 0; j < length / sizeof(short) - 1; j++)
    {
        val += list[j];
        fprintf(o_file, "%li\n", val);
    }

    fprintf(o_file, "Block end: %li\n", val + list[j]);
    fclose(o_file);
    free(list);
    return 0;
}
