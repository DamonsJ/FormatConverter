#ifndef _GDALFORMAT_HEADER_
#define _GDALFORMAT_HEADER_


#include <iostream>
#include <fstream>
#include <algorithm>

// utility header
#include "../include/DLogger.h"
#include "../include/DTimer.h"

using namespace DUtility;
class /*__declspec(dllexport)*/ GDALFormatConvert {

public:
	GDALFormatConvert();
	~GDALFormatConvert();

public:
	//************************************  
	// @brief : set logger : if you don't need  log, you can ignore this function
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: void
	// @param : void  
	//************************************ 
	static void SetLogger(DLog *_log) {
		m_log = _log;
	}
public:
	//************************************  
	// @brief : convert esi shape file to wasp .map file
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _map_file_name : map file name
	//************************************ 
	static std::string Shape2WAsPMap(std::string _shp_file_name, std::string _map_file_name);
	//************************************  
	// @brief : convert esi shape file to geojson file
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _geojson_file_name : geojson file name
	//************************************ 
	static std::string Shape2GeoJSON(std::string _shp_file_name, std::string _geojson_file_name);

public:
	//************************************  
	// @brief : convert wasp map file to  esi shape file  
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _map_file_name : map file name
	// @param : std::string projInfo : projection info ,wasp .map file do not have projection normally
	//************************************ 
	static std::string WAsPMap2Shape(std::string _map_file_name, std::string _shp_file_name, std::string projInfo = "");
	//************************************  
	// @brief : convert wasp map file to  esi shape file  
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _map_file_name : map file name
	// @param : int epsg : epsg belt number
	//************************************ 
	static std::string WAsPMap2Shape(std::string _map_file_name, std::string _shp_file_name, int epsg = 0);
public:
	//************************************  
	// @brief : convert  esi shape file  to rasterized tif file
	// @detail: **rasterize** shaped to raster file which is saved as tif file
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _tif_file_name : rasterized tif file name
	// @param : int dstWidth: dest tif file width
	// @param : int dstHeight: dest tif file height
	// @param : char * elvationField: if elvationField is not null, elevation is consider using
	//			field specified by elvationField
	//************************************ 
	static std::string Shape2Tiff_Rasterize(std::string _shp_file_name, std::string _tif_file_name, int dstWidth, int dstHeight,char * elvationField = nullptr);
	//************************************  
	// @brief : convert  esi shape file  to rasterized tif file
	// @detail: **rasterize** shaped to raster file which is saved as tif file
	// @author: SunHongLei
	// @date  : 2019/01/07  
	// @return: std::string: error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _tif_file_name : rasterized tif file name
	// @param : double pixelWidth:  dest tif file pixel in width
	// @param : double pixelHeight: dest tif file  pixel in height
	// @param : char * elvationField: if elvationField is not null, elevation is consider using
	//			field specified by elvationField
	//************************************ 
	static std::string Shape2Tiff_Rasterize(std::string _shp_file_name, std::string _tif_file_name, double pixelWidth, double pixelHeight, char * elvationField = nullptr);

public:
	//************************************  
	// @brief : convert shape file to dem tif file using interpolation method 
	// @author: SunHongLei
	// @date  : 2019/01/09  
	// @return: std::string error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _tif_file_name : interpolated tif file name
	// @param : int dstWidth: dest tif file width
	// @param : int dstHeight: dest tif file height
	// @param : char * elvationField: if elvationField is not null, elevation is consider using
	//			field specified by elvationField
	// @param : algorithm type : "invdistnn" = 1, "invdist"  = 2, "average"  = 3,  "linear"  = 4, "nearest"  = 5 
	//************************************ 
	static std::string Shape2Tiff_Interpolation(std::string _shp_file_name, std::string _tif_file_name, int dstWidth, int dstHeight, const char * elvationField = nullptr, int algo = 5);
	//************************************  
	// @brief : convert shape file to dem tif file using interpolation method 
	// @author: SunHongLei
	// @date  : 2019/01/09  
	// @return: std::string error message: empty if no error
	// @param : std::string _shp_file_name : shape file name
	// @param : std::string _tif_file_name : interpolated tif file name
	// @param : double pixelWidth:  dest tif file pixel in width
	// @param : double pixelHeight: dest tif file  pixel in height
	// @param : char * elvationField: if elvationField is not null, elevation is consider using
	//			field specified by elvationField
	// @param : algorithm type : "invdistnn" = 1, "invdist"  = 2, "average"  = 3,  "linear"  = 4, "nearest"  = 5 
	//************************************ 
	static std::string Shape2Tiff_Interpolation(std::string _shp_file_name, std::string _tif_file_name, double pixelWidth, double pixelHeight, const char * elvationField = nullptr, int algo = 5);
public:
	//************************************  
	// @brief : generate contour from dem file and write result as shp file  
	// @author: SunHongLei
	// @date  : 2019/01/09  
	// @return: void
	// @param : std::string _dem_file_name: dem data file
	// @param : std::string _contour_file_name: contour file as shp
	// @param : double dContourInterval: contour interval value
	//************************************ 
	static std::string GenerateContourFromDemWriteAsShpFile(std::string _dem_file_name, std::string _contour_file_name, double dContourInterval);
	//************************************  
	// @brief : generate contour from dem file and write result as wasp map file  
	// @author: SunHongLei
	// @date  : 2019/01/09  
	// @return: void
	// @param : std::string _dem_file_name: dem data file
	// @param : std::string _map_file_name: map file name
	// @param : double dContourInterval: contour interval value
	//************************************ 
	static std::string GenerateContourFromDemWriteAsMapFile(std::string _dem_file_name, std::string _map_file_name, double dContourInterval);
	//************************************  
	// @brief : generate contour from dem file and write result as geojson file  
	// @author: SunHongLei
	// @date  : 2019/01/09  
	// @return: void
	// @param : std::string _dem_file_name: dem data file
	// @param : std::string _geojson_file_name: geojson file name
	// @param : double dContourInterval: contour interval value
	//************************************ 
	static std::string GenerateContourFromDemWriteAsGeoJSONFile(std::string _dem_file_name, std::string _geojson_file_name, double dContourInterval);

public:
	//************************************  
	// @brief : extract boundary from vector file and output boundary as _out_shp_name 
	// @author: SunHongLei
	// @date  : 2019/03/22  
	// @return: void
	// @param : double pixel : pixel while rasterize shape file  in meter
	// @param : double thresh_hold : thresh hold used to smooth raster
	// @param : double thresh_area : thresh hold area used to filter small boundary
	//************************************ 
	static std::string ExtractShapeFileBoundary(std::string _shp_file_name, std::string _out_shp_name, double pixel = 15,double thresh_hold = 140.0, double thresh_area = -1.0);
protected:
	static void WriteInfoLog(std::string _Msg);
protected:
	static DLog *m_log;
	static DTimer m_timer;
};

#endif// 2019/01/04 