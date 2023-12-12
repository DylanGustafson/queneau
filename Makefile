queneau-3step: queneau-3step.cpp
	g++ -o queneau-3step -Ofast -fopenmp queneau-3step.cpp
	
queneau-combo: queneau-combo.cpp
	g++ -o queneau-combo -Ofast -fopenmp queneau-combo.cpp
	
queneau: queneau-output.cpp
	g++ -o queneau -Ofast -fopenmp queneau-output.cpp
