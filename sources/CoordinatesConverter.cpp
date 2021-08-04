#include "CoordinatesConverter.h"
#include<qdebug.h>

CoordinatesConverter::CoordinatesConverter()
{
	transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
}

CoordinatesConverter::CoordinatesConverter(vtkSmartPointer<vtkMatrix4x4> _transformMatrix)
{
	transformMatrix = _transformMatrix;
}

CoordinatesConverter::CoordinatesConverter(DicomHandler::dicomDataProperties& _properties)
{
	transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	float zdir[3];
	vtkMath::Cross(_properties.imageOrientationX, _properties.imageOrientationY, zdir);

	for (int i = 0; i < 3; i++)
	{
		transformMatrix->Element[i][0] = _properties.imageOrientationX[i] * _properties.pixelSpacing[1];
		transformMatrix->Element[i][1] = _properties.imageOrientationY[i] * _properties.pixelSpacing[0];
		transformMatrix->Element[i][2] = zdir[i] * _properties.pixelSpacing[2];// * 
		transformMatrix->Element[i][3] = _properties.imagePosition[i];
	}
}

CoordinatesConverter::CoordinatesConverter(TransformationMatrixFeatures _transMatrixFeat)
{
	transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	float zdir[3];
	vtkMath::Cross(_transMatrixFeat.orientationX, _transMatrixFeat.orientationY, zdir);

	for (int i = 0; i < 3; i++)
	{
		transformMatrix->Element[i][0] = _transMatrixFeat.orientationX[i];
		transformMatrix->Element[i][1] = _transMatrixFeat.orientationY[i];
		transformMatrix->Element[i][2] = zdir[i];
		transformMatrix->Element[i][3] = _transMatrixFeat.translation[i];
	}
	transformMatrix->Element[0][0] = transformMatrix->Element[0][0] * _transMatrixFeat.scale[0];
	transformMatrix->Element[1][1] = transformMatrix->Element[1][1]* _transMatrixFeat.scale[1];
	transformMatrix->Element[2][2] = transformMatrix->Element[2][2]* _transMatrixFeat.scale[2];
}

CoordinatesConverter::~CoordinatesConverter()
{
}

CoordinatesConverter CoordinatesConverter::operator*(CoordinatesConverter& _right) const
{
	vtkSmartPointer<vtkMatrix4x4> ans = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkMatrix4x4::Multiply4x4(	this->transformMatrix,_right.transformMatrix, ans);
	return CoordinatesConverter(ans);
}

//transforms coordinates according to transformMatrix.
float* CoordinatesConverter:: transform(float x, float y, float z)
{	
	float v[4] = { x, y ,z , 1 };
	float* out = new float[3];
	float* ptr = transformMatrix.GetPointer()->MultiplyPoint(v);

	for (int i = 0; i < 3; i++)
	{
		out[i]= ptr[i];
	}
	return out;
}

//transforms coordinates by applying translation first. 
float * CoordinatesConverter::transformTranslationFirst(float x, float y, float z)
{
	float* out = new float[3];
	float v[3] = { x, y ,z };
	vtkSmartPointer<vtkMatrix3x3> rotationMatrix = vtkSmartPointer<vtkMatrix3x3>::New();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			rotationMatrix->SetElement(i,j, transformMatrix->Element[i][j]);
		}
		v[i] += transformMatrix->Element[i][3];
	}
	rotationMatrix->MultiplyPoint(v, out);
	return out;
}

//gets boundary values for volume. 
void CoordinatesConverter::getCoordinateSystemProperties(DicomHandler::dicomDataProperties& _properties90, CoordinateSystemProperties* _properties)
{
	qDebug()<<"CoordinatesConverter::getCoordinateSystemProperties";

	float* worldCoord1 = new float[3];
	float* worldCoord2  =new float[3];

	//get values of corners.
	worldCoord1 = this->transform(0, 0);
	worldCoord2 = this->transform(_properties90.dimension[0] - 1, _properties90.dimension[1] - 1);

	
	CoordinatesConverter converter(_properties90);
	float* worldCoord3 = new float[3];
	float* worldCoord4 = new float[3];

	//get values of corners _properties90.
	worldCoord3= converter.transform(0, 0);
	worldCoord4= converter.transform(_properties90.dimension[0] - 1, _properties90.dimension[1] - 1);
	
	for (int i = 0; i < 3; i++)
	{
		float boundaries[4] = { worldCoord1[i], worldCoord2[i] ,worldCoord3[i] ,worldCoord4[i] };

		//get min and max values and origin. 
		_properties->maxExtend[i] = *std::max_element(boundaries,boundaries + 4);
		_properties->minExtend[i] = *std::min_element(boundaries,boundaries + 4);
		_properties->origin[i] = _properties->minExtend[i];
	}
}


void CoordinatesConverter::getTransMatrixFeatures(CoordinateSystemProperties _baseSystem, CoordinateSystemProperties _referenceSystem, TransformationMatrixFeatures* _transMatrixFeat)
{
	for (int i = 0; i < 3; i++)
	{
		_transMatrixFeat->translation[i] = _baseSystem.origin[i] - _referenceSystem.origin[i];
		_transMatrixFeat->scale[i] = (_baseSystem.maxExtend[i] - _baseSystem.minExtend[i]) / (_referenceSystem.maxExtend[i] - _referenceSystem.minExtend[i]);
	}
	
	_transMatrixFeat->orientationX[0] = 1;
	_transMatrixFeat->orientationX[1] = 0;
	_transMatrixFeat->orientationX[2] = 0;

	_transMatrixFeat->orientationY[0] = 0;
	_transMatrixFeat->orientationY[1] = 1;
	_transMatrixFeat->orientationY[2] = 0;
}

bool CoordinatesConverter::isNeedleXAxisAligned()
{
	//x axis downwards[0]     y-axis sidewards [1]
	qDebug() << "ele2"<<transformMatrix->Element[1][0];
	return transformMatrix->Element[1][0] == 0;//transformMatrix->GetElement()
}
