queneau-list: queneau.c
	gcc -o queneau -O3 -fopenmp -Wno-unused-result queneau.c

prime-sieve: prime-sieve.c
	gcc -o prime-sieve -O2 prime-sieve.c
