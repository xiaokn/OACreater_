#pragma once  
#include<iostream>  
#include <vector>
#include <set>
#define INF 100000 //表示正无穷  
const int DefaultVertices = 30;
typedef struct SequenceStack
{
	int* base;
	int* top;
	int stackSize;
}SequenceStack;
class Graph
{
public:
	static const int maxWeight = INF;
	Graph(int size = DefaultVertices){};
	~Graph(){};
	bool GraphEmpty()const //检查为空  
	{
		if (numEdges == 0)return true;
		else return false;
	}
	bool GraphFull()const //检查为满  
	{
		if (numVertices == maxVertices || numEdges == maxVertices*(maxVertices - 1) / 2)
			return true;
		else
			return false;
	}
	int NumberOfVertices(){ return numVertices; } //返回当前顶点数  
	int NumberOfEdges(){ return numEdges; }       //返回当前边数  
	virtual int getValue(int i) = 0;                    //取顶点i的值，i不合理返回0  
	virtual int getWeight(int v1, int v2) = 0;           //取边（v1，v2）的权值  
	virtual bool insertVertex(const int& vertex) = 0;   //插入一个顶点vertex  
	virtual bool insertEdge(int v1, int v2, int cost) = 0;//插入边（v1,v2）,权值cost  
	virtual int getVertexPos(int vertex) = 0;

public:
	int maxVertices;
	int numEdges;
	int numVertices;

};
//////////////////////////////////////////////////////////////////////////
class Graphmtx : public Graph
{

public:
	Graphmtx(int sz = DefaultVertices);                           //构造  
	~Graphmtx()                                                 //析构  
	{
		delete[]VerticesList;
		delete[]Edge;
	}
	int getValue(int i)                                           //取顶点i的值，若i不合理返回NULL  
	{
		if (i >= 0 && i < numVertices) return VerticesList[i];
		else return NULL;
	}
	int getWeight(int v1, int v2)                                  //取边（v1,v2）的权值，不合理返回0  
	{
		if (v1 != -1 && v2 != -1)
			return Edge[v1][v2];
		else
			return 0;
	}
	bool insertVertex(const int& vertex);
	bool insertEdge(int v1, int v2, int cost);
	void inputGraph();
	void outputGraph();
	int getVertexPos(int vertex)                                  //给出顶点在图中的位置  
	{
		for (int i = 0; i < numVertices; i++)
			if (VerticesList[i] == vertex)return i;
		return -1;                                              //找不到返回-1  
	}
	//检测环
	void checkRing();
	void DFS(int startVertax);
	void DFSTraverse();
	void setVisitedFlag(int vertaxID, int value);
	void push_stack(SequenceStack* stack, int value);
	int firstAdjacentVertax(int vertaxID);
	int nextAdjacentVertax(int vertaxID, int nextVertaxID);
	void print_stack(SequenceStack stack);
	void pop_stack(SequenceStack* stack, int* value);
	void clear_stack(SequenceStack* stack);
public:
	int *VerticesList;                                            //顶点表  
	int * *Edge;                                                  //邻接矩阵  
	//检测环
	int* visitedFlag;
	SequenceStack loop_stack;
	int innerStep;
	int isRecall;
	int temp;
	int pop_value;
	int heap;
	std::set<int> recordLoopVertex;

};