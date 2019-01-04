#include "ViewMat.h"


CViewMat::CViewMat()
{

}


CViewMat::~CViewMat()
{
}

double& CViewMat::operator()(int i, int j)
{
	assert(i >= 0 && i < 4 && j >= 0 && j < 4);
	return m_m[i * 4 + j];
}

double& CViewMat::operator[](int i)
{
	assert(i >= 0 && i < 16);
	return m_m[i];
}

CViewMat& CViewMat::operator=(double m[16])
{
	for (int i = 0; i < 16; ++i)
	{
		m_m[i] = m[i];
	}
	qglviewer::ManipulatedFrame frame;
	frame.setFromMatrix(m);
	m_quaternion = frame.orientation();
	return *this;
}

CViewMat& CViewMat::operator=(double m[4][4])
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			m_m[i * 4 + j] = m[i][j];
		}
	}
	qglviewer::ManipulatedFrame frame;
	frame.setFromMatrix(m);
	m_quaternion = frame.orientation();
	return *this;
}

CViewMat& CViewMat::operator=(const qglviewer::Frame& frame)
{
	frame.getMatrix(m_m);
	m_quaternion = frame.orientation();
	return *this;
}

void CViewMat::GetRotationMat(float m[3][3]) const
{
	m_quaternion.getRotationMatrix(m);
}

void CViewMat::LoadView(const char* viewPath)
{
	std::fstream fin;
	fin.open(viewPath, std::ios::in);
	for (int i = 0; i < 16; ++i)
	{
		fin >> m_m[i];
	}
	qglviewer::ManipulatedFrame frame;
	frame.setFromMatrix(m_m);
	m_quaternion = frame.orientation();
	fin.flush();
	fin.close();
	fin.clear();
}

void CViewMat::SaveView(const char* viewPath)
{
	std::fstream fout;
	fout.open(viewPath, std::ios::out);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			fout << m_m[i * 4 + j] << " ";
		}
		fout << std::endl;
	}

	fout.flush();
	fout.close();
	fout.clear();
}

void CViewMat::GetViewMat(double m[16])
{
	for (int i = 0; i < 16; ++i)
	{
		m[i] = m_m[i];
	}
}

qglviewer::Quaternion CViewMat::Orientation()
{
	return m_quaternion;
}
