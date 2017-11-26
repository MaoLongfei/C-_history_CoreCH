#ifndef CHQUERY_H_
#define CHQUERY_H_

#include "Graph.h"
#include "id_queue.h"
#include "constants.h"

class CHQuery {

private:
	adjacencyGraph forwardGraph;
	adjacencyGraph backwardGraph;
	vector<unsigned> rank;
	MinIDQueue forwardQueue;
	MinIDQueue backwardQueue;
	vector<edgeCost> forwardCost;
	vector<edgeCost> backwardCost;
	vector<unsigned> forwardCount;
	vector<unsigned> backwardCount;
	unsigned runTime;
	edgeCost tentativeDistance;

public:
	CHQuery(const adjacencyGraph& graph, vector<unsigned>& order) :
		forwardGraph(graph),
		backwardGraph(adjacencyGraph::reverse(graph)),
		rank(order.size()),
		forwardQueue(graph.vertexNumber()),
		backwardQueue(graph.vertexNumber()),
		forwardCost(graph.vertexNumber()),
		backwardCost(graph.vertexNumber()),
		forwardCount(graph.vertexNumber()),
		backwardCount(graph.vertexNumber()),
		runTime(0),
		tentativeDistance({ inf_weight ,{ 0 , maxCapacity , 0 } })
	{
		for (unsigned i = 0; i < forwardCost.size(); i++) {
			edgeConsumptionProfile initial = {0 , maxCapacity , 0};
			forwardCost[i] = { inf_weight , initial };
			backwardCost[i] = { inf_weight , initial };
			forwardCount[i] = 0;
			backwardCount[i] = 0;
		}

		for (unsigned i = 0; i < order.size(); i++) {
			rank[order[i]] = i;
		}
	}

	edgeCost getForwardCost(unsigned i) {
		if (runTime != forwardCount[i]) {
			forwardCount[i] = runTime;
			edgeConsumptionProfile initial = { 0 , maxCapacity , 0 };
			forwardCost[i] = { inf_weight , initial };
		}
		return forwardCost[i];
	}

	edgeCost getBackwardCost(unsigned i) {
		if (runTime != backwardCount[i]) {
			backwardCount[i] = runTime;
			edgeConsumptionProfile initial = { 0 , maxCapacity , 0 };
			backwardCost[i] = { inf_weight , initial };
		}
		return backwardCost[i];
	}

	edgeCost run(unsigned source, unsigned target) {
		runTime++;
		edgeConsumptionProfile initial = { 0 , maxCapacity , 0 };
		forwardQueue.clear();
		backwardQueue.clear();
		tentativeDistance = { inf_weight , initial };
		forwardCount[source] = runTime;
		forwardCost[source] = { 0 , initial };
		backwardCount[target] = runTime;
		backwardCost[target] = { 0 , initial };
		forwardQueue.push({source, 0});
		backwardQueue.push({target, 0});

		while (!forwardQueue.empty()) {
			unsigned u = forwardQueue.pop().id;
			if (u == target) break;
			edgeCost distanceU = getForwardCost(u);
			if (distanceU.timeCost > tentativeDistance.timeCost) break;
			FORALL_OUTGOING_EDGES(forwardGraph, u, e) {
				unsigned v = forwardGraph.getEdgeHead(e);
				if (rank[v] <= rank[u]) continue;
				edgeCost distanceV = getForwardCost(v);
				if (distanceU.timeCost + forwardGraph.getEdgeWeight(e).timeCost < distanceV.timeCost) {
					forwardCost[v].timeCost = distanceU.timeCost + forwardGraph.getEdgeWeight(e).timeCost;
					forwardCost[v].energyCost = edgeConsumptionProfileCombine( distanceU.energyCost , forwardGraph.getEdgeWeight(e).energyCost);

					if (forwardQueue.contains_id(v))
						forwardQueue.decrease_key({v, forwardCost[v]});
					else
						forwardQueue.push({v, forwardCost[v]});

//					if (getForwardCost(v).timeCost < tentativeDistance.timeCost)
//					{
//					    tentativeDistance = getForwardCost(v);
//					    tentativeDistance = forwardCost[v];
//					}
					if (getForwardCost(v).timeCost + getBackwardCost(v).timeCost < tentativeDistance.timeCost) {
						tentativeDistance.timeCost = getForwardCost(v).timeCost + getBackwardCost(v).timeCost;
						tentativeDistance.energyCost = edgeConsumptionProfileCombine(getForwardCost(v).energyCost, getBackwardCost(u).energyCost);
					}
				}
			}
		}

		while (!backwardQueue.empty()) {
			unsigned u = backwardQueue.pop().id;
			if (u == source) break;
			edgeCost distanceU = getBackwardCost(u);
			if (distanceU.timeCost > tentativeDistance.timeCost) break;
			FORALL_OUTGOING_EDGES(backwardGraph, u, e) {
				unsigned v = backwardGraph.getEdgeHead(e);
				if (rank[v] <= rank[u]) continue;
				edgeCost distanceV = getBackwardCost(v);
				if (distanceU.timeCost + backwardGraph.getEdgeWeight(e).timeCost < distanceV.timeCost) {
					backwardCost[v].timeCost = distanceU.timeCost + backwardGraph.getEdgeWeight(e).timeCost;
					backwardCost[v].energyCost = edgeConsumptionProfileCombine(distanceU.energyCost, backwardGraph.getEdgeWeight(e).energyCost);
					if (backwardQueue.contains_id(v))
						backwardQueue.decrease_key({v, backwardCost[v]});
					else
						backwardQueue.push({v, backwardCost[v]});
					if (getForwardCost(v).timeCost + getBackwardCost(v).timeCost < tentativeDistance.timeCost) {
						tentativeDistance.timeCost = getForwardCost(v).timeCost + getBackwardCost(v).timeCost;
						tentativeDistance.energyCost = edgeConsumptionProfileCombine( getForwardCost(v).energyCost , getBackwardCost(v).energyCost);
					}
				}
			}
		}

		return tentativeDistance;
	}
};

#endif /* CHQUERY_H_ */
