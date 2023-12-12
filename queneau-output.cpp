//Queneau searching program by Dylan G.

#include <iostream>
#include <string.h>
#include <fstream>
#include <new>
#include <vector>
#include <cstdlib>
#include <omp.h>

//Loads the global int array "primes" with a list of prime numbers
#include "primes.cpp"

//Global variable specifying the range size handled by each thread
long chunk_size;

//Returns the positive offset of the first number after 'a' divisible by 'b' (returns 0 if b|a)
long offset(long a, int b)
{
    return (b - (a % b)) % b;
}

//Counts the number of primes below the square root of the maximum 2n+1 value in the chunk
int count_primes(long N_max)
{
    int i, p;
    for(i = 1; i < sizeof(primes); i++)
    {
        p = primes[i - 1];
        if(p * p > N_max)
            break;
    }
    return i;
}

//Calculates positive [a^n mod b] in O(log n) time
long powmod(long a, long n, long b)
{
    long res = 1;
    while (n > 0)
    {
        if (n & 1)
            res = res * a % b;
        a = a * a % b;
        n >>= 1;
    }
    //Force a positive output when a < 0
    return (res + b) % b;
}

//Performs a sieve of eratosthenes on the chunk. Returns an array of bools
//indicating which numbers in the chunk to skip because 2n+1 is prime or n%4=0
bool* prime_sieve(long chunk_start, int num_primes)
{
    int i, p;
    long j, Nmin, mstart;
    
    //Start with all "false" values ("true" means skip)
    bool* skip = new bool[chunk_size]();

    //Set true flag for all multiples of 4
    for(j = offset(chunk_start, 4); j < chunk_size; j += 4)
        skip[j] = true;

    //First 2n+1 value for calculating offsets
    Nmin = 2 * chunk_start + 1;
    
    //Loop through each prime number except 2 (since 2n+1 is always odd)
    for(i = 1; i < num_primes; i++)
    {
        p = primes[i];

        //Calculate offset of first N entry divisible by p.
        mstart = offset((Nmin - p) >> 1, p);

        //Skip p itself if p is in the chunk
        if(Nmin <= p)
            mstart += p;

        //Set true flag in every position divisible by p
        for(j = mstart; j < chunk_size; j += p)
            if(!skip[j])
                skip[j] = true;
    }

    return skip;
}


//Thread entry point. Finds all Queneau numbers from chunk_start to chunk_start + chunk_size - 1.
//Counts up and multiplies the prime factors of each candidate, disqualifying any candidates where 2 or -2
//is not a primitive root along the way, by checking if 2^(2n/p) mod 2n+1 is 1 for any prime factor p.
std::vector<long> queneau_sieve(long chunk_start)
{
    bool* skip;
    int i, p, num_primes;
    long j, divisor, first_pos, step, chunk_stop, root, order, sum;
    std::vector<long> qlist;
    
    //Contains the products of all prime factors (w/ multiplicity) below sqrt(2n) for each 'order' (2n).
    //After the main loop, it contains each 2n value divided by its largest prime factor.
    long* fprods = new long[chunk_size];
    
    //Start with 1 in each prime factor product
    for(j = 0; j < chunk_size; j++)
        fprods[j] = 1;
    
    //Sieve out numbers where n%4=0 or 2n+1 is composite
    num_primes = count_primes(2*(chunk_start + chunk_size) - 1);
    skip = prime_sieve(chunk_start, num_primes);

    //Final n value in chunk, plus one. Only needed to bound p^e when chunk_start is 0. We only
    //need to check up to final n rather than final 2n, because if (p^e|2n), then (p^e|n) when
    //p is odd. When p is 2 we only check up to p^e=8 since we throw out all n%4=0 values anyway.
    chunk_stop = chunk_start + chunk_size;
    
    //Primary Loop: Loop through all primes below the maximum sqrt(2n) value
    for(i = 0; i < num_primes; i++)
    {
        p = primes[i];
        
        //Secondary Loop: Loop through powers of each prime.
        //This only runs to completion when chunk_start is 0.
        for(divisor = p; divisor < chunk_stop; divisor *= p)
        {
            //Get the offset of the first value divisible by p^e.
            //Should be chunk_start * 2 if chunk_start is odd
            first_pos = offset(chunk_start, divisor);
            
            //Break if divisor is 8 (n%4=0), or if the first number divisble
            //by p^e is outside of the current chunk.
            if(divisor == 8 or first_pos >= chunk_size)
                break;
            
            //Jump through the array by divisor. If p is 2 we must jumpt through
            //by half of this step size, as all of our "orders" (2n) are even
            step = (p == 2) ? divisor >> 1 : divisor;
            
            //Tertiary Loop: Loop through each order (2n) in the chunk
            for(j = first_pos; j < chunk_size; j += step)
            {
                if(skip[j]) continue;
            
                //First iteration of the secondary loop only. This disqualifies any
                //candidate which doesn't have 2 or -2 as a primitive root of 2n+1
                if(divisor == p)
                {
                    order = (chunk_start + j) * 2;
                    
                    //Use -2 if n % 4 == 3, otherwise check 2
                    root = (chunk_start + j) % 4 == 3 ? -2 : 2;
                    
                    //Check if 2 (or -2) is NOT a primitive root of 2n+1
                    if (powmod(root, order / p, order + 1) == 1)
                    {
                        skip[j] = true;
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
    sum = 0;
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        
        //If fprod[j] equals 2n, then we have already checked all of its prime factors
        order = 2 * (chunk_start + j);
        if(fprods[j] == order)
        {
            sum++;
            continue;
        }
        
        //Check if 2 or -2 to the power of (2n) divided by its final factor (large prime) yields
        //1 mod (2n+1). This quotient is just fprod[j], which we know is even, so we just use 2.
        if (powmod(2, fprods[j], order + 1) == 1)
            skip[j] = true;
        else
            sum++;
    }
    
    //No longer need fprods; reallocate memory for qlist
    delete[] fprods;
    qlist.reserve(sum);
    
    //Fill qlist with each valid Queneau number
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        qlist.push_back(chunk_start + j);
    }
    
    //Garbage collect and return
    delete[] skip;
    return qlist;
}

//Main entry point. Reads in upper limit and chunk_size from argv, then splits
//the total range by chunk_size to be divided amongst parallel threads using OpenMP.
int main(int argc, char *argv[])
{
    long count = 0;
    long chunk_total;
    
    //Read in bounds and chunk size from command line (defaults to 0)
    long start = atol(argv[argc - 3]);
    long stop = atol(argv[argc - 2]);
    chunk_size = atol(argv[argc - 1]);
    int nchunks = (stop - start) / chunk_size;
    
    //Queneaus are stored in an array of separate vectors to prevent a race condition
    std::vector<long>* lists = new std::vector<long>[nchunks];
    
    //Create output file name
    std::string outfile_name = "output/";
    outfile_name.append(argv[argc - 3]);
    outfile_name.append("-to-");
    outfile_name.append(argv[argc - 2]);
    
    //Open output file for editing
    std::ofstream outfile(outfile_name);
    if(! outfile.is_open())
    {
        std::cout << "Could not create output file" << '\n';
        return 1;
    }
    
    //Use OpenMP to split chunks among threads
    #pragma omp parallel for
    for(int i = 0; i < nchunks; i++)
        lists[i] = queneau_sieve(start + i * chunk_size);
    
    //Count up the total number of Queneaus found
    for(int i = 0; i < nchunks; i++)
    {
        //Count up totals
        chunk_total = lists[i].size();
        count += chunk_total;
        
        //Write Queneau numbers to output file
        for(long j = 0; j < chunk_total - 1; j++)
            outfile << lists[i][j] << '\n';
        
        //Flush buffer periodically
        outfile << lists[i][chunk_total - 1] << std::endl;
    }
    outfile.close();
    delete[] lists;
    
    //Print total and exit
    std::cout << "Total Queneaus between " << start << " and " << stop << ": " << count << '\n';
    return 0;
}
