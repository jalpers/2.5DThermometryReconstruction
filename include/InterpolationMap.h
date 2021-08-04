#pragma once

#ifndef INTERPOLATIONMAP_H
#define INTERPOLATIONMAP_H
#include<vtkImageData.h>
#include<CoordinatesConverter.h>
#include<DicomHandler.h>
#include <vtkImageData.h>
#include <vtkNIFTIImageReader.h>
#include<vtkNIFTIImageHeader.h>
#include<vtkMatrix4x4.h>
#include<DicomHandler.h>

#define VOLUMESIZE 60

class InterpolationMap
{
public:
	InterpolationMap();
	InterpolationMap(CoordinatesConverter _world2VolumeConverter);
	~InterpolationMap();
	//Loads seqmented tube diocm file und stores in look up table
	void loadVesselMap(QString _filename);
	
	// Creates a 2D lookup map for interpolating the volume on polar based coordinates
	void populateInterpolationMap(QString _fileName);

	typedef  struct PixelCoord
	{
		int x;
		int y;
		PixelCoord(int _x, int _y) : x(_x), y(_y) {}
		PixelCoord() : x(0), y(0) {}

		inline PixelCoord operator+(PixelCoord a) {
			return { a.x + x,a.y + y };
		}
		inline bool operator==(PixelCoord a) {
			if (a.x == x && a.y == y)
				return true;
			else
				return false;
		}
		inline bool operator!=(PixelCoord a) {
			if (a.x != x || a.y != y)
				return true;
			else
				return false;
		}
	};

	typedef struct PixelProperties
	{
		int radius;
		float angle;
		bool tobeInterpolated;
		PixelCoord interpolationPartner0;
		PixelCoord interpolationPartner1;
		float weight0;
		float weight1;
		PixelProperties() : radius(0), angle(0), tobeInterpolated(true),
			interpolationPartner0(PixelCoord(0, 0)), interpolationPartner1(PixelCoord(0, 0)),
			weight0(0), weight1(0) {}
		PixelProperties(int _l, float _a, bool _i, PixelCoord _partner0, PixelCoord _partner1, float _w0, float _w1) : radius(_l), angle(_a), tobeInterpolated(_i), interpolationPartner0(_partner0), interpolationPartner1(_partner1), weight0(_w0), weight1(_w1) {}
	};

	std::vector<std::vector<PixelProperties>> getinterpolationMap2D();
	vtkSmartPointer<vtkImageData> getVesselMap();
	
private:
	int volumeSize = VOLUMESIZE;
	void createPolarCoordinateMap();

	// sorts pxlMap
	struct HandlePxlMap {
		HandlePxlMap(std::vector<std::vector<PixelProperties>>_pxlMap) {
			PixelProperties P = PixelProperties();
			std::vector<PixelProperties> z;
			//Deep Copy
			z.resize(_pxlMap.size(), P);
			pxlMap_.resize(_pxlMap.at(0).size(), z);
			for (int i = 0; i < pxlMap_.size(); i++)
			{
				for (int j = 0; j < pxlMap_.at(0).size(); j++)
				{
					pxlMap_[i][j] = _pxlMap[i][j];
				}
			}
		}
		bool operator () (const PixelCoord lhs, const PixelCoord rhs) { return (pxlMap_[lhs.x][lhs.y].angle < pxlMap_[rhs.x][rhs.y].angle); }

		std::vector<std::vector<PixelProperties>>pxlMap_;
	};

	std::vector<std::vector<PixelProperties>>interpolationMap2D;

	float dotProduct(float vect_A[], float vect_B[]);
	float getAngle(float vect_A[], float vect_B[]);
	float l2_norm(float u[], int n = 2);
	float InterpolationMap::distanceCenter2Pixel(float _center[2], float _pixel2[2]);

	QStringList slices_angle = { "0","22_5","45","67_5","90","112_5","135","157_5" };
	QStringList slices_time = { "0","90","45","135_5","22_5","112_5","67_5","157_5" };
	
	std::vector<std::vector<PixelCoord>> radiusAngleLut;

	QString fileName;

	void setAllVoxel2Value(int _value);

	vtkSmartPointer<vtkImageData> vesselMap;

	CoordinatesConverter vesselMapVolume2World;
	CoordinatesConverter world2VolumeConverter;
};

#endif //INTERPOLATIONMAP_H