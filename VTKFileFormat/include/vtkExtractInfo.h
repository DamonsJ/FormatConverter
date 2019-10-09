#ifndef _VTKEXTRACTINFO_HEADER_
#define _VTKEXTRACTINFO_HEADER_

#include <string>
#include <vector>

#include <vtkSmartPointer.h>
#include <vtkDataReader.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtksys/SystemTools.hxx>
#include <vtkGenericDataObjectReader.h>

class vtkInfoExtraction
{
public:
	vtkInfoExtraction() {
		vtkreader = nullptr;
	}
	vtkInfoExtraction(std::string _vtk_file_name) {
		vtkreader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
		vtkreader->SetFileName(_vtk_file_name.c_str());
		vtkreader->Update();
	}
	~vtkInfoExtraction() {
	}

public:
	void GetPointAttributeDataList(std::vector<std::string> &pData_AttriData);
	void GetCellAttributeDataList(std::vector<std::string> &pData_AttriData);
	void GetAttributesInFile(std::vector<std::string> &pData_AttriData);
	vtkDataArray* GetAttributesByName(std::string _attribute_name);
public:
	void GetVTKInfo();
protected:
	vtkSmartPointer<vtkGenericDataObjectReader> vtkreader;
};

#endif// 2019/01/10