#ifndef _TYPES_CGAL_HEADER_
#define _TYPES_CGAL_HEADER_

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Poisson_reconstruction_function.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Timer.h>
#include <CGAL/bounding_box.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/array.h>

//////////////////////////////////////////////////////////////////////////
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 Point;
typedef Kernel::Vector_3 Vector;
// Point with normal vector stored in a std::pair.
typedef std::pair<Point, Vector> PointVectorPair;
typedef std::vector<PointVectorPair > PointList;

typedef CGAL::cpp11::array<std::size_t, 3> Facet;
typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Surface_mesh_default_triangulation_3 STr;
typedef CGAL::Surface_mesh_complex_2_in_triangulation_3<STr> C2t3;
typedef CGAL::Implicit_surface_3<Kernel, Poisson_reconstruction_function> Surface_3;
typedef Kernel::Sphere_3 Sphere;
typedef CGAL::Surface_mesh<Point > Mesh;
typedef boost::graph_traits<Mesh>::halfedge_descriptor halfedge_descriptor;
typedef boost::graph_traits<Mesh>::edge_descriptor     edge_descriptor;

#ifdef CGAL_LINKED_WITH_TBB
typedef CGAL::Parallel_tag Concurrency_tag;
#else
typedef CGAL::Sequential_tag Concurrency_tag;
#endif

struct halfedge2edge
{
	halfedge2edge(const Mesh& m, std::vector<edge_descriptor>& edges)
		: m_mesh(m), m_edges(edges)
	{}
	void operator()(const halfedge_descriptor& h) const
	{
		m_edges.push_back(edge(h, m_mesh));
	}
	const Mesh& m_mesh;
	std::vector<edge_descriptor>& m_edges;
};

//////////////////////////////////////////////////////////////////////////
#ifndef PRINT_DETAIL
#define PRINT_DETAIL
#endif

//////////////////////////////////////////////////////////////////////////

#endif// 2018/05/08
