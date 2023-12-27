queneau: queneau.c
	gcc -o queneau -O3 -fopenmp -Wno-unused-result queneau.c
	
queneau-count: queneau-count.c
	gcc -o queneau-count -O3 -fopenmp -Wno-unused-result queneau-count.c

prime-sieve: prime-sieve.c
	gcc -o prime-sieve -O2 prime-sieve.c
