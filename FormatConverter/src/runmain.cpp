#include<iostream>
#include "..\include\DMesh.h"
#include "..\include\export_base_file.h"

//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {


	if (argc < 5) {
		std::cout << " not enough parameter for format convert " << std::endl;
		return -1;
	}

	std::string input_file = "";
	std::string out_file = "";
	for (int i = 1; i < argc; ++i)
	{

		if (std::string(argv[i]) == "-i") {
			input_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-o") {
			out_file = std::string(argv[i + 1]);
		}
	}

	if (input_file.empty() || out_file.empty())
		return -2;

	DMesh m_mesh;

	int err = vcg::tri::io::Importer<DMesh >::Open(m_mesh, input_file.c_str());
	if (vcg::tri::io::Importer<DMesh>::E_NOERROR != err)
	{
		std::cout << "failed to open file :  " << input_file << std::endl;
		std::cout << "error code is : " << vcg::tri::io::Importer<DMesh>::ErrorMsg(err) << std::endl;
		return -3;
	}

	err = vcg::tri::io::Exporter<DMesh >::Save(m_mesh, out_file.c_str());
	if (vcg::tri::io::Exporter<DMesh>::E_NOERROR != err)
	{
		std::cout << "failed to export file :  " << out_file << std::endl;
		std::cout << "error code is : " << vcg::tri::io::Exporter<DMesh>::ErrorMsg(err) << std::endl;
		return -4;
	}

	return 0;
}
