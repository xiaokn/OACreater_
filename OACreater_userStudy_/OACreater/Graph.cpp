#include "Graph.h"
//图的构造
Graphmtx::Graphmtx(int sz)   //构造函数  
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
	//检测环
	visitedFlag = new int[maxVertices];
	for (i = 0; i < maxVertices; i++)
	{
		visitedFlag[i] = 0;
	}
	loop_stack.base = new int[maxVertices];
	loop_stack.top = loop_stack.base;
	loop_stack.stackSize = maxVertices;
}
bool Graphmtx::insertVertex(const int& vertex) //插入一个顶点  
{
	if (numVertices == maxVertices)return false; //顶点表已满，返回false  
	VerticesList[numVertices++] = vertex;
	return true;
}
bool Graphmtx::insertEdge(int v1, int v2, int cost)//插入一条边  
{
	if (v1 > -1 && v1<numVertices && v2>-1 && v2 < numVertices) //检查条件  
	{
		if (Edge[v1][v2] == maxWeight)
		{
			Edge[v1][v2] = Edge[v2][v1] = cost;
			numEdges++;
			return true;
		}
		else
		{
			std::cout << "该边已存在，添加失败" << std::endl;
			return false;
		}
	}
	else return false;
}
void Graphmtx::inputGraph()
{
	//通过从输入流对象in输入n的顶点和e条五项边的信息建立邻接矩阵表示的图G。邻接矩阵初始化工作在构造函数完成  
	int i, j, k, m, n;
	int e1, e2;
	int weight;
	std::cout << "请输入顶点数和边数(空格隔开):" << std::endl;
	std::cin >> n >> m;  //输入点数n的边数m
	//
	maxVertices = n;
	std::cout << "请依次输入顶点:" << std::endl;
	for (i = 0; i < n; i++)//输入顶点，建立顶点表  
	{
		std::cin >> e1;
		this->insertVertex(e1);
		//G.insertVertex(e1);  
	}
	std::cout << "请依次输入边，形如 v1 v2 weight ：" << std::endl;
	i = 0;
	while (i < m)
	{
		std::cin >> e1 >> e2 >> weight;
		j = this->getVertexPos(e1);//查顶点号  
		k = this->getVertexPos(e2);
		if (j == -1 || k == -1)
		{
			std::cout << "边两端点信息有误，重新输入！" << std::endl;
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
	//输出图的所有顶点和边信息  
	int i, j, n, m;
	int e1, e2;
	int weight;
	n = this->NumberOfVertices();     //点数  
	m = this->NumberOfEdges();       //边数  
	std::cout << "顶点数的边数为：";
	std::cout << n << "," << m << std::endl;      //输出点数和边数  
	std::cout << "各边依次为：" << std::endl;
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
//检测环
void Graphmtx::checkRing()
{
	std::cout << "输出环\n";
}
//深度优先搜索检测环
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

