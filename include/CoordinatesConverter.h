#pragma once
#include <vtkSmartPointer.h>
#include <vtkPerspectiveTransform.h>
#include<vtkMath.h>
#include<DicomHandler.h>
#include<vtkMatrix3x3.h>

class CoordinatesConverter
{
public:
	struct CoordinateSystemProperties
	{
		float origin[3];
		float maxExtend[3];
		float minExtend[3];

	}; 
	struct TransformationMatrixFeatures
	{
		float orientationX[3];
		float orientationY[3];
		float translation[3];
		float scale[3];
		float theta[3];
	};
	CoordinatesConverter();
	CoordinatesConverter(vtkSmartPointer<vtkMatrix4x4> _transformMatrix);
	CoordinatesConverter(DicomHandler::dicomDataProperties& _properties);
	CoordinatesConverter(TransformationMatrixFeatures _transMatrixFeat);
	~CoordinatesConverter();
	
	CoordinatesConverter operator*(CoordinatesConverter& _right) const;
	//transforms a coordinate from one coordinate System in another
	float* transform(float x, float y, float z = 0);
	float* transformTranslationFirst(float x, float y, float z = 0);

	//extracts from the dicom header the coordinates system properties 
	void getCoordinateSystemProperties( DicomHandler::dicomDataProperties& _properties90, CoordinateSystemProperties* _properties);
	//computes Transformation Matrix between two coordinate systems
	void getTransMatrixFeatures(CoordinateSystemProperties _baseSystem, CoordinateSystemProperties _referenceSystem, TransformationMatrixFeatures* _transMatrixFeat);
	bool isNeedleXAxisAligned();
	vtkSmartPointer<vtkMatrix4x4> getTransformMatrix() { return transformMatrix; }
private:
	
	vtkSmartPointer<vtkMatrix4x4> transformMatrix;
};

