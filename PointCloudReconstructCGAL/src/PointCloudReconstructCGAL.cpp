#include "..\include\PointCloudReconstructCGAL.h"
//////////////////////////////////////////////////////////////////////////
#include <CGAL/estimate_scale.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/IO/read_off_points.h>
#include <CGAL/IO/write_off_points.h>
#include <CGAL/IO/read_ply_points.h>
#include <CGAL/IO/write_ply_points.h>
//#include <CGAL/IO/read_las_points.h>
//#include <CGAL/IO/write_las_points.h>

#include <CGAL/compute_average_spacing.h>
#include <CGAL/remove_outliers.h>
#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/random_simplify_point_set.h>
#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>
#include <CGAL/jet_smooth_point_set.h>
#include <CGAL/bilateral_smooth_point_set.h>
#include <CGAL/mst_orient_normals.h>
#include <CGAL/jet_estimate_normals.h>
#include <CGAL/pca_estimate_normals.h>
#include <CGAL/vcm_estimate_normals.h>
#include <CGAL/edge_aware_upsample_point_set.h>

#include <CGAL/IO/output_surface_facets_to_polyhedron.h>
#include <CGAL/poisson_surface_reconstruction.h>

#include <boost/function_output_iterator.hpp>

//////////////////////////////////////////////////////////////////////////
bool PointCloudReconstructCGAL::ReadPointCloudFromFile(std::string _fileName)
{
	std::size_t pos = _fileName.find_last_of(".");
	if (std::string::npos == pos)
		return false;
	//////////////////////////////////////////////////////////////////////////
	// check file contains binary
	bool isbianary = false;
	int c;
	std::ifstream a(_fileName);
	while ((c = a.get()) != EOF ) {
		if (c > 127)
		{
			isbianary = true;
			break;
		}
	}
	a.close();
	/*
	check file is all ascii
	int c;
	std::ifstream a(_fileName);
	while((c = a.get()) != EOF && c <= 127)
	;
	if(c == EOF) {
	 //file is all ASCII 
	}
	a.close();
	*/
	//////////////////////////////////////////////////////////////////////////
	std::cout << " reading point cloud ......" << std::endl;
	std::string ext = _fileName.substr(pos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == "xyz") {
		return ReadXYZFile(_fileName, isbianary);
	}
	else if (ext == "off") {
		return ReadOFFFile(_fileName);
	}
	else if (ext == "ply") {
		return ReadPLYFile(_fileName, isbianary);
	}
	//else if (ext == "las") {
	//	return ReadLASFile(_fileName, isbianary);
	//}

	return true;
}

bool PointCloudReconstructCGAL::ReadXYZFile(std::string _fileName, bool isbianary/* = false*/) {

	std::ios::openmode opm = std::ios::in;
	if (isbianary)
		opm = std::ios::in | std::ios::binary;
	std::ifstream in(_fileName, opm);

	if (!in || !CGAL::read_xyz_points_and_normals(in, std::back_inserter(m_ptSet),
		CGAL::First_of_pair_property_map<PointVectorPair>(), CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot read file " << _fileName << std::endl;
		in.close();
		return false;
	}

	in.close();
	return true;
}


bool PointCloudReconstructCGAL::ReadOFFFile(std::string _fileName) {
	std::ifstream in(_fileName);
	if (!in || !CGAL::read_off_points_and_normals(in, std::back_inserter(m_ptSet),
		CGAL::First_of_pair_property_map<PointVectorPair>(), CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot read file " << _fileName << std::endl;
		in.close();
		return false;
	}
	in.close();
	return true;
}

bool PointCloudReconstructCGAL::ReadPLYFile(std::string _fileName, bool isbianary/* = false*/) {
	std::ios::openmode opm = std::ios::in;
	if (isbianary)
		opm = std::ios::in | std::ios::binary;
	std::ifstream in(_fileName, opm);

	if (!in || !CGAL::read_ply_points_and_normals(in, std::back_inserter(m_ptSet),
		CGAL::First_of_pair_property_map<PointVectorPair>(), CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot read file " << _fileName << std::endl;
		in.close();
		return false;
	}
	in.close();

	return true;
}
/*
need las dll
bool PointCloudReconstructCGAL::ReadLASFile(std::string _fileName) {

	std::ios::openmode opm = std::ios::in;
	std::ifstream in(_fileName, opm);
	if (!in ||!CGAL::read_las_points_with_properties
		(in,
			std::back_inserter(m_ptSet),
			CGAL::make_las_point_reader(CGAL::First_of_pair_property_map<PointVectorPair>()),
			std::make_tuple
			(CGAL::Second_of_pair_property_map<PointVectorPair>(),
				CGAL::Construct_array(),
				CGAL::LAS_property::X(),
				CGAL::LAS_property::Y(),
				CGAL::LAS_property::Z())))
	{
		std::cout << "Error: cannot read file " << _fileName << std::endl;
		in.close();
		return false;
	}

	in.close();

	return true;
}*/
//////////////////////////////////////////////////////////////////////////
void PointCloudReconstructCGAL::WritePointCloudToFile(std::string _fileName, bool isbianary /*= false*/) {
	std::size_t pos = _fileName.find_last_of(".");
	if (std::string::npos == pos)
		return;

	std::string ext = _fileName.substr(pos + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == "xyz")
		WriteXYZFile(_fileName, isbianary);
	else if (ext == "off")
		WriteOFFFile(_fileName);
	else if (ext == "ply")
		WritePLYFile(_fileName, isbianary);
}

void PointCloudReconstructCGAL::WriteConstructedMesh(std::string _fileName) {
	std::ofstream out(_fileName);
	out << output_mesh;
	out.close();
}
bool PointCloudReconstructCGAL::WriteXYZFile(std::string _fileName, bool isbianary /*= false*/) {
	std::ios::openmode opm = std::ios::out;
	if (isbianary)
		opm = std::ios::out | std::ios::binary;
	std::ofstream out(_fileName, opm);

	out.setf(std::ios::fixed, std::ios::floatfield);
	if (!out || !CGAL::write_xyz_points_and_normals(out, m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot write file " << _fileName << std::endl;
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool PointCloudReconstructCGAL::WriteOFFFile(std::string _fileName) {
	std::ofstream out(_fileName);
	if (!out || !CGAL::write_off_points_and_normals(out, m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot write file " << _fileName << std::endl;
		out.close();
		return false;
	}
	out.close();
	return true;
}

bool PointCloudReconstructCGAL::WritePLYFile(std::string _fileName, bool isbianary /*= false*/) {
	std::ios::openmode opm = std::ios::out;
	if (isbianary)
		opm = std::ios::out | std::ios::binary;
	std::ofstream out(_fileName, opm);
	out.setf(std::ios::fixed, std::ios::floatfield);

	if (!out || !CGAL::write_ply_points_and_normals(out, m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>()))
	{
		std::cout << "Error: cannot write file " << _fileName << std::endl;
		out.close();
		return false;
	}
	out.close();
	return true;
}


//////////////////////////////////////////////////////////////////////////
void PointCloudReconstructCGAL::AnalysisPointCloud(double neighbor_times /*= 1*/) {

#ifdef PRINT_DETAIL
	std::cout << " analysis point cloud ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//1. k-scale calculating
	m_nKSize = neighbor_times * CGAL::estimate_global_k_neighbor_scale(m_ptSet.begin(), m_ptSet.end(), CGAL::First_of_pair_property_map<PointVectorPair>());

	//2. Estimate scale of the point set with average spacing
	m_dAvgSpacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>
		(m_ptSet.begin(), m_ptSet.end(), CGAL::First_of_pair_property_map<PointVectorPair>(), m_nKSize);

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();

	std::cout << " analysis point cloud with "<<m_ptSet.size()<<" points using "<< time <<"s" << std::endl;
	std::cout << " neighbor size is : "<<m_nKSize << std::endl;
	std::cout << " average spacing is : " << m_dAvgSpacing << std::endl;
#endif
}

CGAL::Bbox_3 PointCloudReconstructCGAL::ComputeBoundingBox() {

	CGAL::First_of_pair_property_map<PointVectorPair> m_point_map = CGAL::First_of_pair_property_map<PointVectorPair>();
	Kernel::Iso_cuboid_3 boundbox = CGAL::bounding_box(boost::make_transform_iterator(m_ptSet.begin(), CGAL::Property_map_to_unary_function<CGAL::First_of_pair_property_map<PointVectorPair> >(m_point_map)),
		boost::make_transform_iterator(m_ptSet.end(), CGAL::Property_map_to_unary_function<CGAL::First_of_pair_property_map<PointVectorPair> >(m_point_map)));

	return boundbox.bbox();
}

//////////////////////////////////////////////////////////////////////////

void PointCloudReconstructCGAL::RemoveOutlierPointsPercentage(double removed_percentage) {

#ifdef PRINT_DETAIL
	std::cout << " remove outliers points ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	int before_size = m_ptSet.size();
	m_ptSet.erase(CGAL::remove_outliers(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		m_nKSize,
		removed_percentage, // Minimum percentage to remove
		0.), // No distance threshold (can be omitted)
		m_ptSet.end());
	int after_size = m_ptSet.size();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();

	std::cout << "  remove outliers points  from " << before_size << " to " << after_size << "with time : "<<time << "s" << std::endl;
#endif
}
void PointCloudReconstructCGAL::RemoveOutlierPointsDistance(double average_times/* = 2.0*/, double limits_number_percentage/* = 100.0*/){

#ifdef PRINT_DETAIL
	std::cout << " remove outliers points ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif
	int before_size = m_ptSet.size();

	PointList::iterator first_to_remove
		= CGAL::remove_outliers(m_ptSet.begin(), m_ptSet.end(),
			CGAL::First_of_pair_property_map<PointVectorPair>(),
			m_nKSize,
			limits_number_percentage,                  // No limit on the number of outliers to remove
			average_times * m_dAvgSpacing); // Point with distance above 2*average_spacing are considered outliers

	m_ptSet.erase(first_to_remove, m_ptSet.end());

	int after_size = m_ptSet.size();
#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();

	std::cout << "  remove outliers points  from " << before_size << " to " << after_size << "with time : " << time << "s" << std::endl;
#endif
}
//////////////////////////////////////////////////////////////////////////

void PointCloudReconstructCGAL::Simplification_Random(double percentage) {

#ifdef PRINT_DETAIL
	std::cout << " processing random Simplification ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif
	int before_size = m_ptSet.size();
	m_ptSet.erase(CGAL::random_simplify_point_set(m_ptSet.begin(), m_ptSet.end(), CGAL::First_of_pair_property_map<PointVectorPair>(), percentage), m_ptSet.end());
	int after_size = m_ptSet.size();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " before : " << before_size << " after : " << after_size << " after Simplification" << std::endl;
	std::cout << " function: Random Simplification eclipse " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::Simplification_Grid(double tolerance) {
#ifdef PRINT_DETAIL
	std::cout << " processing grid Simplification ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	int before_size = m_ptSet.size();
	m_ptSet.erase(CGAL::grid_simplify_point_set(m_ptSet.begin(), m_ptSet.end(), CGAL::First_of_pair_property_map<PointVectorPair>(), tolerance), m_ptSet.end());
	int after_size = m_ptSet.size();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " before : " << before_size << " after : " << after_size << " after Simplification" << std::endl;
	std::cout << " function: Grid Simplification eclipse " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::Simplification_Hierarchy(unsigned int point_size/* = 100*/, double varation /*= 0.3*/) {
#ifdef PRINT_DETAIL
	std::cout << " processing hierarchy Simplification ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	int before_size = m_ptSet.size();
	m_ptSet.erase(CGAL::hierarchy_simplify_point_set(m_ptSet.begin(), m_ptSet.end(), CGAL::First_of_pair_property_map<PointVectorPair>(), point_size, varation), m_ptSet.end());
	int after_size = m_ptSet.size();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " before : " << before_size << " after : " << after_size << " after Simplification" << std::endl;
	std::cout << " function: hierarchy Simplification eclipse " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::Simplification_WLOP(double select_percentage /*= 5*/, double radius /*= 0.0*/, double iter_number /*= 35*/, bool require_uniform_sampling /*= false*/) {
#ifdef PRINT_DETAIL
	std::cout << " processing WLOP Simplification ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	if (radius == 0.0)
		radius = 8.0*m_dAvgSpacing;

	int before_size = m_ptSet.size();
	std::vector< Point > outputpl;
	CGAL::wlop_simplify_and_regularize_point_set<CGAL::Sequential_tag >(m_ptSet.begin(),
		m_ptSet.end(),
		std::back_inserter(outputpl),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		select_percentage,
		radius, iter_number, require_uniform_sampling
		);

	m_ptSet.resize(outputpl.size());
	for (int i = 0; i < m_ptSet.size(); ++i)
	{
		m_ptSet[i].first = outputpl[i];
	}

	std::vector< Point >().swap(outputpl);
	outputpl.shrink_to_fit();

	int after_size = m_ptSet.size();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " before : " << before_size << " after : " << after_size << " after Simplification" << std::endl;
	std::cout << " function: WLOP Simplification eclipse " << time << " second(s)" << std::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////

void PointCloudReconstructCGAL::NormalEstimate_JET() {
#ifdef PRINT_DETAIL
	std::cout << " normal estimation using jet ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//***************************************
	// Copy original normals
	//***************************************
	std::vector<Vector> original_normals;
	Vector her = m_ptSet.begin()->second;
	bool points_have_original_normals = (her != CGAL::NULL_VECTOR);
	if (points_have_original_normals)
	{
		std::cout << " have original normals --->" << std::endl;
		original_normals.resize(m_ptSet.size());
		int id = 0;
		for (PointList::iterator p = m_ptSet.begin(); p != m_ptSet.end(); p++)
			original_normals[id++] = p->second;
	}

	//***************************************
	// Computes normals 
	//***************************************
#ifdef PRINT_DETAIL
	std::cout << "Estimates Normals Direction by Jet Fitting (k=" << m_nKSize << ")...\n";
#endif

	CGAL::jet_estimate_normals<Concurrency_tag>(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>(),
		m_nKSize);

#ifdef PRINT_DETAIL
	std::size_t memory = CGAL::Memory_sizer().virtual_size();
	std::cout << "done: " << task_timer.time() << " seconds, " << (memory >> 20) << " Mb allocated" << std::endl;
#endif
	//***************************************
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	//***************************************
	bool issuccess = verify_normal_direction(m_ptSet, original_normals);
	std::vector<Vector>().swap(original_normals);
	original_normals.shrink_to_fit();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " NormalEstimate using JET with normal checking eclipse : " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::NormalEstimate_PCA() {
#ifdef PRINT_DETAIL
	std::cout << " normal estimation using pca ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//***************************************
	// Copy original normals
	//***************************************
	std::vector<Vector> original_normals;
	Vector her = m_ptSet.begin()->second;
	bool points_have_original_normals = (her != CGAL::NULL_VECTOR);
	if (points_have_original_normals)
	{
		std::cout << " have original normals --->" << std::endl;
		original_normals.resize(m_ptSet.size());
		int id = 0;
		for (PointList::iterator p = m_ptSet.begin(); p != m_ptSet.end(); p++)
			original_normals[id++] = p->second;
	}

	//***************************************
	// Computes normals 
	//***************************************
#ifdef PRINT_DETAIL
	std::cout << "Estimates Normals Direction by PCA (k=" << m_nKSize << ")...\n";
#endif

	CGAL::pca_estimate_normals<Concurrency_tag>(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>(),
		m_nKSize);

#ifdef PRINT_DETAIL
	std::size_t memory = CGAL::Memory_sizer().virtual_size();
	std::cout << "done: " << task_timer.time() << " seconds, " << (memory >> 20) << " Mb allocated" << std::endl;
#endif
	//***************************************
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	//***************************************
	bool issuccess = verify_normal_direction(m_ptSet, original_normals);
	std::vector<Vector>().swap(original_normals);
	original_normals.shrink_to_fit();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " NormalEstimate using PCA with normal checking eclipse : " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::NormalEstimate_VCM(double offset_radius) {

#ifdef PRINT_DETAIL
	std::cout << " normal estimation using vcm ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//***************************************
	// Copy original normals
	//***************************************
	std::vector<Vector> original_normals;
	Vector her = m_ptSet.begin()->second;
	bool points_have_original_normals = (her != CGAL::NULL_VECTOR);
	if (points_have_original_normals)
	{
		std::cout << " have original normals --->" << std::endl;
		original_normals.resize(m_ptSet.size());
		int id = 0;
		for (PointList::iterator p = m_ptSet.begin(); p != m_ptSet.end(); p++)
			original_normals[id++] = p->second;
	}

	//***************************************
	// Computes normals 
	//***************************************
#ifdef PRINT_DETAIL
	std::cout << "Estimates Normals Direction by VCM (k=" << m_nKSize << ")...\n";
#endif
	if (offset_radius == 0.0)
		offset_radius = 2.0*m_dAvgSpacing;

	CGAL::vcm_estimate_normals(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>(),
		offset_radius,
		m_nKSize);

#ifdef PRINT_DETAIL
	std::size_t memory = CGAL::Memory_sizer().virtual_size();
	std::cout << "done: " << task_timer.time() << " seconds, " << (memory >> 20) << " Mb allocated" << std::endl;
#endif
	//***************************************
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	// Check the accuracy of normals direction estimation.
	// If original normals are available, compare with them.
	//***************************************
	bool issuccess = verify_normal_direction(m_ptSet, original_normals);
	std::vector<Vector>().swap(original_normals);
	original_normals.shrink_to_fit();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " NormalEstimate using vcm with normal checking eclipse : " << time << " second(s)" << std::endl;
#endif

}

void PointCloudReconstructCGAL::NormalOrientation(bool _bIsEraseUnOriented) {
#ifdef PRINT_DETAIL
	std::cout << "Orients Normals with a Minimum Spanning Tree (k=" << m_nKSize << ")...\n";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//***************************************
	// Copy original normals
	//***************************************
	std::vector<Vector> original_normals;
	Vector her = m_ptSet.begin()->second;
	bool points_have_original_normals = (her != CGAL::NULL_VECTOR);
	if (points_have_original_normals)
	{
		std::cout << " have original normals --->" << std::endl;
		original_normals.resize(m_ptSet.size());
		int id = 0;
		for (PointList::iterator p = m_ptSet.begin(); p != m_ptSet.end(); p++)
			original_normals[id++] = p->second;
	}

	//***************************************
	// orient normals 
	//***************************************
	PointList::iterator unoriented_points_begin =
		CGAL::mst_orient_normals(m_ptSet.begin(), m_ptSet.end(),
			CGAL::First_of_pair_property_map<PointVectorPair>(),
			CGAL::Second_of_pair_property_map<PointVectorPair>(),
			m_nKSize);

#ifdef PRINT_DETAIL
	std::size_t memory = CGAL::Memory_sizer().virtual_size();
	std::cout << "done: " << task_timer.time() << " seconds, " << (memory >> 20) << " Mb allocated" << std::endl;
#endif

	// Note: we do *not* delete points with unoriented normals in this test.
	// Instead, we check the accuracy of normal orientation and,
	// if original normals are available, compare with them.
	bool issuccess = verify_normal_orientation(m_ptSet, unoriented_points_begin, original_normals, _bIsEraseUnOriented);

	std::vector<Vector>().swap(original_normals);
	original_normals.shrink_to_fit();

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " Normal orientation using mst with normal checking eclipse : " << time << " second(s)" << std::endl;
#endif
}

bool PointCloudReconstructCGAL::verify_normal_orientation(const PointList& points, // input points + computed normals
															PointList::const_iterator unoriented_points_begin, // first pt w/ unoriented normal
															const std::vector<Vector>& original_normals,
															bool _bIsEraseUnOriented) // may be empty
{
	bool success = true;

	// Count non-oriented normals
	int unoriented_normals = 0;
	for (PointList::const_iterator p = unoriented_points_begin; p != points.end(); p++)
	{
		unoriented_normals++;
	}
	if (unoriented_normals > 0)
	{
		std::cout << "Error: " << unoriented_normals << " normals are unoriented\n";
		success = false;
	}

	// Compare oriented normals with original ones and count flipped normals
	bool points_have_original_normals = !original_normals.empty();
	if (points_have_original_normals)
	{
		assert(points.size() == original_normals.size());

		std::cout << "Compare with original normals:" << std::endl;

		int flipped_normals = 0; // #normals with wrong orientation
		PointList::const_iterator p;
		std::vector<Vector>::const_iterator n;
		for (p = points.begin(), n = original_normals.begin(); p != unoriented_points_begin; p++, n++)
		{
			Vector v1 = *n; // original normal
			double norm1 = std::sqrt(v1*v1);
			assert(norm1 != 0.0);
			Vector v2 = p->second; // computed normal
			double norm2 = std::sqrt(v2*v2);
			assert(norm2 != 0.0);
			double cos_normal_deviation = (v1*v2) / (norm1*norm2);
			if (cos_normal_deviation < 0) // if flipped
			{
				flipped_normals++;
			}
		}

		if (flipped_normals == 0)
			std::cout << "  ok\n";
		else
			std::cout << "  Error: " << flipped_normals << " normal(s) are flipped\n";
	}

	if (_bIsEraseUnOriented && unoriented_normals) {
		m_ptSet.erase(unoriented_points_begin, m_ptSet.end());
		std::cout << unoriented_normals << "  normals have been erased" << std::endl;
		std::cout << "after erase : " << m_ptSet.size() << std::endl;
	}
	return success;
}

bool PointCloudReconstructCGAL::verify_normal_direction(const PointList& points, // input points + computed normals
														const std::vector<Vector>& original_normals) // may be empty
{
	bool success = true;
	bool points_have_original_normals = !original_normals.empty();
	if (points_have_original_normals)
	{
		assert(points.size() == original_normals.size());

		std::cout << "Compare with original normals:" << std::endl;

		double min_normal_deviation = DBL_MAX; // deviation / original normal
		double max_normal_deviation = DBL_MIN;
		double avg_normal_deviation = 0;
		int invalid_normals = 0; // #normals with large deviation
		PointList::const_iterator p;
		std::vector<Vector>::const_iterator n;
		for (p = points.begin(), n = original_normals.begin(); p != points.end(); p++, n++)
		{
			// Computes normal deviation.
			Vector v1 = *n; // original normal
			double norm1 = std::sqrt(v1*v1);
			assert(norm1 != 0.0);
			Vector v2 = p->second; // computed normal
			double norm2 = std::sqrt(v2*v2);
			assert(norm2 != 0.0);
			double cos_normal_deviation = (v1*v2) / (norm1*norm2);
			if (cos_normal_deviation < 0)
			{
				cos_normal_deviation = -cos_normal_deviation;
			}
			double normal_deviation = std::acos(cos_normal_deviation);

			// statistics about normals deviation
			min_normal_deviation = (std::min)(min_normal_deviation, normal_deviation);
			max_normal_deviation = (std::max)(max_normal_deviation, normal_deviation);
			avg_normal_deviation += normal_deviation;

			// count normal if large deviation
			bool valid = (normal_deviation <= CGAL_PI / 3.); // valid if deviation <= 60 degrees
			if (!valid)
			{
				invalid_normals++;
			}
		}
		avg_normal_deviation /= double(points.size());

		std::cout << "  Min normal deviation=" << min_normal_deviation * 180.0 / CGAL_PI << " degrees\n";
		std::cout << "  Max normal deviation=" << max_normal_deviation * 180.0 / CGAL_PI << " degrees\n";
		std::cout << "  Avg normal deviation=" << avg_normal_deviation * 180.0 / CGAL_PI << " degrees\n";
		if (invalid_normals > 0)
		{
			std::cout << "  Error: " << invalid_normals << " normals have a deviation > 60 degrees\n";
			success = false;
		}
	}

	return success;
}


//////////////////////////////////////////////////////////////////////////

void PointCloudReconstructCGAL::PoissonSurfaceReconstrutionDelaunay(double sm_angle/* = 20.0*/, double sm_radius /*= 30.0*/, double sm_distance /*= 0.375*/) {
#ifdef PRINT_DETAIL
	std::cout << " Performs surface reconstruction with spacing (average spacing=" << m_dAvgSpacing << ")...\n";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	//FT sm_angle = 20.0; // Min triangle angle in degrees.
	//FT sm_radius = 30; // Max triangle size w.r.t. point set average spacing.
	//FT sm_distance = 0.1*m_dAvgSpacing; // Surface Approximation error w.r.t. point set average spacing.

	if (!(CGAL::poisson_surface_reconstruction_delaunay(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(),
		CGAL::Second_of_pair_property_map<PointVectorPair>(),
		output_mesh, m_dAvgSpacing, sm_angle, sm_radius, sm_distance)))
	{
		std::cerr << "Error: reconstruction failed." << std::endl;
		return;
	}

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << " Performs surface reconstruction eclipse : " << time << " second(s)" << std::endl;
#endif
}

void PointCloudReconstructCGAL::PoissonSurfaceReconstrutionDetail(double sm_angle/* = 20.0*/, double sm_radius /*= 30.0*/, double sm_distance /*= 0.375*/) {

#ifdef PRINT_DETAIL
	std::cout << " processing PossionSurfaceReconstruction ......";
	CGAL::Timer task_timer;
	task_timer.start();
#endif

	sm_angle = 15.0; // Min triangle angle in degrees.
	sm_radius = 20; // Max triangle size w.r.t. point set average spacing.
	sm_distance = 0.1*m_dAvgSpacing; // Surface Approximation error w.r.t. point set average spacing.


	// Creates implicit function from the read points using the default solver.
	// Note: this method requires an iterator over points
	// + property maps to access each point's position and normal.
	// The position property map can be omitted here as we use iterators over Point_3 elements.
	Poisson_reconstruction_function function(m_ptSet.begin(), m_ptSet.end(),
		CGAL::First_of_pair_property_map<PointVectorPair>(), CGAL::Second_of_pair_property_map<PointVectorPair>());
	// Computes the Poisson indicator function f()
	// at each vertex of the triangulation.
	if (!function.compute_implicit_function())
		return;
	// Computes average spacing
	FT average_spacing = m_dAvgSpacing;
	// Gets one point inside the implicit surface
	// and computes implicit function bounding sphere radius.
	Point inner_point = function.get_inner_point();
	Sphere bsphere = function.bounding_sphere();
	FT radius = std::sqrt(bsphere.squared_radius());
	// Defines the implicit surface: requires defining a
	// conservative bounding sphere centered at inner point.
	FT sm_sphere_radius = 2.0 * radius;
	FT sm_dichotomy_error = sm_distance * average_spacing / 1000.0; // Dichotomy error must be << sm_distance
	Surface_3 surface(function,
		Sphere(inner_point, sm_sphere_radius*sm_sphere_radius),
		sm_dichotomy_error / sm_sphere_radius);
	// Defines surface mesh generation criteria
	CGAL::Surface_mesh_default_criteria_3<STr> criteria(sm_angle,  // Min triangle angle (degrees)
		sm_radius*average_spacing,  // Max triangle size
		sm_distance*average_spacing); // Approximation error
									  // Generates surface mesh with manifold option
	STr tr; // 3D Delaunay triangulation for surface mesh generation
	C2t3 c2t3(tr); // 2D complex in 3D Delaunay triangulation
	CGAL::make_surface_mesh(c2t3,                                 // reconstructed mesh
		surface,                              // implicit surface
		criteria,                             // meshing criteria
		CGAL::Manifold_with_boundary_tag()/*CGAL::Manifold_tag()*/);  // require manifold mesh
	if (tr.number_of_vertices() == 0)
		return;

	CGAL::output_surface_facets_to_polyhedron(c2t3, output_mesh);
	// computes the approximation error of the reconstruction
	//double max_dist =CGAL::Polygon_mesh_processing::approximate_max_distance_to_point_set(output_mesh,m_ptSet,4000);
	//std::cout << "Max distance to point_set: " << max_dist << std::endl;

#ifdef PRINT_DETAIL
	task_timer.stop();
	double time = task_timer.time();
	std::cout << "function: PossionSurfaceReconstruction eclipse " << time << " second(s)" << std::endl;
#endif
}

//////////////////////////////////////////////////////////////////////////

void PointCloudReconstructCGAL::ReconstrutUsingAFT(std::string offfilename, double radius_ratio_bound /*= 5.0*/, double beta /*= 0.52*/) {

	double per = 0.0;
	std::vector<Point> points;
	std::vector<Facet> facets;

	points.resize(m_ptSet.size());
	for (int i = 0; i < m_ptSet.size();++i)
	{
		points[i] = m_ptSet[i].first;
	}

	Mesh m;
	Construct construct(m, points.begin(), points.end());
	CGAL::advancing_front_surface_reconstruction(points.begin(),
		points.end(),
		construct,
		radius_ratio_bound,
		beta);

	std::ofstream out(offfilename);
	out << m;
	out.close();

	std::vector<Point>().swap(points);
	std::vector<Facet>().swap(facets);
}