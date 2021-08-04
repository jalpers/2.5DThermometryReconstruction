#pragma once

#ifndef DATAVOLUME_H
#define DATAVOLUME_H

#include<InterpolationMap.h>
#include<vtkImageData.h>
#include<CoordinatesConverter.h>
#include<DicomHandler.h>

//vtk
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include<vtkNamedColors.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>

#include <vtkLookupTable.h>
#include <vtkPolyData.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkContourValues.h>
#include<qdebug.h>
#include<vector>
#include <deque>

#include <vtkImageMathematics.h>

#include<vtkOBJExporter.h>
#include<vtkDataSetMapper.h>
#include<vtkXMLImageDataWriter.h>
#include<vtkImageDataGeometryFilter.h>
#include<vtkPolyDataWriter.h>
#include<vtkImageDataGeometryFilter.h>
#include<vtkDICOMWriter.h>
#include <vtkDICOMMRGenerator.h>
#include <vtkImageData.h>
#include<vtkMatrix3x3.h>
#include<vtkImageResize.h>
#include<vtkImageMapper.h>

#define VOLUMESIZE 60
class DataVolume
{
public:
	DataVolume();
	/*
	* Constructor: 
	* Initialisates vtkSmartPointer<vtkImageData> volume 
	* calculates CoordinatesConverter world2VolumeConverter
	* @param _filename: directory of dicom files
	*/
	DataVolume(QString _filenName);
	DataVolume(vtkSmartPointer<vtkImageData> _imageData);
	~DataVolume();
	/*
	* writes the created volume to a stack of dicom files
	* @_targetfileName directory where files are saved
	*/
	void writeToFile(QString _targetfileName);
	/*
	* load and add 2D data to vtkSmartPointer<vtkImageData> volume
	* 
	* 
	*/
	void addSlice(QString _angle = "0", QString _timeStep = "0");
	void display(vtkSmartPointer<vtkRenderWindow> _q_vtk_renderWindow, bool _isCropped = true, bool _cameraAbove = false);
	/*
	* @param _interpolationMap2D: map of weights on a polar coordinates
	*/
	void interpolate(std::vector<std::vector<InterpolationMap::PixelProperties>> _interpolationMap2D, vtkSmartPointer<vtkImageData> _vesselMap);
	vtkSmartPointer<vtkImageData> getImageData();
	void crop(vtkSmartPointer<vtkImageData> volume, vtkSmartPointer<vtkImageData> croppedvolume, float percentage);
	void crop(float percentage);

	CoordinatesConverter getWorld2VoxelConverter() { return world2VolumeConverter; };
	
private:
	void init(QString _fileName);
	vtkSmartPointer<vtkImageData> volume;
	CoordinatesConverter world2VolumeConverter;
	int volumeSize = VOLUMESIZE;
	QString fileName;

	struct Slice
	{
		vtkSmartPointer<vtkImageData> data;
		CoordinatesConverter voxel2WorldConverter;
		int timestep = -1;
		Slice() : data(vtkSmartPointer<vtkImageData>::New()), voxel2WorldConverter(CoordinatesConverter()), timestep(-1) {}
		Slice(vtkSmartPointer<vtkImageData> _data,
			CoordinatesConverter _voxel2WorldConverter,
			int _timestep) : data(_data),
			voxel2WorldConverter(_voxel2WorldConverter),
			timestep(_timestep) {}
	};

	Slice slicesBuffer[8];

	int timestep;

	QStringList slices_angle = { "0","22_5","45","67_5","90","112_5","135","157_5" };
	QStringList slices_time = { "0","90","45","135_5","22_5","112_5","67_5","157_5" };

	void setAllVoxel2Value(int _value);
};
#endif // DATAVOLUME_H