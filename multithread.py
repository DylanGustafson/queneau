import time
import multiprocess as mp
#import os

#Generate indices of Queneau candidates by sieving out known failures
def sieve(chunk_min):
    #Creatie list of empty dictionaries
    factors = [{} for i in range(chunk_size)]

    #Create initial list of ones (trues) [1 = valid candidate]
    candidates = [True] * chunk_size

    #Remove all candidates where n % 4 == 0
    candidates[-chunk_min % 4 :: 4] = [0] * (chunk_size // 4)

    Nmin = int(2 * chunk_min + 1)

    max_divisor = int((2*(chunk_min + chunk_size) + 1) ** 0.5)
    nprimes = next(i[0] for i in enumerate(primes) if i[1] > max_divisor)

    #Sieve of Eratosthenes, but it only looks at odd (2n+1) numbers
    for k in range(nprimes):
        #Sieve out candidates starting with small primes
        p = primes[k]

        #Not needed when p=2, as all N values are odd
        if p > 2:
            #Calculate offset of first N entry divisible by p.
            offset = ((p - Nmin) >> 1) % p

            #Skip p itself if p is in the list
            if Nmin <= p:
                offset += p

            #Remove all multiples of p from candidates list
            candidates[offset :: p] = [0] * -((offset - chunk_size) // p)

    for k in range(nprimes):
        #Count factors where needed starting with large primes
        q = primes[k]#nprimes - k - 1]

        for i in range(1, chunk_size):    #Will never run to completion
            divisor = q ** i
            if divisor > chunk_size or divisor == 8:
                break

            #Since all of the numbers being factored are even
            step = (divisor >> 1) if q == 2 else divisor

            for j in range(-chunk_min % divisor, chunk_size, step):
                if not candidates[j]:
                    continue
                if i == 1:
                    factors[j][q] = 1
                else:
                    factors[j][q] += 1

    return (candidates, factors)

def chk_prim_root(a, order, factors):
    N = order + 1
    return all(pow(a, order // prime, N) != 1 for prime in factors)

def is_Queneau(n, factor_dict):
    order = 2 * n
    factors = list(factor_dict.keys())
    #factors.reverse()

    #Calculate last factor if needed
    fprod = 1
    for p in factor_dict:
        fprod *= p ** factor_dict[p]
    if fprod != order:
        factors += [order // fprod]

    if chk_prim_root(2, order, factors):
        return True
    elif n % 4 < 3:
        return False
    else:
        return chk_prim_root(-2, order, factors)

def task(chunk_min):
    (qlist, flist) = sieve(chunk_min)
    for j in range(chunk_size):
        if not qlist[j]:
            continue
        qlist[j] = is_Queneau(chunk_min + j, flist[j])

    return (sum(qlist), qlist) #, os.getpid())

if __name__ == '__main__':
    start = 0
    stop  = 2*10 ** 6
    chunk_size = 5*10**4

    #print(f'Number of CPUs: {mp.cpu_count()}')
    with open('primes.txt', 'r') as f:
        primes = [int(i) for i in f.read().split(' ')]

    if 2*(start + chunk_size) + 1 > primes[-1] ** 2:
        print("Not enough primes in list!")
        exit()

    #tic = time.perf_counter()
    #sieve(0)
    #toc = time.perf_counter()
    #print(f'Total time: {toc-tic} seconds')
    #exit()

    tic = time.perf_counter()
    with mp.Pool() as pool:
        data = pool.map(task, range(start, stop, chunk_size))

    a = []
    for i in range(chunk_size):
        if data[0][1][i]:
            a.append(start + i)
        if len(a) >= 7:
            break
    b = []
    for i in range(chunk_size):
        if data[-1][1][chunk_size - i - 1]:
            b.append(stop - i - 1)
        if len(b) >= 7:
            break
    b.reverse()
    toc = time.perf_counter()

    print(f'{a} .... {b}')
    print(f'Queneaus from {start:.2E} to {stop:.2E}: {sum(i[0] for i in data)}')
    print(f'Total time: {toc-tic} seconds')
