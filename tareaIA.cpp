#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <map>
#include <vector>
#include <tuple>
#include <string>
#include <new>
#include <stack>
#include <ctime>
#include <queue>
#include <algorithm>

using namespace std;
char nodoIn, nodoFin;				// variables globales

struct nodo{
	char nombre;
	int heuristica;
	int expandidos;
	vector<pair<nodo*, int>> vecinos;
};

struct comparaNodos{
	bool operator()(const nodo* n1, const nodo* n2) const{
		return n1->heuristica > n2->heuristica;
	}
};

nodo* crearNodo(char nombre, int valHeuristica){
	nodo* nodoTemp = new nodo();
	nodoTemp->nombre = nombre;
	nodoTemp->heuristica = valHeuristica;
	nodoTemp->expandidos = 0;
	return nodoTemp;
};

void imprimirMapa(map<char, nodo*> mapaNodos){
	for(int i=65; i<73; i++){						// 65=A 72=H en ascii
		// it puntero a tupla<char, nodo*> del mapa
		map<char, nodo*>::iterator it = mapaNodos.find((char)i);
		if(it != mapaNodos.end()){
			// it2 puntero a vector<tupla<nodo*, int>>
			vector<pair<nodo*, int>>::iterator it2 = it->second->vecinos.begin();
			vector<pair<nodo*, int>>::iterator it2End = it->second->vecinos.end();
			for(it2; it2 != it2End; it2++){
				cout << it->second->nombre << " -" << get<1>(*it2) << "-> ";
				cout << get<0>(*it2)->nombre << endl;
			}
		}else{
			cout << i << endl;
		}
	}
};

map<char, nodo*> leerArchivo(string entrada){
	ifstream grafoEntrada(entrada);
	if(!grafoEntrada){
		perror("No se pudo leer el archivo de entrada.\n");
		exit(0);
	}
	map<char, nodo*> mapaNodos;						// Usamos map para tener busquedas mas rapidas

	// leer archivo entrada
	string line;
	while(getline(grafoEntrada, line)){				// lee linea por linea
		if(line.substr(0,5) == "Init:"){			// Init
			nodoIn = line[6];
		}else if(line.substr(0,5) == "Goal:"){		// Goal
			nodoFin = line[6];
		}else{
			int posComa = line.find(",");
			if(posComa != string::npos){			// find retorna npos si no encuentra el string buscado (es decir, tenemos linea tipo: nodo-nodo-costo)
				// Encuentra el nombre de los nodos en el mapa
				map<char, nodo*>::iterator it1 = mapaNodos.find(line[0]);
				map<char, nodo*>::iterator it2 = mapaNodos.find(line[2]);
				nodo* nodo1 = it1->second;
				nodo* nodo2 = it2->second;

				int costoTemp = stoi(line.substr(4));
				pair<nodo*, int> tuplaTemp = make_pair(nodo2, costoTemp);
				nodo1->vecinos.emplace_back(tuplaTemp);					// insertamos la relacion al nodo
			}else{														// sin "," caso linea tipo: nodo-heuristica
				int heuristicaTemp;
				heuristicaTemp = stoi(line.substr(2));
				nodo* nodoTemp = crearNodo(line[0], heuristicaTemp);	// crea el nodo para insertar al mapa 			// no toy seguro si es con * xd
				mapaNodos[line[0]] = nodoTemp;							// inserta el nodo al mapa
			}
		}
	}
	grafoEntrada.close();
	// fin leer archivo entrada.
	return mapaNodos;
}

pair<vector<nodo*>, int> dfs(map<char,nodo*> mapaNodos){
	stack<nodo*> stackNodos;
	stackNodos.push(mapaNodos.find(nodoIn)->second);
	map<char, nodo*> visitados;
	int costo = 0;
	vector<nodo*> ruta;
	ruta.push_back(mapaNodos.find(nodoIn)->second);

	while(!stackNodos.empty()){
		nodo* nodoActual = stackNodos.top();
		stackNodos.pop();
		if(nodoActual->nombre == nodoFin){
			return make_pair(ruta, costo);
		}

		int num_vecinos = nodoActual->vecinos.size();
		while(num_vecinos>0){
			int indice = rand()%num_vecinos;
			nodo* nodoVecino = get<0>(nodoActual->vecinos[indice]);
			if(visitados.count(nodoVecino->nombre) == 0){
				costo += get<1>(nodoActual->vecinos[indice]);
				nodoActual->expandidos++;
				stackNodos.push(nodoVecino);
				visitados[nodoVecino->nombre] = nodoVecino;
				ruta.push_back(nodoVecino);
				break;
			}else{
				// teniendo que verificar si un vecino ya estaba expandido aleatoriamente es trabajo redundante
				// y esto lo hace más simple, siempre que no se necesite acceder a los vecinos denuevo
				if(num_vecinos == 1) costo -= get<1>(nodoActual->vecinos[0]);
				nodoActual->vecinos.erase(nodoActual->vecinos.begin()+indice);
				num_vecinos--;

			}
		}
	}

	cout << "No se encontró la ruta" << endl;
	return make_pair(ruta, -1);
}

pair<vector<nodo*>, int> bcu(map<char, nodo*> mapaNodos){
	// pq<tipo_dato, contenedor_tipo_dato, comparador>
	int costo = 0;
	priority_queue<pair<int, nodo*>, vector<pair<int, nodo*>>, greater<pair<int, nodo*>>> pq;
	map<nodo*, pair<int, nodo*>> visitados;

	pq.emplace(0, mapaNodos.find(nodoIn)->second);
	while(!pq.empty()){
		auto [costoActual, nodoActual] = pq.top();
		pq.pop();
		if(nodoActual->nombre == nodoFin){			// reconstruir camino
			vector<nodo*> ruta;
			costo = visitados[nodoActual].first;	// el costo final sera el costo acumulado del nodo final
			while(nodoActual->nombre != nodoIn){
				ruta.push_back(nodoActual);
				nodoActual = visitados[nodoActual].second;
			}
			ruta.push_back(mapaNodos.find(nodoIn)->second);
			reverse(ruta.begin(), ruta.end());
			return make_pair(ruta, costo);
		}
		// Expandir nodos del nodoActual
		for(const auto& [nodoVecino, costoVecino] : nodoActual->vecinos){
			int newCosto = costoActual + costoVecino;
			// si el nodo ya fue visitado y el costo acumulado es >= no se expande
			if(visitados.count(nodoVecino) && newCosto >= visitados[nodoVecino].first){
				continue;
			}
			// actualizar costo acumulado, padre del nodo, el contador de veces que se expande un nodo y agregar a pq
			nodoActual->expandidos++;
			visitados[nodoVecino] = {newCosto, nodoActual};
			pq.emplace(newCosto, nodoVecino);
		}
	}
	return make_pair(vector<nodo*>(), -1);
}

pair<vector<nodo*>, int> greedy(map<char, nodo*> mapaNodos){
	vector<nodo*> ruta;
	int costo = 0;
	nodo* nodoActual = mapaNodos.find(nodoIn)->second;
	ruta.push_back(nodoActual);
	while(true){
		int LowerHeuristica = 2147483647;
		int costoTemp = 0;
		int numVecinos = nodoActual->vecinos.size();
		nodo* nodoAExpandir = nodoActual->vecinos[0].first;
		for(int i = 0; i<numVecinos; i++){
			if(nodoActual->vecinos[i].first->heuristica < LowerHeuristica){
				LowerHeuristica = nodoActual->vecinos[i].first->heuristica;
				nodoAExpandir = nodoActual->vecinos[i].first;
				costoTemp = nodoActual->vecinos[i].second;
			}
		}
		costo += costoTemp;
		ruta.push_back(nodoAExpandir);
		nodoActual->expandidos++;
		nodoActual = nodoAExpandir;

		if(nodoActual->nombre == nodoFin){
			return make_pair(ruta, costo);
		}
	}
}

pair<vector<nodo*>, int> aEstrella(map<char, nodo*> mapaNodos){
	priority_queue<nodo*, vector<nodo*>, comparaNodos> pq; 			// falta definir comparaNodos
	map<nodo*, int> valorG;
	map<nodo*, nodo*> padres;
	nodo* nodoInicial = mapaNodos.find(nodoIn)->second;
	pq.push(nodoInicial);
	valorG[nodoInicial] = 0;

	while(!pq.empty()){
		nodo* nodoActual = pq.top();
		pq.pop();
		if(nodoActual->nombre == nodoFin){
			break;
		}
		for(const auto& vecino : nodoActual->vecinos){
			nodo* nodoVecino = vecino.first;
			int costo = vecino.second;
			int nuevoValorG = valorG[nodoActual] + costo;
			// Si el nodo vecino no se ha visitado o el nuevo valorG es menor que el anterior
			if(valorG.find(nodoVecino) == valorG.end()){
				valorG[nodoVecino] = nuevoValorG;
				padres[nodoVecino] = nodoActual;
				pq.push(nodoVecino);
				nodoVecino->expandidos++;
			}
		}
	}
	vector<nodo*> ruta;
	nodo* nodoActual = mapaNodos.find(nodoFin)->second;
	int costo = valorG[nodoActual];
	while(nodoActual != nullptr){
		ruta.push_back(nodoActual);
		nodoActual = padres[nodoActual];
	}
	reverse(ruta.begin(), ruta.end());
	return make_pair(ruta, costo);
}

int main(){
	srand(time(NULL));

	// -------------------------------------------------------------------------------------
	// ------------------------------- depth first search ----------------------------------
	// -------------------------------------------------------------------------------------
	map<char, nodo*> mapaNodos = leerArchivo("entrada.txt");
	// imprimirMapa(mapaNodos);
	cout << "-------------------algoritmo dfs----------------"<< endl;
	// Se obtiene la ruta y costo de la ruta a traves de la funcion, luego se imprimen los datos pedidos
	auto [rutaDfs, costo] = dfs(mapaNodos);
	for(int i=0; i<rutaDfs.size()-1;i++){
		cout << rutaDfs[i]->nombre << " -> ";
	}
	cout << rutaDfs[rutaDfs.size()-1]->nombre << endl;
	cout << "Costo: " << costo << endl;
	for(const auto& pair : mapaNodos){
		cout << pair.second->nombre << ": " << pair.second->expandidos << endl;
	}

	// -------------------------------------------------------------------------------------
	// ----------------------------- Busqueda costo uniforme -------------------------------
	// -------------------------------------------------------------------------------------
	
	mapaNodos = leerArchivo("entrada.txt");
	cout << "-------------------algoritmo busqueda costo uniforme----------------"<< endl;
	auto [rutaBcu, costoBcu] = bcu(mapaNodos);
	for(int i=0; i<rutaBcu.size()-1;i++){
		cout << rutaBcu[i]->nombre << " -> ";
	}
	cout << rutaBcu[rutaBcu.size()-1]->nombre << endl;
	cout << "Costo: " << costoBcu << endl;
	for(const auto& pair : mapaNodos){
		cout << pair.second->nombre << ": " << pair.second->expandidos << endl;
	}

	// -------------------------------------------------------------------------------------
	// ----------------------------- Busqueda greedy ---------------------------------------
	// -------------------------------------------------------------------------------------

	mapaNodos = leerArchivo("entrada.txt");
	cout << "-------------------algoritmo greedy----------------"<< endl;
	auto [rutaGreedy, costoGreedy] = greedy(mapaNodos);
	for(int i=0; i<rutaGreedy.size()-1;i++){
		cout << rutaGreedy[i]->nombre << " -> ";
	}
	cout << rutaGreedy[rutaGreedy.size()-1]->nombre << endl;
	cout << "Costo: " << costoGreedy << endl;
	for(const auto& pair : mapaNodos){
		cout << pair.second->nombre << ": " << pair.second->expandidos << endl;
	}

	// -------------------------------------------------------------------------------------
	// ----------------------------- Busqueda A* -------------------------------------------
	// -------------------------------------------------------------------------------------

	mapaNodos = leerArchivo("entrada.txt");
	cout << "-------------------algoritmo A*----------------"<< endl;
	auto [rutaA, costoA] = aEstrella(mapaNodos);
	for(int i=0; i<rutaA.size()-1;i++){
		cout << rutaA[i]->nombre << " -> ";
	}
	cout << rutaA[rutaA.size()-1]->nombre << endl;
	cout << "Costo: " << costoA << endl;
	for(const auto& pair : mapaNodos){
		cout << pair.second->nombre << ": " << pair.second->expandidos << endl;
	}

	return 0;	
}