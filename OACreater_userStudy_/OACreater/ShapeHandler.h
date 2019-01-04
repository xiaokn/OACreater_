#pragma once
#include <vector>
#include "direct.h"
#include "ViewMat.h"
#include "ShapePart.h"
struct ShapeRelation
{
	Edge edges[2]; //两条边为一条关系
	bool is2relations;//是否是2条以上关系
	RecordNo recordNo; //可视化的两个点
	double dis; //表示关系的好坏进行排序
};
struct SpacingRelation
{
	std::vector<RecordNo> recordNos; //可视化的两个点
	std::vector<cv::Vec3d> vecs;//向量

};
struct EditRelaion
{
	std::vector<ShapeRelation> m_recordRelations;//记录关系，用于可视化显示以及关系处理
	std::vector<ShapeRelation> m_optRelations;//可利用的关系，用于优化
	std::vector<ShapeRelation> m_addRelations;
	std::vector<ShapeRelation> m_deleteRelations;
};
struct SpacingRelaion
{
	std::vector<std::vector<RecordNo>> m_SpacingRelations;
	std::vector<ShapeRelation> m_addSpacingRelations;
	std::vector<std::vector<ShapeRelation>> m_addSpacingRelationsGroup;
	std::vector<RecordNo> m_deleteSpacingRelations;
};
class CShapeHandler
{
public:
	CShapeHandler();
	CShapeHandler(const CShapeHandler& shapeHandler);
	~CShapeHandler();
public:
	void DrawAllParts(QColor modelColor);
	void DrawAllParts();
	void DrawAllParts(qglviewer::Camera* c);
	void DrawAllPartsWithNames();
	void AddNewPart(CShapePart* newShapePart);
	void AddNewPart(CShapePart newShapePart);
	CShapePart* GetActivePart();
	bool SetActivePart(int idx);
	CShapeHandler& operator= (const CShapeHandler& shapeHandler);
public:
	void DestoryAllParts();
	void SaveWholeModel(const char* filePath,std::vector<bool> floatings);
	void ComputeEdgeAndNormal(qglviewer::Camera* c);
	void ComputeEdgeAndNormal(qglviewer::Camera* c, Edge e);
	void ComputeEdgeAndNormal(qglviewer::Camera* c, int shapeIndex);
	std::vector<std::vector<Edge>> GetEdgeRelationsWithRansac();
	void FixAllModels(qglviewer::Camera* c);
public:
	float ComputEdgeToLineDis(Edge line, Edge edge);
public:
	std::vector<CShapePart*> m_allParts;
	int m_activePartIdx;


};

