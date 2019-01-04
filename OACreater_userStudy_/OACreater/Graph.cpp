#include "Graph.h"
//ͼ�Ĺ���
Graphmtx::Graphmtx(int sz)   //���캯��  
{
	maxVertices = sz;
	numVertices = 0;
	numEdges = 0;
	innerStep = 0;
	isRecall = 0;
	int i, j;
	VerticesList = new int[maxVertices];
	Edge = new int *[maxVertices];
	for (i = 0; i < maxVertices; i++)
		Edge[i] = new int[maxVertices];
	for (i = 0; i < maxVertices; i++)
		for (j = 0; j < maxVertices; j++)
			Edge[i][j] = (i == j) ? 0 : maxWeight;
	//��⻷
	visitedFlag = new int[maxVertices];
	for (i = 0; i < maxVertices; i++)
	{
		visitedFlag[i] = 0;
	}
	loop_stack.base = new int[maxVertices];
	loop_stack.top = loop_stack.base;
	loop_stack.stackSize = maxVertices;
}
bool Graphmtx::insertVertex(const int& vertex) //����һ������  
{
	if (numVertices == maxVertices)return false; //���������������false  
	VerticesList[numVertices++] = vertex;
	return true;
}
bool Graphmtx::insertEdge(int v1, int v2, int cost)//����һ����  
{
	if (v1 > -1 && v1<numVertices && v2>-1 && v2 < numVertices) //�������  
	{
		if (Edge[v1][v2] == maxWeight)
		{
			Edge[v1][v2] = Edge[v2][v1] = cost;
			numEdges++;
			return true;
		}
		else
		{
			std::cout << "�ñ��Ѵ��ڣ����ʧ��" << std::endl;
			return false;
		}
	}
	else return false;
}
void Graphmtx::inputGraph()
{
	//ͨ��������������in����n�Ķ����e������ߵ���Ϣ�����ڽӾ����ʾ��ͼG���ڽӾ����ʼ�������ڹ��캯�����  
	int i, j, k, m, n;
	int e1, e2;
	int weight;
	std::cout << "�����붥�����ͱ���(�ո����):" << std::endl;
	std::cin >> n >> m;  //�������n�ı���m
	//
	maxVertices = n;
	std::cout << "���������붥��:" << std::endl;
	for (i = 0; i < n; i++)//���붥�㣬���������  
	{
		std::cin >> e1;
		this->insertVertex(e1);
		//G.insertVertex(e1);  
	}
	std::cout << "����������ߣ����� v1 v2 weight ��" << std::endl;
	i = 0;
	while (i < m)
	{
		std::cin >> e1 >> e2 >> weight;
		j = this->getVertexPos(e1);//�鶥���  
		k = this->getVertexPos(e2);
		if (j == -1 || k == -1)
		{
			std::cout << "�����˵���Ϣ�����������룡" << std::endl;
		}
		else
		{
			if (this->insertEdge(j, k, weight))
				i++;
		}
	}

}
void Graphmtx::outputGraph()
{
	//���ͼ�����ж���ͱ���Ϣ  
	int i, j, n, m;
	int e1, e2;
	int weight;
	n = this->NumberOfVertices();     //����  
	m = this->NumberOfEdges();       //����  
	std::cout << "�������ı���Ϊ��";
	std::cout << n << "," << m << std::endl;      //��������ͱ���  
	std::cout << "��������Ϊ��" << std::endl;
	for (i = 0; i < n; i++)
	{
		for (j = i + 1; j < n; j++)
		{
			weight = this->getWeight(i, j);
			if (weight > 0 && weight < maxWeight)
			{
				e1 = this->getValue(i);
				e2 = this->getValue(j);
				std::cout << "(" << e1 << "," << e2 << "," << weight << ")" << std::endl;
			}
		}
	}
}
//��⻷
void Graphmtx::checkRing()
{
	std::cout << "�����\n";
}
//�������������⻷
void Graphmtx::DFS(int startVertax)
{
	setVisitedFlag(startVertax, 1);
	int nextVertax;
	push_stack(&loop_stack, startVertax);
	nextVertax = firstAdjacentVertax(startVertax);
	innerStep++;
	for (;;)
	{
		if (nextVertax != -1)
		{
			if (visitedFlag[nextVertax] == 1 && nextVertax == heap && innerStep == 2)
			{
				nextVertax = nextAdjacentVertax(startVertax, nextVertax);
				
				continue;
			}
			else if (visitedFlag[nextVertax] == 1 && nextVertax == heap && innerStep != 2)
			{
				
				print_stack(loop_stack);
				nextVertax = nextAdjacentVertax(startVertax, nextVertax);
				
				continue;
			}
			else if (visitedFlag[nextVertax] == 0)
			{
				DFS(nextVertax);
			}
			if (isRecall == 1)
			{
				//std::cout << "============nextAdjacentVertax start\n";
				innerStep--;
				temp = nextVertax;
				nextVertax = nextAdjacentVertax(startVertax, nextVertax);
				pop_stack(&loop_stack, &pop_value);
				setVisitedFlag(temp, 0);
				isRecall = 0;
				//std::cout << "============nextAdjacentVertax end\n";
				continue;
			}
			nextVertax = nextAdjacentVertax(startVertax, nextVertax);
		}
		else if (nextVertax == -1)
		{
			isRecall = 1;
			break;
		}
	}
	
}
void Graphmtx::DFSTraverse()
{
	int i;
	for (heap = 0; heap < maxVertices; heap++)
	{
		for (i = 0; i < maxVertices; i++)
		{
			visitedFlag[i] = 0;
		}
		if (visitedFlag[heap] == 0)
		{
			printf("\n-------------------the loop start and end with %d----------------\n", heap);
			clear_stack(&loop_stack);
			innerStep = 0;
			isRecall = 0;
			DFS(heap);
		}
	}
}
void Graphmtx::setVisitedFlag(int vertaxID, int value)
{
	if (vertaxID<maxVertices)
	visitedFlag[vertaxID] = value;
}

void Graphmtx::push_stack(SequenceStack* stack, int value)
{
	*(stack->top) = value;
	(stack->top)++;
}

int Graphmtx::firstAdjacentVertax(int vertaxID)
{
	int i;
	for (i = 0; i < maxVertices; i++)
	{
		if (Edge[vertaxID][i] == 1)
			return i;
	}
	return -1;
}

int Graphmtx::nextAdjacentVertax(int vertaxID, int nextVertaxID)
{
	int i;
	for (i = nextVertaxID + 1; i < maxVertices; i++)
	{
		if (Edge[vertaxID][i] == 1)
			return i;
	}
	return -1;
}

void Graphmtx::print_stack(SequenceStack stack)
{
	int temp = *(stack.base);
	while (stack.top != stack.base)
	{
		recordLoopVertex.insert(*(stack.base));
		(stack.base)++;
		//printf("%d->", *((stack.base)++));
	}
	//printf("%d\n", temp);
}

void Graphmtx::pop_stack(SequenceStack* stack, int* value)
{
	if (stack->stackSize == 0)
	{
		printf("stack is empty , can not to pop!\n");
		exit(0);
	}
	*value = *(--(stack->top));
}

void Graphmtx::clear_stack(SequenceStack* stack)
{
	stack->top = stack->base;
}

