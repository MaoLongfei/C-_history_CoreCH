#ifndef GRAPH_H_
#define GRAPH_H_

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
using namespace std;

#include "vector_io.h"

#define FORALL_OUTGOING_EDGES(G, u, e) for(unsigned e = G.getFirstEdge(u); e <= G.getLastEdge(u); e++)
#define FORALL_VERTICES(G, u) for(unsigned u = 0; u < G.vertexNumber(); u++)
#define FORALL_EDGES(G, e) for(unsigned e = 0; e < G.edgeNumber(); e++)

class adjacencyGraph;
class overheadGraph;
class tailInformationGraph;

struct edgeConsumptionProfile
{
	int in;
	int out;
	int cost;
};

struct edgeCost
{
	unsigned timeCost;
	edgeConsumptionProfile energyCost;
};

edgeConsumptionProfile edgeConsumptionProfileTranform(unsigned maxCapacity, int energyCost)
{
	edgeConsumptionProfile temp;
	temp.cost = energyCost;
	if (energyCost >= 0)
	{
		assert(energyCost <= maxCapacity);

		temp.in = energyCost;
		temp.out = maxCapacity - energyCost;
	}
	else
	{
		temp.in = 0;
		temp.out = maxCapacity;
	}

	return temp;
}

edgeConsumptionProfile edgeConsumptionProfileCombine(edgeConsumptionProfile edge1, edgeConsumptionProfile edge2)
{
	edgeConsumptionProfile combinedEdge;

	if (edge1.in > (edge1.cost + edge2.in))
	{
		combinedEdge.in = edge1.in;
	}
	else
	{
		combinedEdge.in = edge1.cost + edge2.in;
	}

	if (edge2.out < (edge1.out - edge2.cost))
	{
		combinedEdge.out = edge2.out;
	}
	else
	{
		combinedEdge.out = edge1.out - edge2.cost;
	}
	if ((edge1.cost + edge2.cost)>(edge1.in - edge2.out))
	{
		combinedEdge.cost = (edge1.cost + edge2.cost);
	}
	else
	{
		combinedEdge.cost = edge1.in - edge2.out;
	}
	return combinedEdge;
}

vector<edgeCost> weightGnerate(vector<unsigned> time, vector<int>energy)
{
	vector<edgeCost> temp(time.size());
	for (unsigned i = 0; i < time.size(); ++i)
	{
		edgeConsumptionProfile tempConsumptionProfile = edgeConsumptionProfileTranform(maxCapacity, energy[i]);
		temp[i] = { time[i],tempConsumptionProfile };
	}

	return temp;
}

class adjacencyGraph {

protected:
	vector<unsigned> first_out;
	vector<unsigned> head;
	vector<edgeCost> weight;
	vector<unsigned> time;
	vector<int> energy;

public:
	adjacencyGraph(const vector<unsigned> first_out, const vector<unsigned> head, const vector<edgeCost> weight) :
		first_out(first_out),
		head(head),
		weight(weight)
	{ }

	adjacencyGraph(const unsigned vertexNumber, const unsigned edgeNumber) :
		first_out(vertexNumber + 1),
		head(edgeNumber),
		time(edgeNumber),
		energy(edgeNumber),
		weight(edgeNumber)
	{ }

	adjacencyGraph(const string first_out_filename, const string head_filename, const string time_filename, const string energy_filename) :
		first_out(load_vector<unsigned>(first_out_filename)),
		head(load_vector<unsigned>(head_filename)),
		time(load_vector<unsigned>(time_filename)),
		energy(load_vector<int>(energy_filename))
	{
			weight = weightGnerate(time,energy);//

		}

	adjacencyGraph(const string folder_name) :
//		adjacencyGraph(folder_name + "first_out", folder_name + "head", folder_name + "travel_time" , folder_name + "geo_distance")
		adjacencyGraph(folder_name + "first_out", folder_name + "head", folder_name + "travel_time" , folder_name + "geo_distance")
		{ }

	adjacencyGraph(const adjacencyGraph& g) :
		first_out(g.first_out),
		head(g.head),
		time(g.time),
		energy(g.energy),
		weight(g.weight)
	{ }

	adjacencyGraph(const tailInformationGraph& g);

	static adjacencyGraph reverse(const adjacencyGraph& g);
	
	vector<unsigned> getFirstOut() const {return first_out;}
 	vector<unsigned> getHead() const { return head; }
	vector<edgeCost> getWeight() const { return weight; }

	const unsigned vertexNumber() const { return first_out.size() - 1; }
	const unsigned edgeNumber() const { return head.size(); }

	const unsigned getFirstEdge(unsigned u) const { assert(u < vertexNumber()); return first_out[u]; }
	const unsigned getLastEdge(unsigned u) const { assert(u < vertexNumber()); return first_out[u+1] - 1; }
	const unsigned outgoingEdgeNumber(unsigned u) const { assert(u < vertexNumber()); return first_out[u+1] - first_out[u]; }

	const edgeCost getEdgeWeight(unsigned e) const { assert(e < edgeNumber()); return weight[e]; }
	const unsigned getEdgeHead(unsigned e) const { assert(e < edgeNumber()); return head[e]; }
	const bool isValidEdge(unsigned e) const { assert(e < edgeNumber()); return true; }

	const unsigned getEdge(unsigned u, unsigned v) const {
		assert(u < vertexNumber());
		assert(v < vertexNumber());
		FORALL_OUTGOING_EDGES((*this), u, e) {
			assert(e < edgeNumber());
			if (getEdgeHead(e) == v) return e;
		}
		return -1;
	}

	void setEdgeWeight(unsigned e, edgeCost w) { assert(e < edgeNumber()); weight[e] = w; }
};

class tailInformationGraph {

private:
	unsigned vertices;
	vector<unsigned> tail;
	vector<unsigned> head;
	vector<edgeCost> weight;
	vector<unsigned> time;
	vector<int> energy;

public:
	tailInformationGraph(const int vertices, const string tail_filename, const string head_filename, const string time_filename, const string energy_filename) :
		vertices(vertices)
	{
		tail = load_vector<unsigned>(tail_filename);
		head = load_vector<unsigned>(head_filename);
		time = load_vector<unsigned>(time_filename);
		energy = load_vector<int>(energy_filename);
		weight = weightGnerate(time, energy);
	}

	tailInformationGraph(const adjacencyGraph& g) :
		vertices(g.vertexNumber()),
		tail(g.edgeNumber()),
		head(g.getHead()),
		weight(g.getWeight())
	{
		FORALL_VERTICES(g, u) {
			FORALL_OUTGOING_EDGES(g, u, e) {
				tail[e] = u;
			}
		}
	}

	static tailInformationGraph reverse(tailInformationGraph& g) {
		tailInformationGraph r = g;
		r.tail.swap(r.head);
//		std::cout<<"here reverse"<<std::endl;
		return r;
	}

	const unsigned vertexNumber() const { return vertices; }
	const unsigned edgeNumber() const { return head.size(); }

	const edgeCost getEdgeWeight(unsigned e) const { assert(e < edgeNumber()); return weight[e]; }
	const unsigned getEdgeTail(unsigned e) const { assert(e < edgeNumber()); return tail[e]; }
	const unsigned getEdgeHead(unsigned e) const { assert(e < edgeNumber()); return head[e]; }
};

adjacencyGraph::adjacencyGraph(const tailInformationGraph& g) :
	first_out(g.vertexNumber() + 1),
	head(g.edgeNumber()),
	weight(g.edgeNumber())
{

	vector<unsigned> edgeOrder(edgeNumber());
	for (unsigned i = 0; i < edgeOrder.size(); i++) edgeOrder[i] = i;
	sort(edgeOrder.begin(), edgeOrder.end(),
	[&](const unsigned i1, const unsigned i2)
	{
		return g.getEdgeTail(i1) < g.getEdgeTail(i2);
	}
	);

	vector<unsigned> outDegree(g.vertexNumber());
	FORALL_EDGES((*this), e) {
		outDegree[g.getEdgeTail(edgeOrder[e])]++;
		head[e] = g.getEdgeHead(edgeOrder[e]);
		weight[e] = g.getEdgeWeight(edgeOrder[e]);
	}

	unsigned e = 0;
	FORALL_VERTICES((*this), v) {
		first_out[v] = e;
		e += outDegree[v];
	}
	first_out[g.vertexNumber()] = g.edgeNumber();
}

adjacencyGraph adjacencyGraph::reverse(const adjacencyGraph& g) {
	tailInformationGraph ge(g);
	tailInformationGraph rev = tailInformationGraph::reverse(ge);
	adjacencyGraph ret(rev);
	return ret;
}


class overheadGraph : public adjacencyGraph {
protected:
	vector<unsigned> last_out;
	vector<bool> is_valid;
	vector<unsigned> originalEdges;
	unsigned overhead;

public:
	overheadGraph(const adjacencyGraph& g) :
		adjacencyGraph(g),
		last_out(g.vertexNumber()),
		is_valid(g.edgeNumber()),
		originalEdges(g.edgeNumber()),
		overhead(0)
	{
		for (unsigned v = 0; v < last_out.size(); v++) {
			last_out[v] = g.getLastEdge(v);
		}

		for (unsigned e = 0; e < is_valid.size(); e++) {
			is_valid[e] = true;
		}

		for (unsigned i = 0; i < originalEdges.size(); i++) {
			originalEdges[i] = 1;
		}
	}

	overheadGraph(const adjacencyGraph& g, const unsigned overhead) :
		adjacencyGraph(g.vertexNumber(), g.edgeNumber() * (1 + overhead)),
		last_out(g.vertexNumber()),
		is_valid(g.edgeNumber() * (1 + overhead)),
		originalEdges(g.edgeNumber() * (1 + overhead)),
		overhead(overhead)
	{
		for (unsigned e = 0; e < is_valid.size(); e++) {
			is_valid[e] = false;
		}

		unsigned e = 0;
		FORALL_VERTICES(g, v) {
			first_out[v] = e;
			FORALL_OUTGOING_EDGES(g, v, f) {
				head[e] = g.getEdgeHead(f);
				weight[e] = g.getEdgeWeight(f);
				is_valid[e] = true;
				originalEdges[e] = 1;
				e++;
			}
			last_out[v] = e - 1;
			e += g.outgoingEdgeNumber(v) * overhead;
//			std::cout<<"vertexID : "<<v<<" Edge: "<<e<<std::endl;
		}
//		std::cout<<"here overheadin"<<std::endl;
	}

	overheadGraph(const overheadGraph& g) :
		adjacencyGraph(g),
		last_out(g.last_out),
		is_valid(g.is_valid),
		originalEdges(g.originalEdges),
		overhead(overhead)
	{ }

	adjacencyGraph toNonoverheadGraph() {
		vector<unsigned> new_first_out(vertexNumber() + 1);
		vector<unsigned> new_head(validEdgeNumber());
		vector<edgeCost> new_weight(validEdgeNumber());

		unsigned edgeIndex = 0;
		FORALL_VERTICES((*this), u) {
			new_first_out[u] = edgeIndex;
			FORALL_OUTGOING_EDGES((*this), u, e) {
				if (!isValidEdge(e)) continue;
				new_head[edgeIndex] = head[e];
				new_weight[edgeIndex] = weight[e];
				edgeIndex++;
			}
		}

		new_first_out[vertexNumber()] = new_head.size();

		adjacencyGraph ret(new_first_out, new_head, new_weight);
		return ret;
	}

	const unsigned getLastEdge(unsigned u) const { assert(u < vertexNumber()); return last_out[u]; }
	const unsigned outgoingEdgeNumber(unsigned u) const { assert(u < vertexNumber()); return last_out[u] - first_out[u] + 1; }
	const unsigned getOriginalEdges(unsigned e) const { assert(e < edgeNumber()); assert(is_valid[e]); return originalEdges[e]; }
	const unsigned origionalOutgoingEdgeNumber(unsigned u) const {
		assert(u < vertexNumber());
		int edges = 0;
		FORALL_OUTGOING_EDGES((*this), u, e) {
			if (!is_valid[e]) continue;
			edges += originalEdges[e];
		}
		return edges;
	}
	const edgeCost getEdgeWeight(unsigned e) const { assert(e < edgeNumber()); assert(isValidEdge(e)); return weight[e]; }
	const unsigned getEdgeHead(unsigned e) const { assert(e < edgeNumber()); assert(isValidEdge(e)); return head[e]; }
	const bool isValidEdge(unsigned e) const { assert(e < edgeNumber()); return is_valid[e]; }

	const unsigned getEdge(unsigned u, unsigned v) const {
		assert(u < vertexNumber());
		assert(v < vertexNumber());
		FORALL_OUTGOING_EDGES((*this), u, e) {
			assert(e < edgeNumber());
			if (isValidEdge(e) && getEdgeHead(e) == v) return e;
		}
		return -1;
	}

	const unsigned validEdgeNumber() {
		unsigned edgeNumber = 0;
		FORALL_VERTICES((*this), u) {
			edgeNumber += outgoingEdgeNumber(u);
		}
		return edgeNumber;
	}

	void setEdgeWeight(unsigned e, edgeCost w) { assert(e < edgeNumber()); assert(isValidEdge(e)); weight[e] = w; }

	void deleteEdge(unsigned u, unsigned e) {
		assert(u < vertexNumber());
		assert(e < edgeNumber());
		assert(e >= getFirstEdge(u));
		assert(e <= getLastEdge(u));

		if (!is_valid[e]) return;

		const unsigned last = getLastEdge(u);
		if (e != last) {
			//Swap with last edge
			head[e] = head[last];
			weight[e] = weight[last];
			is_valid[e] = is_valid[last];
			originalEdges[e] = originalEdges[last];
		}
		is_valid[last] = false;
		last_out[u]--;
	}

	void deleteEdges(unsigned u) {
		assert(u < vertexNumber());
		FORALL_OUTGOING_EDGES((*this), u, e) {
			is_valid[e] = false;
		}
		last_out[u] = first_out[u] - 1;
	}

	void addEdge(unsigned u, unsigned v, edgeCost w, unsigned orig = 1) {
		unsigned e = getEdge(u, v);
		if (e != -1 && is_valid[e]) {
			if (weight[e].timeCost > w.timeCost) {
				weight[e] = w;
				originalEdges[e] = orig;
			}
			return;
		}

		//Look for space to the left and to the right of edge array.
		unsigned last = last_out[u];
		unsigned first = first_out[u];

		if (last < edgeNumber() - 1 && !is_valid[last + 1]) {
			head[last + 1] = v;
			weight[last + 1] = w;
			is_valid[last + 1] = true;
			originalEdges[last + 1] = orig;
			last_out[u]++;
		}
		else if (first > 0 && !is_valid[first - 1]) {
			head[first - 1] = v;
			weight[first - 1] = w;
			is_valid[first - 1] = true;
			originalEdges[first - 1] = orig;
			first_out[u]--;
		}
		else if (last == edgeNumber() - 1) {
			head.resize(edgeNumber() + 1);
			weight.resize(edgeNumber() + 1);
			is_valid.resize(edgeNumber() + 1);
			originalEdges.resize(edgeNumber() + 1);
			head[last + 1] = v;
			weight[last + 1] = w;
			is_valid[last + 1] = true;
			originalEdges[last + 1] = orig;
			last_out[u]++;
		}
		else {
			//Copy edges of u to end of array
			unsigned oldSize = edgeNumber();
			unsigned newSize = edgeNumber() + outgoingEdgeNumber(u)*(1 + overhead) + 1;
			head.resize(newSize);
			weight.resize(newSize);
			is_valid.resize(newSize);
			originalEdges.resize(newSize);
			for (unsigned i = 0; i < outgoingEdgeNumber(u); i++) {
				head[oldSize + i] = head[first_out[u] + i];
				weight[oldSize + i] = weight[first_out[u] + i];
				is_valid[oldSize + i] = is_valid[first_out[u] + i];
				originalEdges[oldSize + i] = originalEdges[first_out[u] + i];
				is_valid[first_out[u] + i] = false;
			}
			unsigned lastElement = oldSize + outgoingEdgeNumber(u);
			head[lastElement] = v;
			weight[lastElement] = w;
			is_valid[lastElement] = true;
			originalEdges[lastElement] = orig;
			first_out[u] = oldSize;
			last_out[u] = lastElement;
		}
	}
};

#endif /* GRAPH_H_ */
