#ifndef _POINTCLOUDPROCESSCGAL_HEADER_
#define _POINTCLOUDPROCESSCGAL_HEADER_

//////////////////////////////////////////////////////////////////////////
// cgal includes
#include "..\include\types_cgal.h"
//////////////////////////////////////////////////////////////////////////
// c++ includes
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
//////////////////////////////////////////////////////////////////////////

struct Perimeter {
	double bound;
	Perimeter(double bound)
		: bound(bound)
	{}
	template <typename AdvancingFront, typename Cell_handle>
	double operator() (const AdvancingFront& adv, Cell_handle& c,
		const int& index) const
	{
		// bound == 0 is better than bound < infinity
		// as it avoids the distance computations
		if (bound == 0) {
			return adv.smallest_radius_delaunay_sphere(c, index);
		}
		// If perimeter > bound, return infinity so that facet is not used
		double d = 0;
		d = sqrt(squared_distance(c->vertex((index + 1) % 4)->point(),
			c->vertex((index + 2) % 4)->point()));
		if (d > bound) return adv.infinity();
		d += sqrt(squared_distance(c->vertex((index + 2) % 4)->point(),
			c->vertex((index + 3) % 4)->point()));
		if (d > bound) return adv.infinity();
		d += sqrt(squared_distance(c->vertex((index + 1) % 4)->point(),
			c->vertex((index + 3) % 4)->point()));
		if (d > bound) return adv.infinity();
		// Otherwise, return usual priority value: smallest radius of
		// delaunay sphere
		return adv.smallest_radius_delaunay_sphere(c, index);
	}
};

struct Construct {
	Mesh& mesh;
	template < typename PointIterator>
	Construct(Mesh& mesh, PointIterator b, PointIterator e)
		: mesh(mesh)
	{
		for (; b != e; ++b) {
			boost::graph_traits<Mesh>::vertex_descriptor v;
			v = add_vertex(mesh);
			mesh.point(v) = *b;
		}
	}
	Construct& operator=(const Facet f)
	{
		typedef boost::graph_traits<Mesh>::vertex_descriptor vertex_descriptor;
		typedef boost::graph_traits<Mesh>::vertices_size_type size_type;
		mesh.add_face(vertex_descriptor(static_cast<size_type>(f[0])),
			vertex_descriptor(static_cast<size_type>(f[1])),
			vertex_descriptor(static_cast<size_type>(f[2])));
		return *this;
	}
	Construct&
		operator*() { return *this; }
	Construct&
		operator++() { return *this; }
	Construct
		operator++(int) { return *this; }
};

//////////////////////////////////////////////////////////////////////////
/*!
 * \class PointCloudReconstructCGAL
 *
 * \brief point cloud reconstruction using cgal
 *  ref:
 *	https://doc.cgal.org/latest/Poisson_surface_reconstruction_3/
 *  point cloud processing:
 *   analysis ->outlier removal -> simplification-> smoothing -> normal estimation -> normal orientation
 *   -> reconstruction
 * \author Damons
 * \date ËÄÔÂ 2019
 */
class PointCloudReconstructCGAL
{
public:
	PointCloudReconstructCGAL() {}
	~PointCloudReconstructCGAL(){
		PointList().swap(m_ptSet);
	}

//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : read point cloud from file,judge from file name extension
	//			supported: .xyz .off .ply .las
	//			this api can read point cloud with normals
	// @author: SunHongLei
	// @date  : 2018/04/03  
	// @return: void
	// @param : std::string _fileName  :file name
	//************************************ 
	bool ReadPointCloudFromFile(std::string _fileName);

protected:
	// read xyz file
	bool ReadXYZFile(std::string _fileName, bool isbianary = false);
	// read off file
	bool ReadOFFFile(std::string _fileName);
	// read ply file
	bool ReadPLYFile(std::string _fileName, bool isbianary = false);
	// read las file need las dll
	//bool ReadLASFile(std::string _fileName, bool isbianary = false);
//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : write current point cloud to file after some operation
	//			write only points to file as .xyz .off .ply
	// @author: SunHongLei
	// @date  : 2018/04/03  
	// @return: void
	// @param : void  
	//************************************ 
	void WritePointCloudToFile(std::string _fileName, bool isbianary = false);
	//************************************  
	// @brief : write constructed mesh to file  
	// @author: SunHongLei
	// @date  : 2018/05/07  
	// @return: void
	// @param : void  
	//************************************ 
	void WriteConstructedMesh(std::string _fileName);
protected:
	// Write xyz file
	bool WriteXYZFile(std::string _fileName, bool isbianary = false);
	// Write off file
	bool WriteOFFFile(std::string _fileName);
	// write ply file
	bool WritePLYFile(std::string _fileName, bool isbianary = false);
//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : analsis point cloud include neighbor scale and averaging spacing
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : double neighbor_times : enlarge neighbor size by neighbor_times times
	//			it means: neighbor_size = neighbor_times*neighbor_size
	//************************************ 
	void AnalysisPointCloud(double neighbor_times = 1);

//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : remove outliers points with input removed_percentage 
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : double removed_percentage : percentage of points to remove
	//************************************ 
	void RemoveOutlierPointsPercentage(double removed_percentage);
	//************************************  
	// @brief : remove outliers points with distance
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : double average_times:   Point with distance above average_times*average_spacing are considered outliers
	// @param : double limits_number_percentage:  limit on the number of outliers to remove
	//************************************ 
	void RemoveOutlierPointsDistance(double average_times = 2.0,double limits_number_percentage = 100.0);
//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : simplify point cloud using random methods
	// random_simplify_point_set()
	// randomly delete a user-specified fraction of points
	// from the input point set. This algorithm is fast.
	// @author: SunHongLei
	// @date  : 2018/04/03  
	// @return: void
	// @see : https://doc.cgal.org/latest/Point_set_processing_3/index.html#Chapter_Point_Set_Processing
	//************************************ 
	void Simplification_Random(double percentage);
	//************************************  
	// @brief : simplify point cloud using grid methods 
	// Merges points which belong to the same cell of a grid of cell size = epsilon.
	// This method modifies the order of input points so as to pack all remaining points first, 
	// and returns an iterator over the first point to remove(see erase - remove idiom).
	// For this reason it should not be called on sorted containers.
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : double tolerance: tolerance value when merging 3D points  
	// @see : https://doc.cgal.org/latest/Point_set_processing_3/index.html#Chapter_Point_Set_Processing
	//************************************ 
	void Simplification_Grid(double tolerance);
	//************************************  
	// @brief : simplify point cloud using Hierarchy methods 
	// provides an adaptive simplification of the point set through local clusters. 
	// The size of the clusters is either directly selected by the user or it
	// automatically adapts to the local variation of the point set
	// Recursively split the point set in smaller clusters until the clusters have less than size elements 
	// or until their variation factor is below var_max.
	// This method modifies the order of input points so as to pack all remaining points first,
	// and returns an iterator over the first point to remove(see erase - remove idiom).
	// For this reason it should not be called on sorted containers.
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : void  
	// @see : https://doc.cgal.org/latest/Point_set_processing_3/index.html#Chapter_Point_Set_Processing
	//************************************ 
	void Simplification_Hierarchy(unsigned int point_size = 100, double varation = 0.3);
	//************************************  
	// @brief : This is an implementation of the Weighted Locally Optimal Projection (WLOP) simplification algorithm. 
	// The WLOP simplification algorithm can produce a set of denoised, outlier - free and evenly distributed 
	// particles over the original dense point cloud.
	// The core of the algorithm is a Weighted Locally Optimal Projection operator with a density uniformization term.
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param :select_percentage percentage of points to retain.The default value is set to 5 (%).
	// @param :radius spherical neighborhood radius.This is a key parameter that needs to be finely tuned.
	//		   The result will be irregular if too small, but a larger value will impact the runtime.
	//		   In practice, choosing a radius such that the neighborhood of each sample point includes 
	//		   at least two rings of neighboring sample points gives satisfactory result.
	//			The default value is set to 8 times the average spacing of the point set.
	// @param :iter_number	number of iterations to solve the optimsation problem.
	//			The default value is 35. More iterations give a more regular result but increase the runtime.
	// @param :require_uniform_sampling an optional preprocessing, 
	//		   which will give better result if the distribution of the input points is highly non - uniform.The default value is false.
	// @see : https://doc.cgal.org/latest/Point_set_processing_3/index.html#Chapter_Point_Set_Processing
	//************************************ 
	void Simplification_WLOP(double select_percentage = 5, double radius = 0.0, double iter_number = 35, bool require_uniform_sampling = false);
//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : estimates the normal direction at each point from the input set by fitting a jet surface over its k nearest neighbors. 
	//			The default jet is a quadric surface. 
	//			This algorithm is well suited to point sets scattered over curved surfaces. 
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : void  
	//************************************ 
	void NormalEstimate_JET();
	//************************************  
	// @brief : estimates the normal direction at each point from the set by linear least squares fitting of a plane over its k nearest neighbors.
	//			This algorithm is simpler and faster than jet_estimate_normals().  
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : void  
	//************************************ 
	void NormalEstimate_PCA();
	//************************************  
	// @brief : estimates the normal direction at each point from the set by using the Voronoi Covariance Measure of the point set. 
	//			This algorithm is more complex and slower than the previous algorithms. 
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : void 
	// @see   : Quentin Merigot, Maks Ovsjanikov, and Leonidas Guibas. Voronoi-based curvature and feature estimation from point clouds. 
	//			Visualization and Computer Graphics, IEEE Transactions on, 17(6):743¨C756, 2011.
	//************************************ 
	void NormalEstimate_VCM(double offset_radius = 0.0);
	//************************************  
	// @brief : Orients the normals of the [first, beyond) range of points using the propagation of a seed orientation through a minimum
	//			spanning tree of the Riemannian graph 
	//			This method modifies the order of input points so as to pack all sucessfully oriented points first, 
	//			and returns an iterator over the first point with an unoriented normal (see erase-remove idiom). 
	//			For this reason it should not be called on sorted containers.
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : bool _bIsEraseUnOriented: if true un oriented normals will be erased  
	//************************************ 
	void NormalOrientation(bool _bIsEraseUnOriented = false);

//////////////////////////////////////////////////////////////////////////
public:
	//************************************  
	// @brief : Performs surface reconstruction as follows:
	//			compute the Poisson implicit function, through a conjugate gradient solver, represented as a piecewise linear function stored on a 3D Delaunay mesh generated via Delaunay refinement
	//			meshes the function with a user - defined precision using another round of Delaunay refinement : it contours the isosurface corresponding to the isovalue of the median of the function values at the input points
	//			outputs the result in a polygon mesh
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : sm_angle	bound for the minimum facet angle in degrees.
	// @param : sm_radius	bound for the radius of the surface Delaunay balls(relatively to the average_spacing).
	// @param : sm_distance	bound for the center-center distances (relatively to the average_spacing).
	//************************************ 
	void PoissonSurfaceReconstrutionDelaunay(double sm_angle = 20.0, double sm_radius = 30.0, double sm_distance = 0.375);
	void PoissonSurfaceReconstrutionDetail(double sm_angle = 20.0, double sm_radius = 30.0, double sm_distance = 0.375);
	//************************************  
	// @brief : Surface reconstruction from an unstructured point cloud amounts to generate 
	//			a plausible surface that approximates well the input points 
	// @author: SunHongLei
	// @date  : 2019/04/03  
	// @return: void
	// @param : radius_ratio_bound	candidates incident to surface triangles which are not in the beta-wedge are discarded, 
	//			if the ratio of their radius and the radius of the surface triangle is larger than radius_ratio_bound. 
	//			Described in Section Dealing with Multiple Components, Boundaries and Sharp Edges  
	//@param : beta	half the angle of the wedge in which only the radius of triangles counts for the plausibility of candidates
	//************************************ 
	void ReconstrutUsingAFT(std::string offfilename, double radius_ratio_bound = 5.0, double beta = 0.52);
protected:
	//************************************  
	// @brief :  Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them and count normals with large deviation.
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: true on success.
	// @param : void  
	//************************************ 
	bool verify_normal_direction(const PointList& points, // input points + computed normals
								 const std::vector<Vector>& original_normals); // may be empty
	//************************************  
	// @brief : Check the accuracy of normal orientation.
	//			Count non-oriented normals.
	//			If original normals are available, compare with them and count flipped normals.  
	// @author: SunHongLei
	// @date  : 2019/04/02  
	// @return: void
	// @param : void  
	//************************************ 
	bool verify_normal_orientation(const PointList& points, // input points + computed normals
									PointList::const_iterator unoriented_points_begin, // first pt unoriented normal
									const std::vector<Vector>& original_normals, 
									bool _bIsEraseUnOriented = true);

//////////////////////////////////////////////////////////////////////////
public:
	double GetAveragingValue() const {
		return m_dAvgSpacing;
	}
	int GetPointSetSize() const {
		return m_ptSet.size();
	}
	int GetNeighborSize() const {
		return m_nKSize;
	}
	//************************************  
	// @brief : get point set bounding box 
	// @author: SunHongLei
	// @date  : 2018/05/07  
	// @return: void
	// @param : void  
	//************************************ '
	CGAL::Bbox_3 ComputeBoundingBox();

protected:
	PointList m_ptSet;// point cloud container
protected:
	unsigned int m_nKSize;// estimated global k-scale
	double m_dAvgSpacing;// point cloud average spacing
protected:
	Polyhedron output_mesh;// result mesh
};
#endif// 2019/04/02