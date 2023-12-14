queneau-list: queneau-list.cpp
	g++ -o queneau-list -Ofast -fopenmp queneau-list.cpp
	
queneau-count: queneau-count.cpp
	g++ -o queneau-count -Ofast -fopenmp queneau-count.cpp
