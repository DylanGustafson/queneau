queneau-list: queneau-list.c
	gcc -o queneau-list -O3 -fopenmp -Wno-unused-result queneau-list.c

prime-sieve: prime-sieve.c
	gcc -o prime-sieve -O2 prime-sieve.c
