//Queneau searching program by Dylan G.
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

//IO file names
const char* primes_file = "primes.dat";
const char* output_file = "output.txt";

//Global vars that remain constant across each thread
long chunk_size;
int* primes;
int total_primes;

//Returns the positive offset of the first number after 'a' divisible by 'b' (returns 0 if b|a)
long offset(const long a, const long b)
{
    return (b - a % b) % b;
}

//Calculates [2^n mod b] in O(log n) time
long pow2mod(long n, const long b)
{
    __int128 res = 1;
    __int128 a = 2;
    while (n > 0)
    {
        if (n & 1)
            res = res * a % b;
        a = a * a % b;
        n >>= 1;
    }
    return (long)res;
}

//Performs a sieve of eratosthenes on the chunk. Returns the number of primes needed to do so.
int prime_sieve(const long chunk_start, char* skip)
{ 
    int i, p;
    long j, N_min, N_max, mstart;

    //First 2n+1 value for calculating offsets
    N_min = 2 * chunk_start + 1;
    N_max = N_min + 2 * (chunk_size - 1);
    
    //Loop through each prime number except 2 (since 2n+1 is always odd)
    for(i = 1; i < total_primes; i++)
    {
        p = primes[i];
        if((long)p * p > N_max)
            break;

        //Calculate offset of first N entry divisible by p.
        mstart = offset((N_min - p) >> 1, p);

        //Skip p itself if p is in the chunk
        if(N_min <= p)
            mstart += p;

        //Set true flag in every position divisible by p
        for(j = mstart; j < chunk_size; j += p)
            if(!skip[j])
                skip[j] = 1;
    }
    
    if(i == total_primes)
    {
        printf("Error: Not enough primes in %s\n", primes_file);
        exit(1);
    }
    
    return i;
}

//Uses a prime factor sieve to disqualify any value where 2 (or -2) is not a
//primitive root of 2n+1. Returns the number of valid indices remaining in "skip"
long root_sieve(const long chunk_start, const int num_primes, char* skip)
{
    int i, p;
    long j, first_pos, step, chunk_stop, order, pm, count;
    __int128 divisor;
    
    //Contains the products of all prime factors (w/ multiplicity) below sqrt(2n) for each 'order' (2n).
    //After the main loop, it contains each 2n value divided by its largest prime factor.
    //long* fprods = new long[chunk_size];
    long* fprods = (long*) malloc(chunk_size * sizeof(long));
    
    //Start with 1 in each prime factor product
    for(j = 0; j < chunk_size; j++)
        fprods[j] = 1;
    
    //Final n value in chunk, plus one. Used to bound divisor (p^e) in secondary loop. We only
    //need to check up to final n rather than final 2n, because if (p^e|2n), then (p^e|n) when
    //p is odd. When p is 2 we only check up to p^e=8 since we throw out all n%4=0 values anyway.
    chunk_stop = chunk_start + chunk_size;
    
    //Primary Loop: Loop through all primes below the maximum sqrt(2n) value
    for(i = 0; i < num_primes; i++)
    {
        p = primes[i];
        
        //Secondary Loop: Loop through powers of each prime.
        for(divisor = p; divisor < chunk_stop; divisor *= p)
        {
            //Get the offset of the first value divisible by p^e.
            //Should be chunk_start * 2 if chunk_start is odd
            first_pos = offset(chunk_start, divisor);
        
            //Break if divisor is 8 (n%4 = 0), or if the first number divisble
            //by p^e is outside of the current chunk.
            if(divisor == 8 || first_pos >= chunk_size)
                break;
            
            //Use divisor as a step size for jumping through the chunk. If p is 2
            //we must halve this step size, as all of our "orders" (2n) are even
            step = (i == 0) ? divisor >> 1 : divisor;
            
            for(j = first_pos; j < chunk_size; j += step)
            {
                if(skip[j]) continue;
            
                //First iteration of the secondary loop only. This disqualifies any
                //candidate which doesn't have 2 or -2 as a primitive root of 2n+1
                if(divisor == p)
                {
                    //Evaluate 2^(2n/p) % (2n+1)
                    order = (chunk_start + j) * 2;
                    pm = pow2mod(order / p, order + 1);
                    
                    //When n%4 = 3 the root to check should be -2 instead of 2, which only
                    //changes the math if the exponent 2n/p is odd (only when p = 2).
                    if(i == 0 && (chunk_start + j) % 4 == 3)
                        //-2 is not a primitive root if 2^(2n/p) % (2n+1) = 2n
                        pm -= order;
                    else
                        //2 is not a primitive root if 2^(2n/p) % (2n+1) = 1
                        pm--;
                    
                    //If primitive root check failed, skip this index from now on
                    if(pm == 0)
                    {
                        skip[j] = 1;
                        continue;
                    }
                }
                
                //Multiply the product of prime factors by p each time
                fprods[j] *= p;
            }
        }
    }
    
    //Loop through the chunk one last time to see if all of the prime factors have been checked. 
    //If not, check the last factor (large prime) to see if the primitive root check passes.
    count = 0;
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        
        //If fprod[j] equals 2n, then we have already checked all of its prime factors
        order = 2 * (chunk_start + j);
        if(fprods[j] == order)
        {
            count++;
            continue;
        }
        
        //Check if 2^(2n/p) % (2n+1) = 1, where p is the largest prime factor of 2n. 2n/p is
        //just fprod[j], which we know is even, therefore we don't need to consider -2 here.
        if (pow2mod(fprods[j], order + 1) == 1)
            skip[j] = 1;
        else
            count++;
    }
    
    //Delete fprods and return queneau count
    free(fprods);
    return count;
}

//Thread entry point. Sets up array of chars (bools) indicating which n values in the chunk are
//disqualified, using prime_sieve and root_sieve. Returns a long array of valid queneau numbers.
long* queneau_sieve(const long chunk_start, long* chunk_count)
{
    int num_primes;
    long i, j;
    long* qlist;
    
    //Array of indices to skip (skip[j] = 0 means chunk_start + j is a queneau)
    char* skip = (char*) malloc(chunk_size);
    
    //Sieve out all multiples of 4 by setting each to 1, and the rest to 0
    for(j = 0; j < chunk_size; j += 4)
    {
        skip[j] = 1;
        skip[j + 1] = 0;
        skip[j + 2] = 0;
        skip[j + 3] = 0;
    }
    
    //Sieve out numbers where 2n+1 is composite, and update num_primes
    num_primes = prime_sieve(chunk_start, skip);
    
    //Sieve out numbers where 2 (or -2) is not a primitive root mod 2n+1, and update chunk_count
    *chunk_count = root_sieve(chunk_start, num_primes, skip);
    
    qlist = (long*) malloc(*chunk_count * sizeof(long));
    
    //Fill qlist with each valid queneau number
    i = 0;
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        qlist[i] = chunk_start + j;
        i++;
    }
    
    //Garbage collect and return
    free(skip);
    return qlist;
}

//Main entry point. Reads in upper limit and chunk_size from argv, then splits
//the total range by chunk_size to be divided amongst parallel threads using OpenMP.
int main(int argc, char* argv[])
{
    long j, total_count;
    
    //Read in bounds and chunk size from command line (defaults to 0)
    long start = atol(argv[argc - 3]);
    long block_size = atol(argv[argc - 2]);
    chunk_size = atol(argv[argc - 1]);
    int nchunks = block_size / chunk_size;
    
    //Queneaus are stored in an array of separate vectors to prevent a race condition
    long** lists = (long**) malloc(nchunks * sizeof(long*));
    long* counts = (long*) malloc(nchunks * sizeof(long));
    
    //Open primes.dat to get primes needed for sieve
    FILE* p_file = fopen(primes_file, "rb");
    if(p_file == NULL)
    {
        printf("Could not find \"%s\"\n", primes_file);
        return 1;
    }
    
    //First 4 bytes is total number of primes
    fread(&total_primes, sizeof(int), 1, p_file);
    primes = (int*) malloc(total_primes * sizeof(int));
    
    //Read in list of primes from the rest of primes.dat
    fread(primes, sizeof(int), total_primes, p_file);
    fclose(p_file);
    
    //Open the output file BEFORE doing calculations in case there's an issue
    FILE* o_file = fopen(output_file, "w");
    if(o_file == NULL)
    {
        printf("Could not create \"%s\"\n", output_file);
        return 1;
    }
    
    //Use OpenMP to split chunks among threads
    #pragma omp parallel for
    for(int i = 0; i < nchunks; i++)
        lists[i] = queneau_sieve(start + i * chunk_size, &counts[i]);
    
    //Dont need list of primes anymore
    free(primes);
    
    //Count up the total number of Queneaus found
    total_count = 0;
    for(int i = 0; i < nchunks; i++)
    {
        //Sum up the total count
        total_count += counts[i];
        
        //Write Queneau numbers to output file
        for(j = 0; j < counts[i]; j++)
            fprintf(o_file, "%li\n", lists[i][j]);
        
        //Delete each chunk qlist after writing
        free(lists[i]);
    }
    fclose(o_file);
    
    //Delete pointers to chunk lists and their list sizes
    free(lists);
    free(counts);
    
    printf("%li\n", total_count);
    return 0;
}
