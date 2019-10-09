#include <iostream>
#include "..\include\vtkExtractInfo.h"

int main() {

	std::string vtkName = "F:\\VTKProgram\\VTKSimplification\\data\\test\\layer_1\\test.vtk";
	vtkInfoExtraction *info = new vtkInfoExtraction(vtkName);
	std::vector<std::string > result;
	info->GetPointAttributeDataList(result);
	info->GetVTKInfo();
	system("pause");
	return 0;
}