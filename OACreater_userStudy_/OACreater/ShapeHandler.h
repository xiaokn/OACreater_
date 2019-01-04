#pragma once
#include <vector>
#include "direct.h"
#include "ViewMat.h"
#include "ShapePart.h"
struct ShapeRelation
{
	Edge edges[2]; //������Ϊһ����ϵ
	bool is2relations;//�Ƿ���2�����Ϲ�ϵ
	RecordNo recordNo; //���ӻ���������
	double dis; //��ʾ��ϵ�ĺû���������
};
struct SpacingRelation
{
	std::vector<RecordNo> recordNos; //���ӻ���������
	std::vector<cv::Vec3d> vecs;//����

};
struct EditRelaion
{
	std::vector<ShapeRelation> m_recordRelations;//��¼��ϵ�����ڿ��ӻ���ʾ�Լ���ϵ����
	std::vector<ShapeRelation> m_optRelations;//�����õĹ�ϵ�������Ż�
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

