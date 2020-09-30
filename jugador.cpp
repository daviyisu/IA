#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <functional>
#include <vector>

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.

Action ComportamientoJugador::think(Sensores sensores)
{
	if (sensores.nivel == 4)
	{

		Action accion = actIDLE;
		bool mapaNuevo = descubrirMapa(sensores);
		unsigned char contenidoCasilla;

		if (mapaNuevo or nuevoObjetivo(sensores))
		{
			hayplan = false;
			destino.fila = sensores.destinoF;
			destino.columna = sensores.destinoC;
		}

		if (!hayplan)
		{
			actualizaPlan(sensores);
		}

		if (hayplan and plan.size() > 0)
		{

			if (sensores.superficie[2] == 'a' || (sensores.terreno[0] == 'X' and sensores.bateria < 2250))
			{
				accion = actIDLE;
			}
			else
			{
				accion = plan.front();
				plan.erase(plan.begin());
			}
		}

		return accion;
	}
	else
	{
		if (!hayplan)
		{
			actual.fila = sensores.posF;
			actual.columna = sensores.posC;
			actual.orientacion = sensores.sentido;
			destino.fila = sensores.destinoF;
			destino.columna = sensores.destinoC;
			hayplan = pathFinding(sensores.nivel, actual, destino, plan);
		}

		Action sigAccion;
		if (hayplan and plan.size() > 0)
		{
			sigAccion = plan.front();
			plan.erase(plan.begin());
		}

		return sigAccion;
	}
}

void ComportamientoJugador::actualizaPlan(Sensores sensores)
{
	actual.fila = sensores.posF;
	actual.columna = sensores.posC;
	actual.orientacion = sensores.sentido;
	hayplan = pathFinding(sensores.nivel, actual, destino, plan);
}

bool ComportamientoJugador::nuevoObjetivo(Sensores sensores)
{
	return destino.fila != sensores.destinoF or destino.columna != sensores.destinoC;
}

bool ComportamientoJugador::descubrirMapa(Sensores sensores)
{
	mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];
	int i_izq, i_dere, z, j;
	bool mapaNuevo = false;
	switch (sensores.sentido)
	{
	case norte:

		z = 1;

		for (int i = -1; i > -4; i--)
		{
			for (int j = i; j <= -i; j++)
			{
				if (mapaResultado[sensores.posF + i][sensores.posC + j] == '?')
				{
					mapaNuevo = true;
				}

				mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.terreno[z];
				z++;
			}
		}
		break;

	case este:

		z = 1;

		for (int i = 1; i < 4; i++)
		{
			for (int j = i; j >= -i; j--)
			{
				if (mapaResultado[sensores.posF - j][sensores.posC + i] == '?')
				{
					mapaNuevo = true;
				}

				mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.terreno[z];
				z++;
			}
		}

		break;

	case sur:
		z = 1;

		for (int i = 1; i < 4; i++)
		{
			for (int j = i; j >= -i; j--)
			{
				if (mapaResultado[sensores.posF + i][sensores.posC + j] == '?')
				{
					mapaNuevo = true;
				}

				mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.terreno[z];
				z++;
			}
		}

		break;

	case oeste:
		z = 1;

		for (int i = 1; i < 4; i++)
		{
			for (int j = i; j >= -i; j--)
			{
				if (mapaResultado[sensores.posF + j][sensores.posC - i] == '?')
				{
					mapaNuevo = true;
				}

				mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.terreno[z];
				z++;
			}
		}
		break;
	}

	return mapaNuevo;
}

// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan)
{
	switch (level)
	{
	case 1:
		cout << "Busqueda en profundad\n";
		return pathFinding_Profundidad(origen, destino, plan);
		break;
	case 2:
		cout << "Busqueda en Anchura\n";
		return pathFinding_Anchura(origen, destino, plan);
		break;
	case 3:
		cout << "Busqueda Costo Uniforme\n";
		return pathFinding_PorCosto(origen, destino, plan);
		break;
	case 4:
		cout << "Busqueda para el reto\n";
		return pathFinding_Reto(origen, destino, plan);
		break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}

//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla)
{
	if (casilla == 'P' or casilla == 'M')
		return true;
	else
		return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st)
{
	int fil = st.fila, col = st.columna;

	// calculo cual es la casilla de delante del agente
	switch (st.orientacion)
	{
	case 0:
		fil--;
		break;
	case 1:
		col++;
		break;
	case 2:
		fil++;
		break;
	case 3:
		col--;
		break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil < 0 or fil >= mapaResultado.size())
		return true;
	if (col < 0 or col >= mapaResultado[0].size())
		return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col]))
	{
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
		st.fila = fil;
		st.columna = col;
		return false;
	}
	else
	{
		return true;
	}
}

struct ComparaEstados
{
	bool operator()(const estado &a, const estado &n) const
	{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
			(a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

struct ComparaDistancias
{
	bool operator()(const nodo &a, const nodo &b) const
	{
		return (a.G + a.H > b.G + b.H);
	}
};

struct ComparaCoste
{
	bool operator()(const nodo &a, const nodo &b) const
	{
		if (a.coste_actual > b.coste_actual)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan)
{
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;					   // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

	while (!pila.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
		if (generados.find(hijoTurnR.st) == generados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
		if (generados.find(hijoTurnL.st) == generados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (generados.find(hijoForward.st) == generados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty())
		{
			current = pila.top();
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

bool ComportamientoJugador::tieneBikini(const estado &st) const
{
	return mapaResultado[st.fila][st.columna] == 'K';
}

bool ComportamientoJugador::tieneZapatillas(const estado &st) const
{
	return mapaResultado[st.fila][st.columna] == 'D';
}

bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan)
{
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;					   // Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

	while (!cola.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
		if (generados.find(hijoTurnR.st) == generados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
		if (generados.find(hijoTurnL.st) == generados.end())
		{
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (generados.find(hijoForward.st) == generados.end())
			{
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty())
		{
			current = cola.front();
			while (generados.find(current.st) != generados.end() && !cola.empty())	//Eliminar nodos ya generados
			{
				cola.pop();
				current = cola.front();
			}
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

bool ComportamientoJugador::pathFinding_PorCosto(const estado &origen, const estado &destino, list<Action> &plan)
{
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> generados;				 // Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaCoste> q2; // Lista de Abiertos

	nodo current;
	int costo_padre = 0;
	current.st = origen;
	current.secuencia.empty();
	current.coste_actual = 0;
	current.st.tieneBikini = false;
	current.st.tieneZapatillas = false;

	q2.push(current);

	while (!q2.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		q2.pop();
		generados.insert(current.st);
		costo_padre = current.coste_actual;

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.tieneBikini = current.st.tieneBikini;
		hijoTurnR.st.tieneZapatillas = current.st.tieneZapatillas;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
		if (generados.find(hijoTurnR.st) == generados.end())
		{
			hijoTurnR.coste_actual = devuelveCoste(hijoTurnR.st) + costo_padre;
			hijoTurnR.secuencia.push_back(actTURN_R);
			q2.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.tieneBikini = current.st.tieneBikini;
		hijoTurnL.st.tieneZapatillas = current.st.tieneZapatillas;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
		if (generados.find(hijoTurnL.st) == generados.end())
		{
			hijoTurnL.coste_actual = devuelveCoste(hijoTurnL.st) + costo_padre;
			hijoTurnL.secuencia.push_back(actTURN_L);
			q2.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		hijoForward.st.tieneBikini = current.st.tieneBikini;
		hijoForward.st.tieneZapatillas = current.st.tieneZapatillas;
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (generados.find(hijoForward.st) == generados.end())
			{
				hijoForward.coste_actual = devuelveCoste(hijoForward.st) + costo_padre;
				hijoForward.secuencia.push_back(actFORWARD);
				q2.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!q2.empty())
		{
			current = q2.top();
		}

		if (tieneBikini(current.st))
		{
			current.st.tieneBikini = true;
		}

		if (tieneZapatillas(current.st))
		{
			current.st.tieneZapatillas = true;
		}

		if (!q2.empty()) //Eliminar nodos ya generados
		{
			current = q2.top();
			while (generados.find(current.st) != generados.end() && !q2.empty())
			{
				q2.pop();
				current = q2.top();
			}
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

bool ComportamientoJugador::pathFinding_Reto(const estado &origen, const estado &destino, list<Action> &plan)
{
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado, ComparaEstados> generados;					  // Lista de Cerrados
	priority_queue<nodo, vector<nodo>, ComparaDistancias> q2; // Lista de Abiertos

	nodo current;
	int costo_padre = 0;
	current.st = origen;
	current.secuencia.empty();
	current.coste_actual = 0;
	current.st.tieneBikini = false;
	current.st.tieneZapatillas = false;
	current.G = 0;
	current.H = 0;

	q2.push(current);

	while (!q2.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna))
	{

		q2.pop();
		generados.insert(current.st);
		costo_padre = current.coste_actual;

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion + 1) % 4;
		hijoTurnR.st.tieneBikini = current.st.tieneBikini;
		hijoTurnR.st.tieneZapatillas = current.st.tieneZapatillas;
		hijoTurnR.H = devuelveHeuristica(hijoTurnR);
		hijoTurnR.G = devuelveCoste(hijoTurnR.st) + costo_padre;

		if (generados.find(hijoTurnR.st) == generados.end())
		{
			hijoTurnR.secuencia.push_back(actTURN_R);
			q2.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion + 3) % 4;
		hijoTurnL.st.tieneBikini = current.st.tieneBikini;
		hijoTurnL.st.tieneZapatillas = current.st.tieneZapatillas;
		hijoTurnL.G = devuelveCoste(hijoTurnL.st) + costo_padre;
		hijoTurnL.H = devuelveHeuristica(hijoTurnL);
		if (generados.find(hijoTurnL.st) == generados.end())
		{

			hijoTurnL.secuencia.push_back(actTURN_L);
			q2.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		hijoForward.st.tieneBikini = current.st.tieneBikini;
		hijoForward.st.tieneZapatillas = current.st.tieneZapatillas;
		hijoForward.G = devuelveCoste(hijoTurnL.st) + costo_padre;
		hijoForward.H = devuelveHeuristica(hijoTurnL);
		if (!HayObstaculoDelante(hijoForward.st))
		{
			if (generados.find(hijoForward.st) == generados.end())
			{
				hijoForward.coste_actual = devuelveCoste(hijoForward.st) + costo_padre;
				hijoForward.secuencia.push_back(actFORWARD);
				q2.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!q2.empty())
		{
			current = q2.top();
		}

		if (tieneBikini(current.st))
		{
			current.st.tieneBikini = true;
		}

		if (tieneZapatillas(current.st))
		{
			current.st.tieneZapatillas = true;
		}

		if (!q2.empty()) //Eliminar nodos ya generados
		{
			current = q2.top();
			while (generados.find(current.st) != generados.end() && !q2.empty())
			{
				q2.pop();
				current = q2.top();
			}
		}
	}

	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna)
	{
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else
	{
		cout << "No encontrado plan\n";
	}

	return false;
}

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan)
{
	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			cout << "A ";
		}
		else if (*it == actTURN_R)
		{
			cout << "D ";
		}
		else if (*it == actTURN_L)
		{
			cout << "I ";
		}
		else
		{
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

void AnularMatriz(vector<vector<unsigned char>> &m)
{
	for (int i = 0; i < m[0].size(); i++)
	{
		for (int j = 0; j < m.size(); j++)
		{
			m[i][j] = 0;
		}
	}
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan)
{
	AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it != plan.end())
	{
		if (*it == actFORWARD)
		{
			switch (cst.orientacion)
			{
			case 0:
				cst.fila--;
				break;
			case 1:
				cst.columna++;
				break;
			case 2:
				cst.fila++;
				break;
			case 3:
				cst.columna--;
				break;
			}
			mapaConPlan[cst.fila][cst.columna] = 1;
		}
		else if (*it == actTURN_R)
		{
			cst.orientacion = (cst.orientacion + 1) % 4;
		}
		else
		{
			cst.orientacion = (cst.orientacion + 3) % 4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor)
{
	return false;
}

int ComportamientoJugador::devuelveHeuristica(const nodo &origen) const
{
	int result = abs(origen.st.fila - destino.fila) + abs(origen.st.columna - destino.columna);
	return result;
}

bool ComportamientoJugador::heDescubierto(Sensores sensores) const
{
}

int ComportamientoJugador::devuelveCoste(const estado &st) const
{
	char tipo = mapaResultado[st.fila][st.columna];
	int result;
	switch (tipo)
	{
	case 'A':
		if (st.tieneBikini)
		{
			result = 10;
		}
		else
		{
			result = 100;
		}
		break;

	case 'B':
		if (st.tieneZapatillas)
		{
			result = 5;
		}
		else
		{
			result = 50;
		}
		break;
	case 'T':
		result = 2;
		break;

	default:
		result = 1;
		break;
	}
	return result;
}
