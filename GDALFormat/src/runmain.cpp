#include "../include/GDALFormat.h"
#include "../include/DPath.h"

#include <iostream>


//////////////////////////////////////////////////////////////////////////
bool GenerateContour(int argc, char **argv) {

	bool isDebug = false;
	std::string input_file = "";
	std::string out_file = "";
	double interval = 0.0;
	for (int i = 2; i < argc;++i)
	{
		if (std::string(argv[i]) == "-d") {
			isDebug = true;
		}
		if (std::string(argv[i]) == "-i") {
			input_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-o") {
			out_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-v") {
			interval = std::stod(std::string(argv[i + 1]));
		}
	}

	if (input_file.empty() || out_file.empty())
		return false;

	DLog *log = nullptr;
	if (isDebug) {
		log = new DLog;
		log->init("gdal_format_logger");
		GDALFormatConvert::SetLogger(log);
	}
	DUtility::DPath m_path(out_file);
	std::string ext = m_path.extension();
	std::string err;

	if (ext == std::string("map")) {
		err = GDALFormatConvert::GenerateContourFromDemWriteAsMapFile(input_file, out_file, interval);
	}
	else if (ext == std::string("geojson")) {
		err = GDALFormatConvert::GenerateContourFromDemWriteAsGeoJSONFile(input_file, out_file, interval);
	}
	else if (ext == std::string("shp")) {
		err = GDALFormatConvert::GenerateContourFromDemWriteAsShpFile(input_file, out_file, interval);
	}


	if (log)
		delete log;

	if (err.empty())
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CreateGrid(int argc, char **argv) {

	bool isDebug = false;
	std::string input_file = "";
	std::string out_file = "";
	std::string elevation = "elevation";
	double pixel = 0.0;
	for (int i = 2; i < argc; ++i)
	{
		if (std::string(argv[i]) == "-d") {
			isDebug = true;
		}
		if (std::string(argv[i]) == "-i") {
			input_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-o") {
			out_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-p") {
			pixel = std::stod(std::string(argv[i + 1]));
		}
		if (std::string(argv[i]) == "-e") {
			elevation = std::string(argv[i + 1]);
		}
	}

	if (input_file.empty() || out_file.empty())
		return false;

	DLog *log = nullptr;
	if (isDebug) {
		log = new DLog;
		log->init("gdal_format_logger_cgd");
		GDALFormatConvert::SetLogger(log);
	}
	//DUtility::DPath m_path(input_file);
	//std::string ext = m_path.extension();
	std::string err = GDALFormatConvert::Shape2Tiff_Interpolation(input_file, out_file, pixel, -pixel, elevation.c_str());
	
	if (log)
		delete log;

	if (err.empty())
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool ExtractBoundary(int argc, char **argv) {

	bool isDebug = false;
	std::string input_file = "";
	std::string out_file = "";
	
	double pixel = 15.0;
	double thresh_hold = 140.0;
	double thresh_area = 140.0*140.0*2.0;
	for (int i = 2; i < argc; ++i)
	{
		if (std::string(argv[i]) == "-d") {
			isDebug = true;
		}
		if (std::string(argv[i]) == "-i") {
			input_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-o") {
			out_file = std::string(argv[i + 1]);
		}
		if (std::string(argv[i]) == "-p") {
			pixel = std::stod(std::string(argv[i + 1]));
		}
		if (std::string(argv[i]) == "-t") {
			thresh_hold = std::stod(std::string(argv[i + 1]));
		}
		if (std::string(argv[i]) == "-a") {
			thresh_area = std::stod(std::string(argv[i + 1]));
		}
	}

	if (input_file.empty() || out_file.empty())
		return false;

	DLog *log = nullptr;
	if (isDebug) {
		log = new DLog;
		log->init("gdal_format_logger_cgd");
		GDALFormatConvert::SetLogger(log);
	}

	std::string err = GDALFormatConvert::ExtractShapeFileBoundary(input_file, out_file, pixel, thresh_hold, thresh_area);
	if (log)
		delete log;

	if (err.empty())
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool FormatConvert(int argc, char **argv) {
	std::string input_file = "";
	std::string out_file = "";
	for (int i = 2; i < argc; ++i)
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

	DPath m_path1(input_file);
	DPath m_path2(out_file);

	if(m_path1.extension() == "shp" && m_path2.extension() == "geojson")
	{
		std::string err = GDALFormatConvert::Shape2GeoJSON(input_file, out_file);
		if (err.empty())
			return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {

	//////////////////////////////////////////////////////////////////////////
	//for (int i = 0; i < argc;++i)
	//{
	//	std::cout <<i<<"  parameter: "<< argv[i] << std::endl;
	//}
	//////////////////////////////////////////////////////////////////////////
	if (std::string(argv[1]) == "-gc") {

		if (argc < 8) {
			std::cout << " not enough parameter for generate contour " << std::endl;
			return -1;
		}

		GenerateContour(argc, argv);
	}
	else if (std::string(argv[1]) == "-cgd"){
		if (argc < 8) {
			std::cout << " not enough parameter for create grid " << std::endl;
			return -2;
		}
		CreateGrid(argc, argv);
	}
	else if (std::string(argv[1]) == "-eb") {
		if (argc < 5) {
			std::cout << " not enough parameter for extract boundary " << std::endl;
			return -3;
		}
		ExtractBoundary(argc, argv);
	}
	else if (std::string(argv[1]) == "-ftc") {
		if (argc < 5) {
			std::cout << " not enough parameter for extract boundary " << std::endl;
			return -3;
		}
		FormatConvert(argc, argv);
	}
	return 0;
}
