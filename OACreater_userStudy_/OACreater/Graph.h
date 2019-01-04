#pragma once  
#include<iostream>  
#include <vector>
#include <set>
#define INF 100000 //��ʾ������  
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
	bool GraphEmpty()const //���Ϊ��  
	{
		if (numEdges == 0)return true;
		else return false;
	}
	bool GraphFull()const //���Ϊ��  
	{
		if (numVertices == maxVertices || numEdges == maxVertices*(maxVertices - 1) / 2)
			return true;
		else
			return false;
	}
	int NumberOfVertices(){ return numVertices; } //���ص�ǰ������  
	int NumberOfEdges(){ return numEdges; }       //���ص�ǰ����  
	virtual int getValue(int i) = 0;                    //ȡ����i��ֵ��i��������0  
	virtual int getWeight(int v1, int v2) = 0;           //ȡ�ߣ�v1��v2����Ȩֵ  
	virtual bool insertVertex(const int& vertex) = 0;   //����һ������vertex  
	virtual bool insertEdge(int v1, int v2, int cost) = 0;//����ߣ�v1,v2��,Ȩֵcost  
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
	Graphmtx(int sz = DefaultVertices);                           //����  
	~Graphmtx()                                                 //����  
	{
		delete[]VerticesList;
		delete[]Edge;
	}
	int getValue(int i)                                           //ȡ����i��ֵ����i��������NULL  
	{
		if (i >= 0 && i < numVertices) return VerticesList[i];
		else return NULL;
	}
	int getWeight(int v1, int v2)                                  //ȡ�ߣ�v1,v2����Ȩֵ����������0  
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
	int getVertexPos(int vertex)                                  //����������ͼ�е�λ��  
	{
		for (int i = 0; i < numVertices; i++)
			if (VerticesList[i] == vertex)return i;
		return -1;                                              //�Ҳ�������-1  
	}
	//��⻷
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
	int *VerticesList;                                            //�����  
	int * *Edge;                                                  //�ڽӾ���  
	//��⻷
	int* visitedFlag;
	SequenceStack loop_stack;
	int innerStep;
	int isRecall;
	int temp;
	int pop_value;
	int heap;
	std::set<int> recordLoopVertex;

};