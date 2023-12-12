//Queneau searching program by Dylan G.

#include <iostream>
#include <new>
#include <map>
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

//Finds the prime factors (below sqrt 2n) of each "order" (2n) in the chunk,
//and their multiplicity. Returns an array of map objects of various lengths.
//Each key (int) is a prime factor, and each value (char) is its multiplicity.
std::map<int, char>* factor_sieve(bool* skip, long chunk_start, int num_primes)
{
    int i, p;
    long j, divisor, first_pos, step, chunk_stop;
    std::map<int, char>* factors = new std::map<int, char>[chunk_size];

    //Final n value in chunk, plus one. Only needed to bound p^e when chunk_start is 0. We only
    //need to check up to final n rather than final 2n, because if (p^e|2n), then (p^e|n) when
    //p is odd. When p is 2 we only check up to p^e=8 since we throw out all n%4=0 values anyway.
    chunk_stop = chunk_start + chunk_size;
    
    //Loop through all primes below the maximum sqrt(2n) value
    for(i = 0; i < num_primes; i++)
    {
        p = primes[i];
        
        //Loop through powers of each prime.
        //This only runs to completion when chunk_start is 0.
        for(divisor = p; divisor < chunk_stop; divisor *= p)
        {
            //Get the offset of the first value divisible by p^e.
            first_pos = offset(chunk_start, divisor);
            
            //Break if divisor is 8 (n%4=0), or if the first number divisble
            //by p^e is outside of the current chunk.
            if(divisor == 8 or first_pos >= chunk_size)
                break;
            
            //Jump through the array by divisor. If p is 2 we must jumpt through
            //by half of this step size, as all of our "orders" (2n) are even
            step = divisor;
            if(p == 2)
                step >>= 1;
            
            //Increment the multiplicity of key 'p' of each map object divisible by p^e
            for(j = first_pos; j < chunk_size; j += step)
                if(!skip[j])
                    factors[j][p]++;
        }
    }
    return factors;
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

//Checks if a Queneau candidate is actually a valid Queneau number,
//by determining if either 2 or -2 are primitive roots of 2n+1
bool is_queneau(long n, std::map<int, char> pfactors)
{
    long fprod;
    long order = n * 2;
    long mod = order + 1;
    std::map<int, char>::iterator itr;
    
    //Only need to check one of these, specifically just -2 if n % 4 == 3
    long root = (n % 4 == 3) ? -2 : 2;

    //Loop through the prime factors of 2n to check if 2 (or -2) is a primitive root
    for (itr = pfactors.begin(); itr != pfactors.end(); ++itr)
        if (powmod(root, order / itr->first, mod) == 1)
            return false;

    //Multiply all of the prime factors below sqrt(2n) to check for one more large prime factor
    fprod = 1;
    for (itr = pfactors.begin(); itr != pfactors.end(); ++itr)
        for(char e = 0; e < itr->second; e++)
            fprod *= itr->first;

    //If the product of the factors is 2n (no more prime factors), then we have checked them all
    if(fprod == order)
        return true;

    //Otherwise need to check the last factor (large prime)
    return powmod(root, fprod, mod) != 1;
}

//Thread entry point. Finds Queneau numbers within the range specified by chunk_start
//and the global var chunk_size. Returns a vector of longs containing each Queneau number. 
std::vector<long> find_queneaus(long chunk_start)
{
    bool* skip;
    std::map<int, char>* factors;
    std::vector<long> qlist;

    long j;
    int num_primes;
    long count = 0;
    long sum = 0;

    //Create the prime sieve and factor sieve
    num_primes = count_primes(2*(chunk_start + chunk_size) - 1);
    skip = prime_sieve(chunk_start, num_primes);
    factors = factor_sieve(skip, chunk_start, num_primes);

    //Loop through each number and check primitive roots when valid
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        if(is_queneau(chunk_start + j, factors[j]))
            sum++;
        else
            skip[j] = 1;
    }
    
    //Don't need factors anymore; reallocate the memory for qlist
    delete[] factors;
    qlist.reserve(sum);
    
    //Fill qlist with each Queneau number
    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        qlist.push_back(chunk_start + j);
    }
    
    //Don't need these anymore
    delete[] skip;
    return qlist;
}

//Main entry point. Reads in upper limit and chunk_size from argv, then splits
//the total range by chunk_size to be divided amongst parallel threads using OpenMP.
int main(int argc, char *argv[])
{
    long sum = 0;
    
    //Read in bounds and chunk size from command line (defaults to 0)
    long start = atol(argv[argc - 3]);
    long stop = atol(argv[argc - 2]);
    chunk_size = atol(argv[argc - 1]);
    int nchunks = (stop - start) / chunk_size;
    
    //Queneaus are stored in an array of separate vectors to prevent a race condition
    std::vector<long>* lists = new std::vector<long>[nchunks];

    //Use OpenMP to split chunks among threads
    #pragma omp parallel for
    for(int i = 0; i < nchunks; i++)
        lists[i] = find_queneaus(start + i * chunk_size);
    
    //Count up the total number of Queneaus found
    for(int i = 0; i < nchunks; i++)
        sum += lists[i].size();
        //Here is where you would write them to a file
    
    //Print sum and exit
    std::cout << "Total Queneaus between " << start << " and " << stop << ": " << sum << '\n';
    delete[] lists;
    return 0;
}
