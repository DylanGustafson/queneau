queneau-listcpp: queneau-list.cpp
	g++ -o queneau-listcpp -O3 -fopenmp queneau-list.cpp

queneau-list: queneau-list.c
	gcc -o queneau-list -O3 -fopenmp -Wno-unused-result queneau-list.c

queneau-count: queneau-count.cpp
	g++ -o queneau-count -O3 -fopenmp queneau-count.cpp

prime-sieve: prime-sieve.c
	gcc -o prime-sieve -O2 prime-sieve.c
