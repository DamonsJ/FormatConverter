#ifndef _MESHPROCESSINGCGAL_HEADER_
#define _MESHPROCESSINGCGAL_HEADER_

//////////////////////////////////////////////////////////////////////////
// cgal includes
#include "..\include\types_cgal.h"
#include "DMesh.h"

class MeshProcessingCGAL
{
public:
	MeshProcessingCGAL();
	MeshProcessingCGAL(std::string _filename);
	~MeshProcessingCGAL();

public:
	//************************************  
	// @brief : read triangle mesh from file .obj .ply .off .stl .3ds .vmi
	// @author: SunHongLei
	// @date  : 2019/04/10  
	// @return: void
	// @param : void  
	//************************************ 
	bool ReadMesh(std::string _filename);
public:
	//************************************  
	// @brief : refine current mesh 
	// @author: SunHongLei
	// @date  : 2019/04/10  
	// @return: void
	// @param : void  
	//************************************ 
	void RefineMesh(std::string _outname);
	//************************************  
	// @brief : clip mesh using box 
	// @author: SunHongLei
	// @date  : 2019/04/10  
	// @return: void
	// @param : void  
	//************************************ 
	void ClipMesh(std::string _outname,double _minx,double _miny,double _minz, double _maxx, double _maxy, double _maxz);
	//************************************  
	// @brief : simplification mesh using edge collapse 
	// @author: SunHongLei
	// @date  : 2019/04/10  
	// @return: void
	// @param : void  
	//************************************ 
	void Simplification(std::string _outname, double ratio = 0.1);

protected:
	Mesh m_mesh;
	double m_dEdgeLength;
};
#endif// 2019/04/10