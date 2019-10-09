#include "..\include\PointCloudReconstructCGAL.h"
#include "..\include\MeshProcessingCGAL.h"
#include <string>
int main(int argc,char **argv) {
	if (argc < 4) {
		std::cout << " not enough parameter for build model " << std::endl;
		return -1;
	}

	std::string input_file = "";
	std::string out_file = "";
	std::string ratio_str = "";
	std::string range_str = "";
	for (int i = 1; i < argc; ++i)
	{

		if (std::string(argv[i]) == "-i") {
			input_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-o") {
			out_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-r") {
			ratio_str = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-rg") {
			range_str = std::string(argv[i + 1]);
		}
	}

	if (input_file.empty() || out_file.empty())
		return -2;

	if(std::string(argv[1]) == "-reconstruct")
	{
		PointCloudReconstructCGAL *m_pcr = new PointCloudReconstructCGAL;
		if (m_pcr->ReadPointCloudFromFile(input_file))
		{
			m_pcr->AnalysisPointCloud(2.0);
			m_pcr->RemoveOutlierPointsDistance();
			m_pcr->Simplification_Grid(10.0*m_pcr->GetAveragingValue());
			m_pcr->ReconstrutUsingAFT(out_file);
		}
		else {
			return -3;
		}
		delete m_pcr;
	}
	else if (std::string(argv[1]) == "-remesh")
	{
		MeshProcessingCGAL m_mpCGAL;
		if (m_mpCGAL.ReadMesh(input_file))
			m_mpCGAL.RefineMesh(out_file);
		else
			return -4;
	}
	else if (std::string(argv[1]) == "-simplify")
	{
		double ratio = 0.1;
		if (!ratio_str.empty())
			ratio = std::stod(ratio_str);

		MeshProcessingCGAL m_mpCGAL;
		if (m_mpCGAL.ReadMesh(input_file))
			m_mpCGAL.Simplification(out_file, ratio);
		else
			return -4;
	}
	else if (std::string(argv[1]) == "-clip")
	{
		if (!range_str.empty())
		{
			std::string::size_type pos;
			std::vector<double > values;
			while ((pos = range_str.find_first_of(","))!= std::string::npos)
			{
				std::string val_str = range_str.substr(0,pos);
				range_str = range_str.substr(pos+1);
				values.push_back(std::stod(val_str));
			}
			if (values.size()!=6)
			{
				return -6;
			}
			MeshProcessingCGAL m_mpCGAL;
			if (m_mpCGAL.ReadMesh(input_file))
				m_mpCGAL.ClipMesh(out_file, values[0], values[1], values[2], values[3], values[4], values[5]);
			else
				return -4;

			std::vector<double >().swap(values);
		}
		else {
			return -5;
		}
		
	}
	return 0;
}
