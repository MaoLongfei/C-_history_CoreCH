// reference.cpp : Defines the entry point for the console application.

#include "CHQuery.h"
#include "ContractionBuilder.h"
#include "Graph.h"
#include "vector_io.h"
#include "timer.h"
#include <string>
#include <iostream>
using namespace std;


void coreCH(const string graph_folder) {

//	string test_folder = graph_folder + "test/";

	std::cout<<"Loading graph"<<std::endl;
/*
	vector<unsigned> first_out={0,3,6,9,12,15,16,17,17,17};
	vector<unsigned> head =  {3,5,8,0,2,7,1,4,7,0,5,6,2,6,7,6,4};
	vector<unsigned> time = {1,10,20,2,3,2,3,1,5,2,2,7,1,3,10,4,2};
	vector<int> energy ={1,10,20,2,3,2,3,1,5,2,2,7,1,3,10,4,2};
	vector<edgeCost> weight = weightGnerate(time, energy);
	adjacencyGraph graph(first_out,head,weight);
	*/
	adjacencyGraph graph(graph_folder);
	std::cout<<"Getting charging stations"<<std::endl;
	//读取基本信息
	vector<bool> chargingStation (graph.vertexNumber()) ;
	for(unsigned i = 0;i<chargingStation.size();++i)
	{
	    if( i % 1000 ==0)
	    {
		chargingStation[i] = true;
	    }
	    else
	    {
		chargingStation[i] = false;
	    }
	    
	} 
	std::cout<<"Setting up Builder"<<std::endl;
	ContractionBuilder builder(graph,chargingStation);
	long long contractionBegin = get_micro_time();
	cout<<"Core size (min. 1):"<<endl;
	unsigned size = 1;
	cin>>size;
	cout<<endl;
	
	builder.run(size);
	
	std::cout<<"Contraction finished"<<std::endl;														//input value is the size of the core
	long long contractionEnd = get_micro_time();
	
	cout<< "Contraction Time: "<< contractionEnd - contractionBegin<<endl;
	//运行run函数获得增强图和wichtigkeit

	adjacencyGraph aug = builder.getAugmentedGraph();

//	save_vector("graph/stupferich/CH_Core/first_out", aug.getFirstOut());
//	save_vector("graph/stupferich/CH_Core/head", aug.getHead());
//	save_vector("graph/stupferich/CH_Core/weight", aug.getWeight());
	
	vector<unsigned> order = builder.getOrder();
//	for(unsigned i = 0;i<order.size();++i)
	{
//	    cout<<"Order : "<<i<<" is node: "<<order[i]<<endl;
	}

//	save_vector("graph/stupferich/CH_Core/order",order);
	
	CHQuery ch_time(aug, order);
	cout<<"Edge Number after contraction:		"<<aug.getHead().size()<<endl;
//	cout<<aug.getHead().size()<<endl;
	vector<unsigned> ori = load_vector<unsigned>("../../graph/karlsruhe/head");
	
	vector<unsigned> t_first_out = aug.getFirstOut();
	vector<unsigned> t_head = aug.getHead();
	vector<edgeCost> t_cost = aug.getWeight();
	
/*	
	for(unsigned i = 0;i<t_first_out.size()-1;++i)
	{
	    cout<<i<<" : "<<endl;
	    for(unsigned j = t_first_out[i];j<t_first_out[i+1];++j)
	    {
		cout<<"head: "<<t_head[j]<<endl;
		cout<<"time: "<<t_cost[j].timeCost<<endl;
	    }
	}
*/	
	
//	cout<<"Original edges:				 "<<ori.size()<<endl;
	//通过增强图和wichtigkeit查询路径
/*
	vector<unsigned> source = load_vector<unsigned>(test_folder + "source");
	vector<unsigned> target = load_vector<unsigned>(test_folder + "target");
	vector<unsigned> travel_time_length = load_vector<unsigned>(test_folder + "travel_time_length");

	vector<edgeCost>	queryResult(source.size());
	vector<unsigned> queryResultTime(source.size());
	save_vector("graph/stupferich/CH_Core/queryResult",queryResult);
	save_vector("graph/stupferich/CH_Core/queryResultTime",queryResultTime);
	
long long queryBegin = get_micro_time();
    unsigned queryTimes = source.size() / 1000;
	for (unsigned i = 0; i < queryTimes; i++) {
		edgeCost pathCost = ch_time.run(source[i], target[i]);
		unsigned time = pathCost.timeCost;
		queryResult[i] = pathCost;
		queryResultTime[i] = time;
	}
	
long long queryEnd = get_micro_time();

	cout<<"Query time: "<< queryEnd - queryBegin<<" (for "<<queryTimes<<" queries)"<<endl;
	*/
}


int main()
{
	string graph_folder = "./graph/stupferich/";
	coreCH(graph_folder);
    return 0;
}

