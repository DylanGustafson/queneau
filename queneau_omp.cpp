#include <iostream>
#include <new>
#include <map>
#include <vector>
#include <cstdlib>
#include <omp.h>
#include "primes.cpp"

long chunk_size;

long offset(long a, int b)
{
    return (-a % b + b) % b;
}

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

bool* prime_sieve(long chunk_start, int num_primes)
{
    int i, p;
    long j, Nmin, mstart;
    bool* skip = new bool[chunk_size]();

    for(j = offset(chunk_start, 4); j < chunk_size; j += 4)
        skip[j] = true;

    Nmin = 2 * chunk_start + 1;
    for(i = 1; i < num_primes; i++)
    {
        p = primes[i];

        //Calculate offset of first N entry divisible by p.
        mstart = offset((Nmin - p) >> 1, p);

        //Skip p itself if p is in the list
        if(Nmin <= p)
            mstart += p;

        for(j = mstart; j < chunk_size; j += p)
            if(!skip[j])
                skip[j] = true;
    }

    return skip;
}

std::map<int, char>* factor_sieve(bool* skip, long chunk_start, int num_primes)
{
    int i, p;
    long j, divisor, first_pos, step, chunk_stop;
    std::map<int, char>* factors = new std::map<int, char>[chunk_size];

    for(i = 0; i < num_primes; i++)
    {
        p = primes[i];
        chunk_stop = chunk_start + 2 * chunk_size;
        for(divisor = p; divisor < chunk_stop; divisor *= p)
        {
            first_pos = offset(chunk_start, divisor);
            if(first_pos >= chunk_size)
                break;
            
            step = divisor;
            if(p == 2)
                step >>= 1;
            
            for(j = first_pos; j < chunk_size; j += step)
                if(!skip[j])
                    factors[j][p] += 1;
        }
    }
    return factors;
}

long powmod(long a, long n, long mod)
{
    long res = 1;
    while (n > 0)
    {
        if (n & 1)
            res = res * a % mod;
        a = a * a % mod;
        n >>= 1;
    }
    return res;
}

bool is_queneau(long n, std::map<int, char> pfactors)
{
    long fprod;
    long order = n * 2;
    long mod = order + 1;
    long a = (n % 4 == 3) ? -2 : 2;
    std::map<int, char>::iterator itr;

    for (itr = pfactors.begin(); itr != pfactors.end(); ++itr)
        if ((powmod(a, order / itr->first, mod) + mod) % mod == 1)
            return false;

    fprod = 1;
    for (itr = pfactors.begin(); itr != pfactors.end(); ++itr)
        for(char e = 0; e < itr->second; e++)
            fprod *= itr->first;

    if(fprod == order)
        return true;

    return (powmod(a, fprod, mod) + mod) % mod != 1;
}

std::vector<long> task(long chunk_start)
{
    bool* skip;
    std::map<int, char>* factors;
    std::vector<long> qlist;

    long j;
    int i, num_primes;
    long count = 0;
    bool is_q;

    num_primes = count_primes(2*(chunk_start + chunk_size) - 1);
    skip = prime_sieve(chunk_start, num_primes);
    factors = factor_sieve(skip, chunk_start, num_primes);

    for(j = 0; j < chunk_size; j++)
    {
        if(skip[j]) continue;
        if( is_queneau(chunk_start + j, factors[j]) )
            qlist.push_back(chunk_start + j);
    }
    delete[] factors;
    delete[] skip;
    
    return qlist;
}

int main(int argc, char *argv[])
{
    long sum = 0;
    long start = 0;
    long stop = atol(argv[argc - 2]);
    chunk_size = atol(argv[argc - 1]);
    int nchunks = (stop - start) / chunk_size;
    std::vector<long>* lists = new std::vector<long>[nchunks];

    #pragma omp parallel for
    for(int i = 0; i < nchunks; i ++)
    {
        lists[i] = task(start + i * chunk_size);
    }
    
    for(int i = 0; i < nchunks; i++)
        sum += lists[i].size();
    std::cout << sum << '\n';
        
    delete[] lists;
}
