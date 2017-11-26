#ifndef SIMPLECONTRACTIONBUILDER_H_
#define SIMPLECONTRACTIONBUILDER_H_

#include "Graph.h"
#include "id_queue.h"
#include "constants.h"

class SimpleWitnessSearch {
private:
	overheadGraph* graph;
	MinIDQueue Q;
	vector<unsigned> distance;
	vector<unsigned> timestamp;
	unsigned time;
	unsigned source;
	unsigned target;

public:
	SimpleWitnessSearch(overheadGraph* graph) :
		graph(graph),
		Q(graph->vertexNumber()),
		distance(graph->vertexNumber()),
		timestamp(graph->vertexNumber()),
		time(0),
		source(-1),
		target(-1)
	{
		for (unsigned i = 0; i < distance.size(); i++) {
			distance[i] = inf_weight;
			timestamp[i] = 0;
		}
	}

	unsigned getDistance(unsigned i) {
		if (time != timestamp[i]) {
			timestamp[i] = time;
			distance[i] = inf_weight;
		}
		return distance[i];
	}

	bool isNecessary(unsigned from, unsigned to, unsigned via, unsigned weight) {
		if (from != source) {
			time++;
			Q.clear();
			timestamp[from] = time;
			distance[from] = 0;
			Q.push({from, 0});
		}

		source = from;
		target = to;

		while (!Q.empty()) {
			unsigned u = Q.pop().id;
			if (u == target) break;
			if (u == via) continue;
			unsigned distanceU = getDistance(u);
			if (distanceU > weight) break;
			FORALL_OUTGOING_EDGES((*graph), u, e) {
				if (!graph->isValidEdge(e)) continue;
				unsigned v = graph->getEdgeHead(e);
				unsigned distanceV = getDistance(v);
				if (distanceU + graph->getEdgeWeight(e) < distanceV) {
					distance[v] = distanceU + graph->getEdgeWeight(e);
					if (Q.contains_id(v))
						Q.decrease_key({v, distance[v]});
					else
						Q.push({v, distance[v]});
				}
			}
		}

		return getDistance(target) > weight;
	}
};

class SimpleContractionBuilder {

private:
	overheadGraph graph;
	overheadGraph backwardOverheadGraph;
	overheadGraph backwardSearchGraph;
	vector<unsigned> level;
	vector<unsigned> order;
	WitnessSearch witnessSearch;

public:
	SimpleContractionBuilder(adjacencyGraph& graph, vector<unsigned> order) :
		graph(graph, 1),
		backwardOverheadGraph(graph, 1),
		backwardSearchGraph(adjacencyGraph::reverse(graph), 1),
		level(graph.vertexNumber()),
		order(order),
		witnessSearch(&(this->backwardOverheadGraph))
	 { }

	adjacencyGraph getAugmentedGraph() { return graph.toNonoverheadGraph(); }

	void run() {
		for (unsigned i = 0; i < order.size(); i++) {
			contract(order[i]);
		}
	}

	void contract(unsigned v) {
		cout << "Contracting " << v << endl;
		vector<unsigned> forwardNeighbors;
		vector<unsigned> backwardNeighbors;

		FORALL_OUTGOING_EDGES(backwardOverheadGraph, v, e) {
			if (!backwardOverheadGraph.isValidEdge(e)) continue;
			forwardNeighbors.push_back(backwardOverheadGraph.getEdgeHead(e));
		}

		FORALL_OUTGOING_EDGES(backwardSearchGraph, v, e) {
			if (!backwardSearchGraph.isValidEdge(e)) continue;
			backwardNeighbors.push_back(backwardSearchGraph.getEdgeHead(e));
		}

		FORALL_OUTGOING_EDGES(backwardSearchGraph, v, e) {
			if (!backwardSearchGraph.isValidEdge(e)) continue;
			unsigned u = backwardSearchGraph.getEdgeHead(e);
			FORALL_OUTGOING_EDGES(backwardOverheadGraph, v, f) {
				if (!backwardOverheadGraph.isValidEdge(f)) continue;
				unsigned w = backwardOverheadGraph.getEdgeHead(f);
				unsigned shortcutWeight = backwardSearchGraph.getEdgeWeight(e) + backwardOverheadGraph.getEdgeWeight(f);
				if (witnessSearch.isNecessary(u, w, v, shortcutWeight)) {
					backwardOverheadGraph.addEdge(u, w, shortcutWeight);
					backwardSearchGraph.addEdge(w, u, shortcutWeight);
					graph.addEdge(u, w, shortcutWeight);
				}
			}
		}

		for (unsigned i = 0; i < forwardNeighbors.size(); i++) {
			unsigned e = backwardOverheadGraph.getEdge(v, forwardNeighbors[i]);
			assert(e != -1);
			backwardOverheadGraph.deleteEdge(v, e);
			unsigned f = backwardSearchGraph.getEdge(forwardNeighbors[i], v);
			assert(f != -1);
			backwardSearchGraph.deleteEdge(forwardNeighbors[i], f);
		}

		for (unsigned i = 0; i < backwardNeighbors.size(); i++) {
			unsigned e = backwardSearchGraph.getEdge(v, backwardNeighbors[i]);
			assert(e != -1);
			backwardSearchGraph.deleteEdge(v, e);
			unsigned f = backwardOverheadGraph.getEdge(backwardNeighbors[i], v);
			assert(f != -1);
			backwardOverheadGraph.deleteEdge(backwardNeighbors[i], f);
		}
	}

};

#endif /* SIMPLECONTRACTIONBUILDER_H_ */
