#include "..\include\MeshProcessingCGAL.h"
#include <numeric>

#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/internal/clip.h>
#include <boost/function_output_iterator.hpp>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>

MeshProcessingCGAL::MeshProcessingCGAL() {
	m_dEdgeLength = 0.0;
}

MeshProcessingCGAL::MeshProcessingCGAL(std::string _filename) {
	m_dEdgeLength = 0.0;
	ReadMesh(_filename);
}

MeshProcessingCGAL::~MeshProcessingCGAL() {

}

//////////////////////////////////////////////////////////////////////////

bool MeshProcessingCGAL::ReadMesh(std::string _filename) {

	DMesh m_dmesh;

	int err = vcg::tri::io::Importer<DMesh >::Open(m_dmesh, _filename.c_str());
	if (vcg::tri::io::Importer<DMesh>::E_NOERROR != err)
	{
		std::cout << "failed to open file :  " << _filename << std::endl;
		std::cout << "error code is : " << vcg::tri::io::Importer<DMesh>::ErrorMsg(err) << std::endl;
		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	m_mesh.clear();

	for (auto vi = m_dmesh.vert.begin(); vi != m_dmesh.vert.end(); ++vi) {
		if (!(*vi).IsD())
		{
			m_mesh.add_vertex(Point(vi->P()[0], vi->P()[1], vi->P()[2]));
		}
	}
	std::vector<typename Mesh::Vertex_index> vertices;
	for (auto fi = m_dmesh.face.begin(); fi != m_dmesh.face.end(); ++fi)
	{
		if (!(*fi).IsD())
		{
			const int number_vertex_per_face = (*fi).VN();
			vertices.resize(number_vertex_per_face);
			for (int k = 0; k < (*fi).VN(); k++) {
				size_t idk =vcg::tri::Index(m_dmesh, (*fi).V(k));
				vertices[k] = Mesh::Vertex_index(idk);
			}
			m_mesh.add_face(vertices);
		}
	}
	
	m_dEdgeLength = 0.01*m_dmesh.bbox.Diag();
	std::vector<typename Mesh::Vertex_index>().swap(vertices);

	std::cout << "edge length is " << m_dEdgeLength << std::endl;
	return true;
}


void MeshProcessingCGAL::RefineMesh(std::string _outname) {

	double target_edge_length = m_dEdgeLength;
	unsigned int nb_iter = 3;

	std::cout << "Split border...";
	std::vector<edge_descriptor> border;
	CGAL::Polygon_mesh_processing::border_halfedges(faces(m_mesh), m_mesh, boost::make_function_output_iterator(halfedge2edge(m_mesh, border)));
	CGAL::Polygon_mesh_processing::split_long_edges(border, target_edge_length, m_mesh);
	std::cout << "done." << std::endl;

	std::cout << "Start remeshing of " << " (" << num_faces(m_mesh) << " faces)..." << std::endl;
	CGAL::Polygon_mesh_processing::isotropic_remeshing(
		faces(m_mesh),
		target_edge_length,
		m_mesh,
		CGAL::Polygon_mesh_processing::parameters::number_of_iterations(nb_iter)
		.protect_constraints(true)//i.e. protect border, here
	);
	std::cout << "Remeshing done." << std::endl;

	std::ofstream out(_outname);
	out << m_mesh;
	out.close();
}

void MeshProcessingCGAL::ClipMesh(std::string _outname, double _minx, double _miny, double _minz, double _maxx, double _maxy, double _maxz) {
	//////////////////////////////////////////////////////////////////////////
	Mesh boxmesh;
	Mesh::Vertex_index p0 = boxmesh.add_vertex(Point(_minx, _miny, _minz));
	Mesh::Vertex_index p1 = boxmesh.add_vertex(Point(_minx, _maxy, _minz));
	Mesh::Vertex_index p2 = boxmesh.add_vertex(Point(_maxx, _maxy, _minz));
	Mesh::Vertex_index p3 = boxmesh.add_vertex(Point(_maxx, _miny, _minz));
	Mesh::Vertex_index p4 = boxmesh.add_vertex(Point(_minx, _miny, _maxz));
	Mesh::Vertex_index p5 = boxmesh.add_vertex(Point(_minx, _maxy, _maxz));
	Mesh::Vertex_index p6 = boxmesh.add_vertex(Point(_maxx, _maxy, _maxz));
	Mesh::Vertex_index p7 = boxmesh.add_vertex(Point(_maxx, _miny, _maxz));

	boxmesh.add_face(p0, p1, p2);
	boxmesh.add_face(p0, p2, p3);
	boxmesh.add_face(p4, p6, p5);
	boxmesh.add_face(p4, p7, p6);
	boxmesh.add_face(p0, p7, p4);
	boxmesh.add_face(p0, p3, p7);
	boxmesh.add_face(p1, p5, p6);
	boxmesh.add_face(p1, p6, p2);
	boxmesh.add_face(p2, p6, p7);
	boxmesh.add_face(p2, p7, p3);
	boxmesh.add_face(p0, p5, p1);
	boxmesh.add_face(p0, p4, p5);

	//////////////////////////////////////////////////////////////////////////
	CGAL::Polygon_mesh_processing::clip(m_mesh, boxmesh, false, CGAL::Polygon_mesh_processing::parameters::all_default());

	std::ofstream out(_outname);
	out << m_mesh;
	out.close();

	//////////////////////////////////////////////////////////////////////////
}

void MeshProcessingCGAL::Simplification(std::string _outname, double ratio) {
	// In this example, the simplification stops when the number of undirected edges
	// drops below 10% of the initial count
	CGAL::Surface_mesh_simplification::Count_ratio_stop_predicate<Mesh> stop(ratio);

	int r = CGAL::Surface_mesh_simplification::edge_collapse(m_mesh, stop);
	std::cout << "\nFinished...\n" << r << " edges removed.\n"<< m_mesh.number_of_edges() << " final edges.\n";

	std::ofstream out(_outname);
	out << m_mesh;
	out.close();
}