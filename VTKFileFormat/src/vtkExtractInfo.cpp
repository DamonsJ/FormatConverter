#include "..\include\vtkExtractInfo.h"

void vtkInfoExtraction::GetPointAttributeDataList(std::vector<std::string> &pData_AttriData) {
	//////////////////////////////////////////////////////////////////////////
	pData_AttriData.clear();
	pData_AttriData.shrink_to_fit();
	//////////////////////////////////////////////////////////////////////////
	if (vtkreader && vtkreader->IsFilePolyData()) {
		// get poly data
		vtkSmartPointer<vtkPolyData> polyData = vtkreader->GetPolyDataOutput();
		if (!polyData) {
			std::cout << "failed to read poly data " << std::endl;
			return ;
		}
		// get point attributes data
		vtkSmartPointer<vtkPointData> pd = polyData->GetPointData();
		int pa = pd->GetNumberOfArrays();
		for (int i = 0; i< pa;++i)
		{
			const char *name = pd->GetArrayName(i);
			pData_AttriData.push_back(std::string(name));
		}
	}
}

void vtkInfoExtraction::GetCellAttributeDataList(std::vector<std::string> &pData_AttriData) {
	//////////////////////////////////////////////////////////////////////////
	pData_AttriData.clear();
	pData_AttriData.shrink_to_fit();
	//////////////////////////////////////////////////////////////////////////
	if (vtkreader && vtkreader->IsFilePolyData()) {
		// get poly data
		vtkSmartPointer<vtkPolyData> polyData = vtkreader->GetPolyDataOutput();
		if (!polyData) {
			std::cout << "failed to read poly data " << std::endl;
			return;
		}
		// get point attributes data
		vtkSmartPointer<vtkCellData> cd = polyData->GetCellData();
		int pa = cd->GetNumberOfArrays();
		for (int i = 0; i < pa; ++i)
		{
			const char *name = cd->GetArrayName(i);
			pData_AttriData.push_back(std::string(name));
		}
	}
}

void vtkInfoExtraction::GetAttributesInFile(std::vector<std::string> &pData_AttriData) {
	//////////////////////////////////////////////////////////////////////////
	pData_AttriData.clear();
	pData_AttriData.shrink_to_fit();
	//////////////////////////////////////////////////////////////////////////
	if (vtkreader && vtkreader->IsFilePolyData()) {
		// get poly data
		vtkSmartPointer<vtkPolyData> polyData = vtkreader->GetPolyDataOutput();
		if (!polyData) {
			std::cout << "failed to read poly data " << std::endl;
			return;
		}
		// get point attributes data
		vtkSmartPointer<vtkPointData> pd = polyData->GetPointData();
		int pa = pd->GetNumberOfArrays();
		for (int i = 0; i < pa; ++i)
		{
			const char *name = pd->GetArrayName(i);
			pData_AttriData.push_back(std::string(name));
		}

		// get point attributes data
		vtkSmartPointer<vtkCellData> cd = polyData->GetCellData();
		int ca = cd->GetNumberOfArrays();
		for (int i = 0; i < ca; ++i)
		{
			const char *name = cd->GetArrayName(i);
			pData_AttriData.push_back(std::string(name));
		}
	}
}

vtkDataArray* vtkInfoExtraction::GetAttributesByName(std::string _attribute_name) {
	if (vtkreader && vtkreader->IsFilePolyData()) {
		// get poly data
		vtkSmartPointer<vtkPolyData> polyData = vtkreader->GetPolyDataOutput();
		if (!polyData) {
			std::cout << "failed to read poly data " << std::endl;
			return nullptr;
		}
		// get point attributes data
		vtkSmartPointer<vtkPointData> pd = polyData->GetPointData();
		vtkDataArray*arrp = pd->GetArray(_attribute_name.c_str());
		if (arrp)
			return arrp;

		vtkSmartPointer<vtkCellData> cd = polyData->GetCellData();
		vtkDataArray*arrc = cd->GetArray(_attribute_name.c_str());
		if (arrc)
			return arrc;
	}

	return nullptr;
}

void vtkInfoExtraction::GetVTKInfo() {
	if (vtkreader && vtkreader->IsFilePolyData()) {
		vtkSmartPointer<vtkPolyData> polyData = vtkreader->GetPolyDataOutput();
		if (!polyData) {
			std::cout << "failed to read poly data " << std::endl;
			return;
		}
		int nPoints = polyData->GetNumberOfPoints();
		int nCells = polyData->GetNumberOfCells();

		// get points
		vtkSmartPointer<vtkPoints> points = polyData->GetPoints();
		vtkSmartPointer<vtkDataArray> points_data = points->GetData();
		//int nc = points_data->GetNumberOfComponents();
		//long numberOfVertices = points_data->GetNumberOfTuples();
		//for (int i = 0; i < numberOfVertices; i++)
		//{
		//	float x = vc->GetComponent(i, 0);
		//	float y = vc->GetComponent(i, 1);
		//	float z = vc->GetComponent(i, 2);
		//}
		// Write all of the coordinates of the points in the vtkPolyData to the console.
		for (vtkIdType i = 0; i < polyData->GetNumberOfPoints(); i++)
		{
			double p[3];
			polyData->GetPoint(i, p);
			// This is identical to:
			// polydata->GetPoints()->GetPoint(i,p);
			std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2] << ")" << std::endl;
		}

		// get polygons
		vtkSmartPointer<vtkCellArray > polys = polyData->GetPolys();
		//vtkIdType celllocation = 0;
		//for (vtkIdType i = 0; i < polys->GetNumberOfCells(); i++)
		//{
		//	vtkIdType numIds;
		//	vtkIdType *pointIds;
		//
		//	polys->GetCell(celllocation, numIds,pointIds);
		//	celllocation += 1 + numIds;
		//	for (int j = 0; j < numIds;j++)
		//	{
		//		std::cout << pointIds[j] << " ";
		//	}
		//	std::cout << std::endl;
		//}

		polys->InitTraversal();
		for (vtkIdType i = 0; i < polys->GetNumberOfCells(); i++)
		{
			vtkIdType numIds;
			vtkIdType *pointIds;
			polys->GetNextCell(numIds,pointIds);
			for (int j = 0; j < numIds; j++)
			{
				std::cout << pointIds[j] << " ";
			}
			std::cout << std::endl;
		}
	}
}