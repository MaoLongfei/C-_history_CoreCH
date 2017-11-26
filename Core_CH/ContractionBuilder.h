#ifndef CONTRACTIONBUILDER_H_
#define CONTRACTIONBUILDER_H_

#include "Graph.h"
#include "id_queue.h"
#include "uni_id_queue.h"
#include "constants.h"
#include "vector_io.h"
#include "timer.h"


class WitnessSearch {
private:
	overheadGraph* graph;
	MinIDQueue Q;
	vector<edgeCost> distance;
	vector<unsigned> count;
	unsigned run;
	unsigned source;
	unsigned target;

public:
	WitnessSearch(overheadGraph* graph) :
		graph(graph),
		Q(graph->vertexNumber()),
		distance(graph->vertexNumber()),
		count(graph->vertexNumber()),
		run(0),
		source(-1),
		target(-1)
	{
		for (unsigned i = 0; i < distance.size(); i++) {
			edgeConsumptionProfile initial = { 0,maxCapacity,0 };
			distance[i] = { inf_weight , initial };
			count[i] = 0;
		}
	}

	edgeCost getDistance(unsigned i) {
		if (run != count[i]) {
			count[i] = run;
			edgeConsumptionProfile initial = { 0,maxCapacity,0 };
			distance[i] = { inf_weight , initial };
		}
		return distance[i];
	}

	bool isNecessary(unsigned from, unsigned to, unsigned via, unsigned weight) {
	    
	    /*

	    bool necessarity = true;
	    
	    if(from == to)
	    {
		return false;				//rings exist
	    }
	    else
	    {
		unsigned u = from;
		FORALL_OUTGOING_EDGES((*graph), u, e) 
		{
		    if (!graph->isValidEdge(e)) 
			continue;
		    unsigned v = graph->getEdgeHead(e);
		    if(v== target)
		    {
			necessarity = graph->getEdgeWeight(e).timeCost>weight;
			break;
		    }
		}
		return necessarity;
	    }

	
	     */	
	
		/* 
		 * for normal witness search
	 */	
		if (from != source) {
			run++;
			Q.clear();
			count[from] = run;
			edgeConsumptionProfile initial = { 0,maxCapacity,0 };

			distance[from] = { 0, initial };
			Q.push({ from, 0 });
		}

		source = from;
		target = to;

		while (!Q.empty()) {
			unsigned u = Q.pop().id;
			if (u == target) break;
			if (u == via) continue;
			edgeCost distanceU = getDistance(u);
			if (distanceU.timeCost > weight) break;
			FORALL_OUTGOING_EDGES((*graph), u, e) {
				if (!graph->isValidEdge(e)) continue;
				unsigned v = graph->getEdgeHead(e);
				edgeCost distanceV = getDistance(v);
				if (distanceU.timeCost + graph->getEdgeWeight(e).timeCost < distanceV.timeCost) 
				{
					distance[v].timeCost = distanceU.timeCost + graph->getEdgeWeight(e).timeCost;
					distance[v].energyCost = edgeConsumptionProfileCombine(distanceU.energyCost , graph->getEdgeWeight(e).energyCost);
					if (Q.contains_id(v))
						Q.decrease_key({ v, distance[v] });
					else
						Q.push({ v, distance[v] });
				}
			}
		}

		return getDistance(target).timeCost > weight;

		
	}


};

class ContractionBuilder {

private:
	overheadGraph graph;
	overheadGraph backwardSearchGraph;
	overheadGraph forwardSearchGraph;

	vector<unsigned> level;
	vector<unsigned> order;
	UniMinIDQueue Q;
	unsigned coreSize;
	unsigned contractedNodeNumber;

	vector<bool> chargingStation;
	WitnessSearch witnessSearch;
//protected:
	unsigned shortcutNumber;
	unsigned totalNodes;
	unsigned edgesInCore;
	//	easyWitnessSearch EasyWitnessSearch;

public:
	ContractionBuilder(adjacencyGraph& graph , const vector<bool> chargingStation) :
	//overhead-value: 10 for simpleWitnessSearch, 1 for normal witnessSearch
		graph(graph, 1),
		backwardSearchGraph(adjacencyGraph::reverse(graph), 1),
		forwardSearchGraph(graph, 1),
		level(graph.vertexNumber()),
		Q(graph.vertexNumber()),
		chargingStation(chargingStation),
		witnessSearch(&(this->forwardSearchGraph)),
		contractedNodeNumber(0),
		shortcutNumber(0),
		edgesInCore(0)
		
		//		EasyWitnessSearch(&(this->forwardSearchGraph))
	{
		for (unsigned i = 0; i < level.size(); i++) 
			level[i] = 0;
		vector<unsigned> tempNode = load_vector<unsigned>("../../graph/karlsruhe/first_out");
		//vector<unsigned> tempNode={0,3,6,9,12,15,16,17,17,17};
		totalNodes = tempNode.size() -1;
		vector<unsigned> tempEdge = load_vector<unsigned>("../../graph/karlsruhe/head");
		//vector<unsigned> tempEdge =  {3,5,8,0,2,7,1,4,7,0,5,6,2,6,7,6,4};
		edgesInCore = tempEdge.size();
		cout<<"totalNodes:				 "<<totalNodes<<endl;
	}
	
	vector<unsigned> getOrder() { return order; }
	adjacencyGraph getAugmentedGraph() { return graph.toNonoverheadGraph(); }

	unsigned getKey(unsigned v) {
		unsigned added = 0;
		unsigned addedOriginal = 0;

		FORALL_OUTGOING_EDGES(backwardSearchGraph, v, e) {
			if (!backwardSearchGraph.isValidEdge(e)) continue;
			unsigned u = backwardSearchGraph.getEdgeHead(e);
			FORALL_OUTGOING_EDGES(forwardSearchGraph, v, f) {
				if (!forwardSearchGraph.isValidEdge(f)) continue;
				unsigned w = forwardSearchGraph.getEdgeHead(f);
				edgeCost shortcutWeight;
				shortcutWeight.timeCost = backwardSearchGraph.getEdgeWeight(e).timeCost + forwardSearchGraph.getEdgeWeight(f).timeCost;
				shortcutWeight.energyCost = edgeConsumptionProfileCombine(backwardSearchGraph.getEdgeWeight(e).energyCost, forwardSearchGraph.getEdgeWeight(f).energyCost);
				if (witnessSearch.isNecessary(u, w, v, shortcutWeight.timeCost)) {
//				if (EasyWitnessSearch.fasterShortcut(u, w, v, shortcutWeight)) {
					added++;
					addedOriginal += backwardSearchGraph.getOriginalEdges(e) + forwardSearchGraph.getOriginalEdges(f);
				}
			}
		}

		unsigned deleted = forwardSearchGraph.outgoingEdgeNumber(v) + backwardSearchGraph.outgoingEdgeNumber(v);
		unsigned deletedOriginal = forwardSearchGraph.origionalOutgoingEdgeNumber(v) + backwardSearchGraph.origionalOutgoingEdgeNumber(v);

		return level[v] + (deleted == 0 ? 0 : added / deleted) + (deletedOriginal == 0 ? 0 : addedOriginal / deletedOriginal);
	}

	void run(unsigned nodeInCore) {
	    long long beginTime = get_micro_time();
	    coreSize = nodeInCore;
	    Q.clear();
		
		for (unsigned i = 0; i < graph.vertexNumber(); i++) {
		    if(chargingStation[i])
		    {
			Q.push({i,inf_weight});
		    }
		    else
		    {
			Q.push({ i, getKey(i) });
		    }
		}
		
		while(!Q.empty())
		{
		    UniIDKeyPair temp =Q.pop();		    
		    if(contractedNodeNumber + nodeInCore < graph.vertexNumber())
		    {
			order.push_back(temp.id);
			contract(temp.id);
			contractedNodeNumber++;
//			if(contractedNodeNumber%10000 == 0)
				{
				    long long endTime = get_micro_time();
				    cout<<"ID: "<<temp.id<<"         key: "<<temp.key<<endl;
				    cout <<"Node "<<temp.id<< " is being contracted "  << endl;
				    cout<<"contractedNodeNumber	:		"<< contractedNodeNumber <<endl;
				    cout<<"uncontractedNodeNumber:			"<<totalNodes - contractedNodeNumber<<endl;
				    cout<<"shortcutNumber:				"<<shortcutNumber<<endl;
				    cout<<"edgesInCore: "<<edgesInCore<<endl;
				    cout<<"timeCost till now: "<<endTime - beginTime<<endl;
				}
		    }
		    else
		    {
			cout <<"Node "<<temp.id<< " will not be contracted "  << endl;
			order.push_back(temp.id);
		    }
//		    cout<<"total contracted NodeNumber:		"<<contractedNodeNumber<<endl;
		}
	}

	void contract(unsigned v) {

		vector<unsigned> forwardNeighbors;
		vector<unsigned> backwardNeighbors;

		FORALL_OUTGOING_EDGES(forwardSearchGraph, v, e) {
			if (!forwardSearchGraph.isValidEdge(e)) continue;
			forwardNeighbors.push_back(forwardSearchGraph.getEdgeHead(e));
		}

		FORALL_OUTGOING_EDGES(backwardSearchGraph, v, e) {
			if (!backwardSearchGraph.isValidEdge(e)) continue;
			backwardNeighbors.push_back(backwardSearchGraph.getEdgeHead(e));
		}

		FORALL_OUTGOING_EDGES(backwardSearchGraph, v, e) {
			if (!backwardSearchGraph.isValidEdge(e)) continue;
			unsigned u = backwardSearchGraph.getEdgeHead(e);
			FORALL_OUTGOING_EDGES(forwardSearchGraph, v, f) {
				if (!forwardSearchGraph.isValidEdge(f)) continue;
				unsigned w = forwardSearchGraph.getEdgeHead(f);
				edgeCost shortcutWeight;
				shortcutWeight.timeCost = backwardSearchGraph.getEdgeWeight(e).timeCost + forwardSearchGraph.getEdgeWeight(f).timeCost;
				shortcutWeight.energyCost = edgeConsumptionProfileCombine(backwardSearchGraph.getEdgeWeight(e).energyCost, forwardSearchGraph.getEdgeWeight(f).energyCost);
				if (witnessSearch.isNecessary(u, w, v, shortcutWeight.timeCost)) 
				{
				    
				    	shortcutNumber++;
					edgesInCore++;
					
					//if there is an existed edge but with bigger weight, the new shortcut will be necessary, but the number of edges in core should not change
					FORALL_OUTGOING_EDGES(forwardSearchGraph,u,e)
					{
					    if(!forwardSearchGraph.isValidEdge(e))
						continue;
					     unsigned target  = forwardSearchGraph.getEdgeHead(e);
					    if(target == w)
					    {
						edgesInCore--;
					    }
					}
					
					unsigned originalEdges = backwardSearchGraph.getOriginalEdges(e) + forwardSearchGraph.getOriginalEdges(f);
					forwardSearchGraph.addEdge(u, w, shortcutWeight, originalEdges);
					backwardSearchGraph.addEdge(w, u, shortcutWeight, originalEdges);
					graph.addEdge(u, w, shortcutWeight);
//					cout<<"shortcut added: from "<<u<<" to "<<w <<" with weight "<<shortcutWeight.timeCost<<endl;
				}
			}
		}

		for (unsigned i = 0; i < forwardNeighbors.size(); i++) {
			unsigned e = forwardSearchGraph.getEdge(v, forwardNeighbors[i]);
			assert(e != -1);
			forwardSearchGraph.deleteEdge(v, e);
			unsigned f = backwardSearchGraph.getEdge(forwardNeighbors[i], v);
			assert(f != -1);
			backwardSearchGraph.deleteEdge(forwardNeighbors[i], f);
		}

		for (unsigned i = 0; i < backwardNeighbors.size(); i++) {
			unsigned e = backwardSearchGraph.getEdge(v, backwardNeighbors[i]);
			assert(e != -1);
			backwardSearchGraph.deleteEdge(v, e);
			unsigned f = forwardSearchGraph.getEdge(backwardNeighbors[i], v);
			assert(f != -1);
			forwardSearchGraph.deleteEdge(backwardNeighbors[i], f);
		}
		
		vector<unsigned> neighbors = forwardNeighbors;
		neighbors.insert(neighbors.end(), backwardNeighbors.begin(), backwardNeighbors.end());
		
//		cout<<"Contraction of node "<<v<<" finished."<<endl;
		edgesInCore = edgesInCore - neighbors.size();
//		cout<<"edgesInCore: "<<edgesInCore<<endl;
	}

};

#endif /* CONTRACTIONBUILDER_H_ */
