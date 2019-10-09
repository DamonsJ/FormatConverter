/*!
 * \file DMesh.h
 * \date 2018/12/27 16:11
 *
 * \author damons

 * \brief a simple wrapper of vcg lib :
 * http://vcg.isti.cnr.it/vcglib/
 *
 * TODO: long description
 *
 * \note use for a basic mesh data structure
*/

#ifndef _DMESH_HEADER_
#define _DMESH_HEADER_

#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/point_sampling.h>
#include <vcg/complex/algorithms/create/resampler.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <vcg/complex/algorithms/create/ball_pivoting.h>
#include <vcg/complex/algorithms/refine_loop.h>

//#include <wrap/io_trimesh/import.h>
#include "..\include\import_base_file.h"
//#include <wrap/io_trimesh/export.h>

// pre-defines  data types
typedef float MESH_SCALAR;
typedef vcg::Point2<MESH_SCALAR>   Point2m;
typedef vcg::Point3<MESH_SCALAR>   Point3m;
typedef vcg::Point4<MESH_SCALAR>   Point4m;
typedef vcg::Plane3<MESH_SCALAR>   Plane3m;
typedef vcg::Segment2<MESH_SCALAR> Segment2m;
typedef vcg::Segment3<MESH_SCALAR> Segment3m;
typedef vcg::Box3<MESH_SCALAR>     Box3m;
typedef vcg::Matrix44<MESH_SCALAR> Matrix44m;
typedef vcg::Matrix33<MESH_SCALAR> Matrix33m;

namespace vcg
{
	namespace vertex
	{
		template <class T> class Coord3m : public Coord<vcg::Point3<MESH_SCALAR>, T> {
		public:	static void Name(std::vector<std::string> & name) { name.push_back(std::string("Coord3m")); T::Name(name); }
		};

		template <class T> class Normal3m : public Normal<vcg::Point3<MESH_SCALAR>, T> {
		public:	static void Name(std::vector<std::string> & name) { name.push_back(std::string("Normal3m")); T::Name(name); }
		};

		template <class T> class CurvatureDirmOcf : public CurvatureDirOcf<CurvatureDirTypeOcf<MESH_SCALAR>, T> {
		public:	static void Name(std::vector<std::string> & name) { name.push_back(std::string("CurvatureDirmOcf")); T::Name(name); }
		};

		template <class T> class RadiusmOcf : public RadiusOcf<MESH_SCALAR, T> {
		public:	static void Name(std::vector<std::string> & name) { name.push_back(std::string("RadiusmOcf")); T::Name(name); }
		};

	}//end namespace vertex
	namespace face
	{
		template <class T> class Normal3m : public NormalAbs<vcg::Point3<MESH_SCALAR>, T> {
		public:  static void Name(std::vector<std::string> & name) { name.push_back(std::string("Normal3m")); T::Name(name); }
		};

		template <class T> class CurvatureDirmOcf : public CurvatureDirOcf<CurvatureDirOcfBaseType<MESH_SCALAR>, T> {
		public:	static void Name(std::vector<std::string> & name) { name.push_back(std::string("CurvatureDirdOcf")); T::Name(name); }
		};

	}//end namespace face
}//end namespace vcg


// Forward declarations needed for creating the used types
class DVertex;
class DEdge;
class DFace;

// Declaration of the semantic of the used types
class DUsedTypes : public   vcg::UsedTypes<vcg::Use<DVertex>::AsVertexType,
							vcg::Use<DEdge>::AsEdgeType,
							vcg::Use<DFace>::AsFaceType> {};


// The Main Vertex Class
// Most of the attributes are optional and must be enabled before use.
// Each vertex needs 40 byte, on 32bit arch. and 44 byte on 64bit arch.

class DVertex : public vcg::Vertex< DUsedTypes,
	vcg::vertex::InfoOcf,           /*  4b */
	vcg::vertex::Coord3m,           /* 12b */
	vcg::vertex::BitFlags,          /*  4b */
	vcg::vertex::Normal3m,          /* 12b */
	vcg::vertex::Qualityf,          /*  4b */
	vcg::vertex::Color4b,           /*  4b */
	vcg::vertex::VFAdjOcf,          /*  0b */
	vcg::vertex::MarkOcf,           /*  0b */
	vcg::vertex::TexCoordfOcf,      /*  0b */
	vcg::vertex::CurvaturefOcf,     /*  0b */
	vcg::vertex::CurvatureDirmOcf,  /*  0b */
	vcg::vertex::RadiusmOcf         /*  0b */
> {
};

// The Main Edge Class
class DEdge : public vcg::Edge<DUsedTypes,
					 vcg::edge::BitFlags,          /*  4b */
					 vcg::edge::EVAdj,
					 vcg::edge::EEAdj
> {
};

// Each face needs 32 byte, on 32bit arch. and 48 byte on 64bit arch.
class DFace : public vcg::Face<  DUsedTypes,
	vcg::face::InfoOcf,              /* 4b */
	vcg::face::VertexRef,            /*12b */
	vcg::face::BitFlags,             /* 4b */
	vcg::face::Normal3m,             /*12b */
	vcg::face::QualityfOcf,          /* 0b */
	vcg::face::MarkOcf,              /* 0b */
	vcg::face::Color4bOcf,           /* 0b */
	vcg::face::FFAdjOcf,             /* 0b */
	vcg::face::VFAdjOcf,             /* 0b */
	vcg::face::CurvatureDirmOcf,     /* 0b */
	vcg::face::WedgeTexCoordfOcf     /* 0b */
> {};

class __declspec(dllexport) DMesh : public vcg::tri::TriMesh< vcg::vertex::vector_ocf<DVertex >, vcg::face::vector_ocf<DFace > >
{
public:

	Matrix44m Tr; // Usually it is the identity. It is applied in rendering and filters can or cannot use it. (most of the filter will ignore this)

	const Box3m &trBB()
	{
		static Box3m bb;
		bb.SetNull();
		bb.Add(Tr, bbox);
		return bb;
	}

	void printMeshInfo() {
		int number_facets = this->FN();
		int number_vertex = this->VN();
		int number_edges = this->EN();
		std::cout << "Mesh contains: " << std::endl;
		std::cout << "\t facets number: " << number_facets << std::endl;
		std::cout << "\t vertex number: " << number_vertex << std::endl;
		std::cout << "\t edges  number: " << number_edges << std::endl;
		std::cout << "\t bounding box : " << std::endl;
		std::cout << "\t " << this->bbox.min.X() << "\t " << this->bbox.min.Y() << "\t " << this->bbox.min.Z() << std::endl;
		std::cout << "\t " << this->bbox.max.X() << "\t " << this->bbox.max.Y() << "\t " << this->bbox.max.Z() << std::endl;
		
	}
};

#endif// 2018/04/12