#pragma once
/*
*	Include QT header.
*/
#include <QDebug>
#include <QMainWindow>
#include <QFileDialog>
#include <QVTKWidget.h>
#include <QMainWindow>
#include <QFileDialog>
#include <QVTKWidget.h>
/*
*	Include VTK header.
*/
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkRenderWindow.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>

#include<vtkCubeSource.h>

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <vtkDICOMReader.h>
#include<vtkDICOMMetaData.h>
class DicomHandler
{
public:
	struct dicomDataProperties
	{
		float imagePosition[3];
		float imageOrientationX[3];
		float imageOrientationY[3];
		double pixelSpacing[3];
		float echoTime;
		float magneticFieldStrength;
		int dimension[2];
	};
	struct dicomData
	{
		dicomDataProperties headerData;
		vtkSmartPointer<vtkImageData> imageData;
	};
	DicomHandler();
	~DicomHandler();
	vtkSmartPointer<vtkImageData> loadDicom(QString q_c_dicomFilePath);
	void getImageDataProperties(vtkSmartPointer<vtkDICOMReader> _reader, dicomDataProperties* _properties);
	void getImageDataProperties(QString _fileDataName, dicomDataProperties* _properties);
};