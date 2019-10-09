#include "..\include\DMesh.h"

int main() {

	DMesh m_mesh;
	std::string _fileName = "F:/FormatConverter/FormatConverter/data/result_remesh.3ds";

	int err = vcg::tri::io::Importer<DMesh >::Open(m_mesh, _fileName.c_str());
	if (vcg::tri::io::Importer<DMesh>::E_NOERROR != err)
	{
		std::cout << "failed to open file :  " << _fileName << std::endl;
		std::cout << "error code is : " << vcg::tri::io::Importer<DMesh>::ErrorMsg(err) << std::endl;
		return false;
	}
	m_mesh.printMeshInfo();

	system("pause");
	return 0;
}