#include "gdal_utils.h"
#include "gdal_alg.h"
#include "gdal_priv.h"
#include "cpl_conv.h" 
#include "ogr_spatialref.h"
#include "ogr_api.h"
#include "ogrsf_frmts.h"
#include "../include/GDALFormat.h"
//////////////////////////////////////////////////////////////////////////
DLog *GDALFormatConvert::m_log = nullptr;
DTimer GDALFormatConvert::m_timer;

//////////////////////////////////////////////////////////////////////////
GDALFormatConvert::GDALFormatConvert() {

}

GDALFormatConvert::~GDALFormatConvert() {

}
//////////////////////////////////////////////////////////////////////////
void GDALFormatConvert::WriteInfoLog(std::string _Msg) {

	if (m_log) {
		m_log->info(_Msg);
	}
}
//////////////////////////////////////////////////////////////////////////

std::string GDALFormatConvert::Shape2WAsPMap(std::string _shp_file_name, std::string _map_file_name) {

	WriteInfoLog("<-------------begin convert shape file to wasp map file------------->");
	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		return errMsg;
	}
	WriteInfoLog("<-----------------------open shape file done------------------------>");
	char **papszOptions = nullptr;
	papszOptions = CSLAddNameValue(papszOptions, "WASP_FIELDS", "elevation");
	GDALDriver  *poDriver = GetGDALDriverManager()->GetDriverByName("WAsP");
	GDALDatasetH poDstDS = OGR_Dr_CopyDataSource((OGRSFDriverH)poDriver, poDS, _map_file_name.c_str(), papszOptions);
	GDALClose(poDS);
	GDALClose(poDstDS);
	double t = m_timer.stop();
	WriteInfoLog("<-------------write wasp map file done in : ------------->" + std::to_string(t));
	return errMsg;
}

std::string GDALFormatConvert::Shape2GeoJSON(std::string _shp_file_name, std::string _geojson_file_name) {

	WriteInfoLog("<-------------begin convert shape file to geojson file------------->");
	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		return errMsg;
	}
	WriteInfoLog("<-----------------------open shape file done------------------------>");
	char **papszOptions = nullptr;
	GDALDriver  *poDriver = GetGDALDriverManager()->GetDriverByName("GeoJSON");
	GDALDatasetH poDstDS = OGR_Dr_CopyDataSource((OGRSFDriverH)poDriver, poDS, _geojson_file_name.c_str(), papszOptions);
	GDALClose(poDS);
	GDALClose(poDstDS);
	double t = m_timer.stop();
	WriteInfoLog("<-------------write geojson file done in : ------------->" + std::to_string(t));
	return errMsg;

}
//////////////////////////////////////////////////////////////////////////
std::string GDALFormatConvert::WAsPMap2Shape(std::string _map_file_name, std::string _shp_file_name, std::string projInfo/*=""*/) {

	WriteInfoLog("<-------------begin convert wasp map file to shape file------------->");
	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDatasetH poDS = GDALOpenEx(_map_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _map_file_name;
		return errMsg;
	}

	WriteInfoLog("<-----------------------open map file done------------------------>");
	GDALDriver  *poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	GDALDataset *poDstDS = poDriver->CreateCopy(_shp_file_name.c_str(), (GDALDataset *)poDS, true, nullptr, nullptr, nullptr);
	if (!projInfo.empty())
	{
		std::size_t pos = _shp_file_name.find_last_of(".");
		if (pos != std::string::npos) {
			std::string prjname = _shp_file_name.substr(0, pos) + ".prj";
			std::ofstream ofm(prjname);
			ofm << projInfo;
			ofm.close();
		}
		else {
			errMsg = "error: write prj file error ";
		}
	}
	GDALClose(poDS);
	GDALClose(GDALDataset::ToHandle(poDstDS));
	double t = m_timer.stop();
	WriteInfoLog("<-------------write shape file done in : ------------->" + std::to_string(t));
	return errMsg;
}

std::string GDALFormatConvert::WAsPMap2Shape(std::string _map_file_name, std::string _shp_file_name, int epsg /*= 0*/) {
	std::string prjinfo = "";
	std::string errMsg;
	if (epsg != 0) {
		OGRSpatialReference oReference;
		OGRErr err = oReference.importFromEPSG(epsg);
		if (err != OGRERR_NONE) {
			errMsg = "import epsg error : " + std::to_string(epsg);
			return errMsg;
		}

		char *pszWkt;
		err = oReference.exportToWkt(&pszWkt);
		if (err != OGRERR_NONE) {
			errMsg = "export epsg to string error : " + std::to_string(epsg);
			return errMsg;
		}

		prjinfo = std::string(pszWkt);
	}

	errMsg = WAsPMap2Shape(_map_file_name, _shp_file_name, prjinfo);
	return errMsg;
}

//////////////////////////////////////////////////////////////////////////

std::string GDALFormatConvert::Shape2Tiff_Rasterize(std::string _shp_file_name, std::string _tif_file_name, int dstWidth, int dstHeight, char * elvationField /*= nullptr*/) {
	WriteInfoLog("<-------------begin convert shape file to dem tif file with width and height specified------------->");

	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	//1.  read shape file
	WriteInfoLog("<-------------1. read shape file ------------->");
	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	GDALDataset *pShpDataSet = GDALDataset::FromHandle(poDS);
	//2.  get layers and extract info
	WriteInfoLog("<-------------2. extract info from layer ------------->");
	OGRLayer *layer = pShpDataSet->GetLayer(0);
	if (layer == nullptr) {
		errMsg = "error: error happen when trying to get layer in  " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	// get layer's extent
	OGREnvelope env;
	layer->GetExtent(&env);
	// get projection info
	OGRSpatialReference *pOgrSRS = nullptr;
	pOgrSRS = layer->GetSpatialRef();

	int rwidth = dstWidth;
	int rheight = dstHeight;
	if (std::abs(rwidth) > 0 && std::abs(rheight) > 0) {
		WriteInfoLog("<-------------2.1 height and width user specified ------------->");
	}
	else {
		errMsg = "error: do not have width and height specified";
		WriteInfoLog(errMsg);
		return errMsg;
	}

	// get projection info
	char *prjInfo = nullptr;
	if (pOgrSRS == nullptr) {
		errMsg = "error: can not get projection from " + _shp_file_name;
		WriteInfoLog(errMsg);
	}
	else {
		pOgrSRS->exportToWkt(&prjInfo);
	}
	//3.  create new dataset
	WriteInfoLog("<-------------3. create new dataset ------------->");
	GDALDriver *poDriver = nullptr;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset *poNewDS = poDriver->Create(_tif_file_name.c_str(), rwidth, rheight, 1, GDT_Float32, nullptr);
	double adfGeoTransform[6] = { 0.0 };
	adfGeoTransform[0] = env.MinX;
	adfGeoTransform[1] = (env.MaxX - env.MinX) / rwidth;
	adfGeoTransform[2] = 0.0;
	adfGeoTransform[3] = env.MaxY;
	adfGeoTransform[4] = 0.0;
	adfGeoTransform[5] = (env.MinY - env.MaxY) / rheight;
	GDALSetGeoTransform(poNewDS, adfGeoTransform);
	GDALSetRasterNoDataValue(GDALGetRasterBand(poNewDS, 1), -9999);

	if (prjInfo != nullptr)
	{
		poNewDS->SetProjection(prjInfo);
	}
	else
	{
		errMsg = "error: no projection info specified for " + _shp_file_name;
		WriteInfoLog(errMsg);
	}
	//4. set parameter for transform
	WriteInfoLog("<-------------4. set parameter for transform ------------->");
	int band = 1;
	double burnvalue = 1;
	char **papszOptions = nullptr;
	//papszOptions = CSLSetNameValue(papszOptions, "CHUNKSIZE", "1");
	if (elvationField)
		papszOptions = CSLSetNameValue(papszOptions, "ATTRIBUTE", elvationField);

	void * pTransformArg = nullptr;
	void * m_hGenTransformArg = nullptr;
	m_hGenTransformArg = GDALCreateGenImgProjTransformer(nullptr, prjInfo, (GDALDatasetH)poNewDS, poNewDS->GetProjectionRef(), false, 1.0, 0);
	pTransformArg = GDALCreateApproxTransformer(GDALGenImgProjTransform, m_hGenTransformArg, 0.125);
	//5. transform
	WriteInfoLog("<-------------5. begin rasterize   ------------->");
	OGRLayerH dstLayer = (OGRLayerH)layer;
	CPLErr err = GDALRasterizeLayers((GDALDatasetH)poNewDS, 1, &band, 1, &dstLayer,
		GDALGenImgProjTransform,
		m_hGenTransformArg,
		&burnvalue,
		papszOptions,
		GDALTermProgress,
		nullptr);

	GDALDestroyGenImgProjTransformer(m_hGenTransformArg);
	GDALDestroyApproxTransformer(pTransformArg);
	GDALClose(poNewDS);
	GDALClose(poDS);

	double t = m_timer.stop();
	WriteInfoLog("<-------------convert from shp to tif rasterize time  : ------------->" + std::to_string(t));

	return errMsg;
}

std::string  GDALFormatConvert::Shape2Tiff_Rasterize(std::string _shp_file_name, std::string _tif_file_name, double pixelWidth, double pixelHeight, char * elvationField /*= nullptr*/) {
	WriteInfoLog("<-------------begin convert shape file to dem tif file with pixel specified------------->");

	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	//1.  read shape file
	WriteInfoLog("<-------------1. read shape file ------------->");
	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	GDALDataset *pShpDataSet = GDALDataset::FromHandle(poDS);
	//2.  get layers and extract info
	WriteInfoLog("<-------------2. extract info from layer ------------->");
	OGRLayer *layer = pShpDataSet->GetLayer(0);
	if (layer == nullptr) {
		errMsg = "error: error happen when trying to get layer in  " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	// get layer's extent
	OGREnvelope env;
	layer->GetExtent(&env);
	// get projection info
	OGRSpatialReference *pOgrSRS = nullptr;
	pOgrSRS = layer->GetSpatialRef();

	int rwidth = std::abs(int((env.MaxX - env.MinX) / pixelWidth));
	int rheight = std::abs(int((env.MaxY - env.MinY) / pixelHeight));
	if (std::abs(rwidth) > 0 && std::abs(rheight) > 0) {
		WriteInfoLog("<-------------2.1 height and width user specified ------------->");
	}
	else {
		errMsg = "error: do not have width and height specified";
		WriteInfoLog(errMsg);
		return errMsg;
	}

	// get projection info
	char *prjInfo = nullptr;
	if (pOgrSRS == nullptr) {
		errMsg = "error: can not get projection from " + _shp_file_name;
		WriteInfoLog(errMsg);
	}
	else {
		pOgrSRS->exportToWkt(&prjInfo);
	}
	//3.  create new dataset
	WriteInfoLog("<-------------3. create new dataset ------------->");
	GDALDriver *poDriver = nullptr;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset *poNewDS = poDriver->Create(_tif_file_name.c_str(), rwidth, rheight, 1, GDT_Float32, nullptr);
	double adfGeoTransform[6] = { 0.0 };
	adfGeoTransform[0] = env.MinX;
	adfGeoTransform[1] = (env.MaxX - env.MinX) / rwidth;
	adfGeoTransform[2] = 0.0;
	adfGeoTransform[3] = env.MaxY;
	adfGeoTransform[4] = 0.0;
	adfGeoTransform[5] = (env.MinY - env.MaxY) / rheight;
	GDALSetGeoTransform(poNewDS, adfGeoTransform);
	GDALSetRasterNoDataValue(GDALGetRasterBand(poNewDS, 1), -9999);

	if (prjInfo != nullptr)
	{
		poNewDS->SetProjection(prjInfo);
	}
	else
	{
		errMsg = "error: no projection info specified for " + _shp_file_name;
		WriteInfoLog(errMsg);
	}
	//4. set parameter for transform
	WriteInfoLog("<-------------4. set parameter for transform ------------->");
	int band = 1;
	double burnvalue = 1;
	char **papszOptions = nullptr;
	//papszOptions = CSLSetNameValue(papszOptions, "CHUNKSIZE", "1");
	if (elvationField)
		papszOptions = CSLSetNameValue(papszOptions, "ATTRIBUTE", elvationField);

	void * pTransformArg = nullptr;
	void * m_hGenTransformArg = nullptr;
	m_hGenTransformArg = GDALCreateGenImgProjTransformer(nullptr, prjInfo, (GDALDatasetH)poNewDS, poNewDS->GetProjectionRef(), false, 1.0, 0);
	pTransformArg = GDALCreateApproxTransformer(GDALGenImgProjTransform, m_hGenTransformArg, 0.125);
	//5. transform
	WriteInfoLog("<-------------5. begin rasterize   ------------->");
	OGRLayerH dstLayer = (OGRLayerH)layer;
	CPLErr err = GDALRasterizeLayers((GDALDatasetH)poNewDS, 1, &band, 1, &dstLayer,
		GDALGenImgProjTransform,
		m_hGenTransformArg,
		&burnvalue,
		papszOptions,
		GDALTermProgress,
		nullptr);

	GDALDestroyGenImgProjTransformer(m_hGenTransformArg);
	GDALDestroyApproxTransformer(pTransformArg);
	GDALClose(poNewDS);
	GDALClose(poDS);

	double t = m_timer.stop();
	WriteInfoLog("<-------------convert from shp to tif rasterize time  : ------------->" + std::to_string(t));

	return errMsg;
}

//////////////////////////////////////////////////////////////////////////
std::string GDALFormatConvert::Shape2Tiff_Interpolation(std::string _shp_file_name, std::string _tif_file_name, int dstWidth, int dstHeight, const char * elvationField /*= nullptr*/, int algo /*= 5*/) {

	WriteInfoLog("<-------------begin convert shape file to dem tif file using interpolation with width and height specified------------->");

	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	//1.  read shape file
	WriteInfoLog("<-------------1. read shape file ------------->");
	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	//2.  construct options for convert 
	WriteInfoLog("<-------------2. construct options for convert ------------->");
	std::string w_str = std::to_string(dstWidth);
	std::string h_str = std::to_string(dstHeight);
	char *xsize = const_cast<char*>(w_str.c_str());
	char *ysize = const_cast<char*>(h_str.c_str());

	// get layer's projection 
	GDALDataset *pShpDataSet = GDALDataset::FromHandle(poDS);
	OGRLayer *layer = pShpDataSet->GetLayer(0);
	OGRSpatialReference *orf = layer->GetSpatialRef();
	char * dstProjection = nullptr;
	if (orf)
		orf->exportToWkt(&dstProjection);

	//////////////////////////////////////////////////////////////////////////
	char **papszArgv = new char *[16];
	for (int i = 0; i < 16; ++i)
	{
		papszArgv[i] = new char[32];
	}

	{
		const char *tmp = "-ot";
		int i = 0;
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "Float32";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "-of";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "GTiff";
		memcpy(papszArgv[i++], tmp, 32);
		if (elvationField) {
			tmp = "-zfield";
			memcpy(papszArgv[i++], tmp, 32);
			memcpy(papszArgv[i++], elvationField, 32);
		}
		if (dstProjection) {
			tmp = "-a_srs";
			memcpy(papszArgv[i++], tmp, 32);
			delete papszArgv[i];
			papszArgv[i] = new char[strlen(dstProjection) + 1];
			memcpy(papszArgv[i++], dstProjection, strlen(dstProjection));
		}
		tmp = "-outsize";
		memcpy(papszArgv[i++], tmp, 32);
		memcpy(papszArgv[i++], xsize, 32);
		memcpy(papszArgv[i++], ysize, 32);
		tmp = "-a";
		memcpy(papszArgv[i++], tmp, 32);
		if (5 == algo)
			tmp = "nearest:nodata=-9999";
		if (4 == algo)
			tmp = "linear:nodata=-9999";
		if (3 == algo)
			tmp = "average:nodata=-9999";
		if (2 == algo)
			tmp = "invdist:nodata=-9999";
		if (1 == algo)
			tmp = "invdistnn:nodata=-9999";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "-co";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "NUM_THREADS=ALL_CPUS";
		memcpy(papszArgv[i++], tmp, 32);
		for (; i < 16; ++i)
		{
			delete papszArgv[i];
			papszArgv[i] = nullptr;
		}
	}

	GDALGridOptions *psOptions = GDALGridOptionsNew(papszArgv, nullptr);
	int bUsageError = FALSE;
	//3.  begin converting interpolation
	WriteInfoLog("<-------------3.  begin converting interpolation ------------->");
	GDALDatasetH hOutDS = GDALGrid(_tif_file_name.c_str(), poDS, psOptions, &bUsageError);
	GDALClose(poDS);
	GDALClose(hOutDS);
	GDALGridOptionsFree(psOptions);

	/*
	static const char szAlgNameInvDist[] = "invdist";
	static const char szAlgNameInvDistNearestNeighbor[] = "invdistnn";
	static const char szAlgNameAverage[] = "average";
	static const char szAlgNameNearest[] = "nearest";
	static const char szAlgNameMinimum[] = "minimum";
	static const char szAlgNameMaximum[] = "maximum";
	static const char szAlgNameRange[] = "range";
	static const char szAlgNameCount[] = "count";
	static const char szAlgNameAverageDistance[] = "average_distance";
	static const char szAlgNameAverageDistancePts[] = "average_distance_pts";
	static const char szAlgNameLinear[] = "linear";
	*/

	delete[]papszArgv;

	double t = m_timer.stop();
	WriteInfoLog("<-------------convert from shp to tif interpolation time  : ------------->" + std::to_string(t));
	return errMsg;
}

std::string GDALFormatConvert::Shape2Tiff_Interpolation(std::string _shp_file_name, std::string _tif_file_name, double pixelWidth, double pixelHeight, const char * elvationField /*= nullptr*/, int algo /*= 5*/) {
	WriteInfoLog("<-------------begin convert shape file to dem tif file using interpolation with pixel width and pixel height specified------------->");

	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	//1.  read shape file
	WriteInfoLog("<-------------1. read shape file ------------->");
	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	//2.  construct options for convertion
	WriteInfoLog("<-------------2. construct options for convertion ------------->");
	GDALDataset *pShpDataSet = GDALDataset::FromHandle(poDS);
	OGRLayer *layer = pShpDataSet->GetLayer(0);
	if (!layer) {
		errMsg = "error: can not extract layer from :" + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	// get layer's projection 
	OGRSpatialReference *orf = layer->GetSpatialRef();
	char * dstProjection = nullptr;
	if (orf)
		orf->exportToWkt(&dstProjection);

	// get layer's extent
	OGREnvelope env;
	layer->GetExtent(&env);
	int dstWidth = std::abs(int((env.MaxX - env.MinX) / pixelWidth));
	int dstHeight = std::abs(int((env.MaxY - env.MinY) / pixelHeight));

	//////////////////////////////////////////////////////////////////////////
	std::string w_str = std::to_string(dstWidth);
	std::string h_str = std::to_string(dstHeight);
	char *xsize = const_cast<char*>(w_str.c_str());
	char *ysize = const_cast<char*>(h_str.c_str());
	char **papszArgv = new char *[16];
	for (int i = 0; i < 16; ++i)
	{
		papszArgv[i] = new char[32];
	}

	{
		const char *tmp = "-ot";
		int i = 0;
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "Float32";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "-of";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "GTiff";
		memcpy(papszArgv[i++], tmp, 32);
		if (elvationField) {
			tmp = "-zfield";
			memcpy(papszArgv[i++], tmp, 32);
			memcpy(papszArgv[i++], elvationField, 32);
		}
		if (dstProjection) {
			tmp = "-a_srs";
			memcpy(papszArgv[i++], tmp, 32);
			delete papszArgv[i];
			papszArgv[i] = new char[strlen(dstProjection) + 1];
			memcpy(papszArgv[i++], dstProjection, strlen(dstProjection));
		}
		tmp = "-outsize";
		memcpy(papszArgv[i++], tmp, 32);
		memcpy(papszArgv[i++], xsize, 32);
		memcpy(papszArgv[i++], ysize, 32);
		tmp = "-a";
		memcpy(papszArgv[i++], tmp, 32);
		//https://www.gdal.org/gdal_grid.html#gdal_grid_algorithms
		// "invdistnn" "invdist" "average"  "linear" "nearest" 
		if (5 == algo)
			tmp = "nearest:nodata=-9999";
		if (4 == algo)
			tmp = "linear:nodata=-9999";
		if (3 == algo)
			tmp = "average:nodata=-9999";
		if (2 == algo)
			tmp = "invdist:nodata=-9999";
		if (1 == algo)
			tmp = "invdistnn:nodata=-9999";

		memcpy(papszArgv[i++], tmp, 32);
		tmp = "-co";
		memcpy(papszArgv[i++], tmp, 32);
		tmp = "NUM_THREADS=ALL_CPUS";
		memcpy(papszArgv[i++], tmp, 32);
		for (; i < 16; ++i)
		{
			delete papszArgv[i];
			papszArgv[i] = nullptr;
		}
	}

	GDALGridOptions *psOptions = GDALGridOptionsNew(papszArgv, nullptr);
	int bUsageError = FALSE;
	//3.  begin converting interpolation
	WriteInfoLog("<-------------3.  begin converting interpolation ------------->");
	GDALDatasetH hOutDS = GDALGrid(_tif_file_name.c_str(), poDS, psOptions, &bUsageError);
	GDALClose(poDS);
	GDALClose(hOutDS);
	GDALGridOptionsFree(psOptions);

	delete[]papszArgv;

	double t = m_timer.stop();
	WriteInfoLog("<-------------convert from shp to tif interpolation time  : ------------->" + std::to_string(t));
	return errMsg;
}

//////////////////////////////////////////////////////////////////////////

std::string GDALFormatConvert::GenerateContourFromDemWriteAsShpFile(std::string _dem_file_name, std::string _contour_file_name, double dContourInterval) {

	WriteInfoLog("<-------------begin generating contour from dem tif file ------------->");
	m_timer.start();

	std::string errMsg;
	GDALAllRegister();
	OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// 1. open dem data 
	WriteInfoLog("<-------------open dem data ------------->");
	GDALDatasetH hSrcDS = GDALOpen(_dem_file_name.c_str(), GA_ReadOnly);
	if (hSrcDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _dem_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	GDALRasterBandH hBand = GDALGetRasterBand(hSrcDS, 1);
	// 2. get nodata value
	WriteInfoLog("<------------- get nodata value ------------->");
	int bNoDataSet = 0;
	double dfNoData = GDALGetRasterNoDataValue(hBand, &bNoDataSet);
	// 3. get projection info
	WriteInfoLog("<------------- get projection info ------------->");
	OGRSpatialReferenceH hSRS = nullptr;
	const char *pszWKT = GDALGetProjectionRef(hSrcDS);
	if (pszWKT != NULL && strlen(pszWKT) != 0) {
		hSRS = OSRNewSpatialReference(pszWKT);
	}
	else
		errMsg = "warning : have no projection info ";
	// 4. create layer
	const char *pszFormat = "ESRI Shapefile";
	OGRSFDriverH hDriver = OGRGetDriverByName(pszFormat);
	OGRDataSourceH hDstDS = OGR_Dr_CreateDataSource(hDriver, _contour_file_name.c_str(), nullptr);
	OGRLayerH hLayer = OGR_DS_CreateLayer(hDstDS, "Contour", hSRS, wkbLineString, nullptr);
	// 5. create field
	WriteInfoLog("<------------- create field ------------->");
	OGRFieldDefnH hFld = OGR_Fld_Create("ID", OFTInteger);
	OGR_Fld_SetWidth(hFld, 8);
	OGR_L_CreateField(hLayer, hFld, FALSE);
	OGR_Fld_Destroy(hFld);
	int nIDField = 0;
	// 6. create geometry
	WriteInfoLog("<------------- create geometry ------------->");
	hFld = OGR_Fld_Create("elevation", OFTReal);
	OGR_Fld_SetWidth(hFld, 12);
	OGR_Fld_SetPrecision(hFld, 5);
	OGR_L_CreateField(hLayer, hFld, FALSE);
	OGR_Fld_Destroy(hFld);
	int nElevField = 1;
	// 7. GDALContourGenerate
	WriteInfoLog("<------------- GDALContourGenerate ------------->");
	CPLErr eErr = GDALContourGenerate(hBand, dContourInterval, 0.0, 0, nullptr, bNoDataSet, dfNoData, hLayer, nIDField,
		nElevField, nullptr, nullptr);

	GDALClose(hSrcDS);
	OGR_DS_Destroy(hDstDS);
	GDALDestroyDriverManager();

	double t = m_timer.stop();
	WriteInfoLog("<-------------end generating contour from dem tif file time: " + std::to_string(t) + "------------->");
	if (eErr != CE_None)
	{
		errMsg = "error happed when generating contour from dem file ";
	}

	return errMsg;
}

std::string GDALFormatConvert::GenerateContourFromDemWriteAsMapFile(std::string _dem_file_name, std::string _map_file_name, double dContourInterval) {

	WriteInfoLog("-------- begin generate contour from dem file --------");
	//1. make shp file name
	std::string shp_file_name = _map_file_name + ".shp";
	//2. generate shp file
	WriteInfoLog("-------- generate contour to shp file --------");
	std::string errMsg = GenerateContourFromDemWriteAsShpFile(_dem_file_name, shp_file_name, dContourInterval);
	if (!errMsg.empty())
		return errMsg;
	//3. generate map file
	WriteInfoLog("-------- convert shp file to map file --------");
	errMsg = Shape2WAsPMap(shp_file_name, _map_file_name);

	WriteInfoLog("-------- end generate contour from dem file --------" + errMsg);

	return errMsg;
}

std::string GDALFormatConvert::GenerateContourFromDemWriteAsGeoJSONFile(std::string _dem_file_name, std::string _geojson_file_name, double dContourInterval) {

	WriteInfoLog("-------- begin generate contour from dem file --------");
	//1. make shp file name
	std::string shp_file_name = _geojson_file_name + ".shp";
	//2. generate shp file
	WriteInfoLog("-------- generate contour to shp file --------");
	std::string errMsg = GenerateContourFromDemWriteAsShpFile(_dem_file_name, shp_file_name, dContourInterval);
	if (!errMsg.empty())
		return errMsg;
	//3. generate geojson file
	WriteInfoLog("-------- convert shp file to geojson file --------");
	errMsg = Shape2GeoJSON(shp_file_name, _geojson_file_name);

	WriteInfoLog("-------- end generate contour from dem file --------" + errMsg);

	return errMsg;
}

//////////////////////////////////////////////////////////////////////////
std::string  GDALFormatConvert::ExtractShapeFileBoundary(std::string _shp_file_name, std::string _out_shp_name, double pixel /*= 15*/, double thresh_hold /*= 140.0*/, double thresh_area/* = -1.0*/) {
	WriteInfoLog("<-------------begin extract shape file boundary ------------->");
	double cp = pixel;
	double th = thresh_hold;
	m_timer.start();
	std::string errMsg;
	GDALAllRegister();
	OGRRegisterAll();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	//1.  read shape file
	WriteInfoLog("<-------------1. read shape file ------------->");
	GDALDatasetH poDS = GDALOpenEx(_shp_file_name.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
	if (poDS == nullptr)
	{
		errMsg = "error: error happen when open file " + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	//2.  get pixel data from shape file
	WriteInfoLog("<-------------2. construct options for convertion ------------->");
	GDALDataset *pShpDataSet = GDALDataset::FromHandle(poDS);
	OGRLayer *layer = pShpDataSet->GetLayer(0);
	if (!layer) {
		errMsg = "error: can not extract layer from :" + _shp_file_name;
		WriteInfoLog(errMsg);
		return errMsg;
	}
	// get layer's extent
	OGREnvelope env;
	layer->GetExtent(&env);
	// get layer's projection 
	OGRSpatialReference *orf = layer->GetSpatialRef();
	char * dstProjection = nullptr;
	if (orf) {
		orf->exportToWkt(&dstProjection);
		if (orf->IsGeographic()) {
			cp = cp * 2.7777777777777057e-04 / 30.0;
			th = th * 2.7777777777777057e-04 / 30.0;
		}
	}
	else {
		cp = std::abs((env.MaxX - env.MinX) / 1000);
		th = cp * 10;
	}

	int rwidth = int((env.MaxX - env.MinX) / cp) + 2;
	int rheight = int((env.MaxY - env.MinY) / cp) + 2;

	//////////////////////////////////////////////////////////////////////////
	// 3. rasterize input layer
	WriteInfoLog("<-------------3. rasterized input layer ------------->");

	GDALDriver *poDriver = nullptr;
	poDriver = GetGDALDriverManager()->GetDriverByName("MEM");
	GDALDataset *poNewDS = poDriver->Create("rasterized.tif", rwidth, rheight, 1, GDT_Byte, nullptr);
	double adfGeoTransform[6] = { 0.0 };
	adfGeoTransform[0] = env.MinX;
	adfGeoTransform[1] = cp;
	adfGeoTransform[2] = 0.0;
	adfGeoTransform[3] = env.MaxY;
	adfGeoTransform[4] = 0.0;
	adfGeoTransform[5] = -cp;
	GDALSetGeoTransform(poNewDS, adfGeoTransform);
	GDALSetRasterNoDataValue(GDALGetRasterBand(poNewDS, 1), 0);

	if (dstProjection != nullptr)
	{
		poNewDS->SetProjection(dstProjection);
	}
	else
	{
		errMsg = "error: no projection info specified for " + _shp_file_name;
		WriteInfoLog(errMsg);
	}

	int band = 1;
	double burnvalue = 1;
	char **papszOptions = nullptr;
	papszOptions = CSLSetNameValue(papszOptions, "ATTRIBUTE", "elevation");

	void * pTransformArg = nullptr;
	void * m_hGenTransformArg = nullptr;
	m_hGenTransformArg = GDALCreateGenImgProjTransformer(nullptr, dstProjection, (GDALDatasetH)poNewDS, poNewDS->GetProjectionRef(), false, 1.0, 0);
	pTransformArg = GDALCreateApproxTransformer(GDALGenImgProjTransform, m_hGenTransformArg, 0.125);
	OGRLayerH dstLayer = (OGRLayerH)layer;
	CPLErr err = GDALRasterizeLayers((GDALDatasetH)poNewDS, 1, &band, 1, &dstLayer,
		GDALGenImgProjTransform,
		m_hGenTransformArg,
		&burnvalue,
		papszOptions,
		GDALTermProgress,
		nullptr);

	GDALDestroyGenImgProjTransformer(m_hGenTransformArg);
	GDALDestroyApproxTransformer(pTransformArg);
	//////////////////////////////////////////////////////////////////////////
	// 4. set raster layer data
	int thresh_hold_pixel = 1 + th / cp;
	GDALRasterBandH bandData = GDALGetRasterBand(poNewDS, 1);
	std::vector<unsigned char> band_array(rwidth*rheight, 0);
	GDALRasterIO(bandData, GF_Read, 0, 0, rwidth, rheight, &band_array[0], rwidth, rheight, GDT_Byte, 0, 0);
	for (int r = 0; r < rheight; ++r)
	{
		bool start = false;
		int start_pos = 0;
		for (int c = 0; c < rwidth; ++c)
		{
			if (band_array[r*rwidth + c] > 0) {
				if (!start) {
					start = true;
					start_pos = c;
				}
				else {
					if ((c - start_pos) < thresh_hold_pixel) {
						for (int k = start_pos; k < c; ++k)
						{
							band_array[r*rwidth + k] = 1;
						}
					}
					start_pos = c;
				}
			}
		}
	}
	GDALRasterIO(bandData, GF_Write, 0, 0, rwidth, rheight, &band_array[0], rwidth, rheight, GDT_Byte, 0, 0);
	((GDALRasterBand*)bandData)->FlushCache();

	//////////////////////////////////////////////////////////////////////////
	for (int c = 0; c < rwidth; ++c)
	{
		bool start = false;
		int start_pos = 0;
		for (int r = 0; r < rheight; ++r)
		{
			if (band_array[r*rwidth + c] > 0) {
				if (!start) {
					start = true;
					start_pos = r;
				}
				else {
					if ((r - start_pos) < thresh_hold_pixel) {
						for (int k = start_pos; k < r; ++k)
						{
							band_array[k*rwidth + c] = 1;
						}
					}
					start_pos = r;
				}
			}
		}
	}
	GDALRasterIO(bandData, GF_Write, 0, 0, rwidth, rheight, &band_array[0], rwidth, rheight, GDT_Byte, 0, 0);
	((GDALRasterBand*)bandData)->FlushCache();
	//////////////////////////////////////////////////////////////////////////
	// 4. polygonize raster
	WriteInfoLog("<-------------3. polygonize input raster ------------->");
	GDALDriver* poOGRDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	GDALDataset  *poOGRNewDS = poOGRDriver->Create(_out_shp_name.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	OGRSpatialReference oSRS;
	oSRS.SetFromUserInput(dstProjection);
	OGRLayer *poLayer = poOGRNewDS->CreateLayer(_out_shp_name.c_str(), &oSRS);
	if (poLayer == nullptr)
	{
		errMsg = "error: Creat layer failed! " + _shp_file_name;
		WriteInfoLog(errMsg);

		GDALClose(poNewDS);
		GDALClose(poOGRNewDS);
		std::vector<unsigned char>().swap(band_array);

		return errMsg;
	}
	OGRFieldDefn oField("elevation", OFTInteger);
	if (poLayer->CreateField(&oField) != OGRERR_NONE)
	{
		errMsg = "error: Create elevation Field Failed! " + _shp_file_name;
		WriteInfoLog(errMsg);

		GDALClose(poNewDS);
		GDALClose(poOGRNewDS);
		std::vector<unsigned char>().swap(band_array);

		return errMsg;
	}

	if (GDALPolygonize(bandData, nullptr, (OGRLayerH)poLayer, 0, nullptr, nullptr, nullptr) != CE_None)
	{
		errMsg = "error: GDALPolygonize Failed! " + _shp_file_name;
		WriteInfoLog(errMsg);

		GDALClose(poNewDS);
		GDALClose(poOGRNewDS);
		std::vector<unsigned char>().swap(band_array);

		return errMsg;
	}
	// filter by area or input value
	double filter_area = thresh_area < 0 ? th * th : thresh_area;
	int nbFeatures = poLayer->GetFeatureCount();
	for (int nf = 0; nf < nbFeatures; ++nf)
	{
		OGRFeature *poFeature = poLayer->GetFeature(nf);
		if (0 == poFeature->GetFieldAsInteger("elevation")) {
			poLayer->DeleteFeature(poFeature->GetFID());
		}
		OGRGeometry *geom = poFeature->GetGeometryRef();
		OGRGeometryH geomHandle = OGRGeometry::ToHandle(geom);
		double area = OGR_G_Area(geomHandle);
		if (area < filter_area) {
			poLayer->DeleteFeature(poFeature->GetFID());
			continue;
		}
		//////////////////////////////////////////////////////////////////////////
		int geomCnt = OGR_G_GetGeometryCount(geomHandle);
		if (geomCnt > 1)
		{
			OGRwkbGeometryType geo_type = geom->getGeometryType();
			OGRGeometry *new_geom = OGRGeometryFactory::createGeometry(geo_type);
			OGRGeometryH new_geomHandle = OGRGeometry::ToHandle(new_geom);

			for (int gc = 0; gc < geomCnt; ++gc)
			{
				OGRGeometryH curG = OGR_G_GetGeometryRef(geomHandle, gc);
				double area = OGR_G_Area(curG);
				if (area > filter_area) {
					OGRErr err = OGR_G_AddGeometry(new_geomHandle, curG);
					if (err != OGRERR_NONE) {
						continue;
					}
				}
			}
			int newCnt = OGR_G_GetGeometryCount(new_geomHandle);
			OGRErr err = poFeature->SetGeometryDirectly(new_geom);
		}
		OGRErr err = OGR_L_SetFeature(OGRLayerH(poLayer), OGRFeatureH(poFeature));
	}
	poLayer->SyncToDisk();
	//////////////////////////////////////////////////////////////////////////
	// simplify polygon
	nbFeatures = poLayer->GetFeatureCount();
	for (int nf = 0; nf < nbFeatures; ++nf)
	{
		OGRFeature *poFeature = poLayer->GetFeature(nf);
		OGRGeometry *geom = poFeature->GetGeometryRef();
		OGRGeometryH geomHandle = OGRGeometry::ToHandle(geom);
		//////////////////////////////////////////////////////////////////////////
		OGRGeometry *new_geom = geom->SimplifyPreserveTopology(20.0);
		OGRErr err = poFeature->SetGeometryDirectly(new_geom);
		err = OGR_L_SetFeature(OGRLayerH(poLayer), OGRFeatureH(poFeature));
	}
	poLayer->SyncToDisk();
	//////////////////////////////////////////////////////////////////////////
	GDALClose(poNewDS);
	GDALClose(poOGRNewDS);
	std::vector<unsigned char>().swap(band_array);

	double t = m_timer.stop();
	WriteInfoLog("<-------------end extract shape file boundary: " + std::to_string(t) + "------------->");

	return errMsg;
}
