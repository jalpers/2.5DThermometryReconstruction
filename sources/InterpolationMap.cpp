#include "InterpolationMap.h"
#include<vtkStringArray.h>
#include<vtkImageExtractComponents.h>

InterpolationMap::InterpolationMap()
{
	vesselMap = vtkSmartPointer<vtkImageData>::New();

	vesselMap->SetDimensions(volumeSize, volumeSize, volumeSize);
	vesselMap->SetOrigin(0, 0, 0);

	vesselMap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	setAllVoxel2Value(0);
}

InterpolationMap::InterpolationMap(CoordinatesConverter _world2VolumeConverter)
{
	vesselMap = vtkSmartPointer<vtkImageData>::New();

	vesselMap->SetDimensions(volumeSize, volumeSize, volumeSize);
	vesselMap->SetOrigin(0, 0, 0);

	vesselMap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

	world2VolumeConverter = _world2VolumeConverter;
	setAllVoxel2Value(0);
}

InterpolationMap::~InterpolationMap()
{
}

void InterpolationMap::loadVesselMap(QString _filename)
{
	vtkSmartPointer <vtkDICOMReader> reader =
		vtkSmartPointer <vtkDICOMReader>::New();
	// Provide a multi-frame, multi-stack file
	reader->SetFileName(_filename.toStdString().c_str());
	// Read the meta data, get a list of stacks
	reader->UpdateInformation();

	DicomHandler dHandler;
	DicomHandler::dicomDataProperties properties;
	dHandler.getImageDataProperties(reader, &properties);

	properties.pixelSpacing[2] = 1.1;
	vesselMapVolume2World = CoordinatesConverter(properties);

	//https://dgobbi.github.io/vtk-dicom/doc/api/attributes.html
	// Get the arrays that map slice to file and frame.
	vtkIntArray *fileMap = reader->GetFileIndexArray();
	vtkIntArray *frameMap = reader->GetFrameIndexArray();
	// Get the image data and meta data.
	vtkImageData *image = reader->GetOutput();
	vtkDICOMMetaData *meta = reader->GetMetaData();
	// Get the number of components in the data.
	int numComponents = image->GetNumberOfScalarComponents();

	// Get the full vector dimension for the DICOM data.
	int vectorDimension = fileMap->GetNumberOfComponents();

	// Compute the samples per pixel in original files.
	int samplesPerPixel = numComponents / vectorDimension;
	samplesPerPixel = 1;

	// Check for time dimension
	int timeDimension = reader->GetTimeDimension();
	//if (timeDimension == 0)
	{
		timeDimension = 1;
	}
	
	 //Extract an image at the desired time slot (e.g. for display).
	int componentIndex =  1* vectorDimension / timeDimension * samplesPerPixel;
	for (int z = 0; z < 100; z++)
	{
		componentIndex = z;
		//qDebug() << "componentIndex" << componentIndex;
		vtkNew<vtkImageExtractComponents> extractor;
		extractor->SetInputConnection(reader->GetOutputPort());
		if (samplesPerPixel == 1)
		{
			extractor->SetComponents(componentIndex);
		}
		extractor->Update();

		float* voxelVolumePosition = new float[3];
		float* voxelWorldPosition = new float[3]; 

		vtkSmartPointer<vtkImageData> tmp = extractor->GetOutput();
		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
			{
				uchar* tmpPxl= static_cast<uchar*>(tmp->GetScalarPointer(x, y, 0));
				voxelWorldPosition = vesselMapVolume2World.transform(x, 256-1-y, z);
				voxelVolumePosition = world2VolumeConverter.transformTranslationFirst(voxelWorldPosition[0], voxelWorldPosition[1], voxelWorldPosition[2]);

				if (voxelVolumePosition[0] >= 0 && voxelVolumePosition[0] < volumeSize
					&& voxelVolumePosition[1] >= 0 && voxelVolumePosition[1] < volumeSize
					&& voxelVolumePosition[2] >= 0 && voxelVolumePosition[2] < volumeSize)
				{
					uchar* voxelVolume = static_cast<uchar*>(vesselMap->GetScalarPointer((int)(voxelVolumePosition[0]), (int)(voxelVolumePosition[1]), (int)(voxelVolumePosition[2])));
					voxelVolume[0] = tmpPxl[0];
				}
			}
		}
	}
	vesselMap->Modified();
}

void InterpolationMap::createPolarCoordinateMap()
{
	qDebug() << "createPolarCoordinateMap";
	//-----------------resize InterPolateMAp und LutAngles------------------
	PixelProperties PP = PixelProperties();
	std::vector<PixelProperties> z;
	z.resize(volumeSize, PP);
	interpolationMap2D.resize(volumeSize, z);

	std::vector<PixelCoord> pxlCoord;
	for (int z = 0; z < (int)((volumeSize / 2) + 1); z++)
	{
		radiusAngleLut.push_back(pxlCoord);
	}

	int largestRadius;
	float center[2] = { (float)(((float)volumeSize / 2)),(float)(((float)volumeSize / 2)) };

	largestRadius = (int)(volumeSize / 2);//+1

	float x, y;
	float pxl[2] = { 0 };

	const int refPxl[2] = { 1,0 };
	float vect_A[2] = { 0 };
	float vect_B[2] = { 0 };

	for (int i = 0; i < volumeSize; i++)
	{
		for (int j = 0; j < volumeSize; j++)
		{
			x = i + 0.5;
			y = j + 0.5;
			pxl[0] = x;
			pxl[1] = y;

			interpolationMap2D[i][j].radius = (int)round(distanceCenter2Pixel(center, pxl));
			for (int k = 0; k < 2; k++)
			{
				vect_A[k] = refPxl[k];
				vect_B[k] = pxl[k] - center[k];
			}

			if (pxl[1] > center[1])
			{
				interpolationMap2D[i][j].angle = 360.f - getAngle(vect_A, vect_B);
			}

			else
				interpolationMap2D[i][j].angle = getAngle(vect_A, vect_B);

			//outside the highest radius there is no interpolation 
			if (interpolationMap2D[i][j].radius <= largestRadius)
				interpolationMap2D[i][j].tobeInterpolated = true;
			else
				interpolationMap2D[i][j].tobeInterpolated = false;
		}
	}
}

void InterpolationMap::populateInterpolationMap(QString _fileName)
{
	createPolarCoordinateMap();

	DicomHandler dicomHandler;
	DicomHandler::dicomDataProperties dicomHeader;

	for (int angleNr = 0; angleNr < sizeof(slices_angle); angleNr++)
	{
		QString fileDataName = _fileName + "/" + slices_angle.at(angleNr) + "/" + "0" + ".IMA";

		vtkSmartPointer<vtkImageData>imageDicom = dicomHandler.loadDicom(fileDataName);

		dicomHandler.getImageDataProperties(fileDataName, &dicomHeader);

		CoordinatesConverter voxel2WorldConverter(dicomHeader);

		float* voxelVolumePosition = new float[3];
		float* voxelWorldPosition = new float[3];

		int* dim = dicomHeader.dimension;
		//push all pixels with radius and radius +1 in the [radius] entry of the Lut
		for (unsigned int x = 0; x < dim[0]; x++)
		{
			for (unsigned int y = 0; y < dim[1]; y++)
			{
				voxelWorldPosition = voxel2WorldConverter.transform(x, y);
				voxelVolumePosition = world2VolumeConverter.transformTranslationFirst(voxelWorldPosition[0], voxelWorldPosition[1], voxelWorldPosition[2]);

				PixelCoord pxlCoord = PixelCoord((int)round(voxelVolumePosition[0]), (int)round(voxelVolumePosition[2]));

				PixelProperties* pxlProp = &interpolationMap2D[pxlCoord.x][pxlCoord.y];

				if (pxlProp->tobeInterpolated != false)
				{
					pxlProp->tobeInterpolated = false;

					int radius = pxlProp->radius;

					if (radiusAngleLut.size() > radius + 1)
					{
						std::vector<PixelCoord>* vectorAngles1 = &radiusAngleLut[radius + 1];
						vectorAngles1->push_back(pxlCoord);
					}
					std::vector<PixelCoord>* vectorAngles = &radiusAngleLut[radius];

					vectorAngles->push_back(pxlCoord);
				}
			}
		}
	}
	//sorting pixel an radius anglewise, delete radius + 1 angle if there is double entry for the same angle
	for (int i = 0; i < (int)volumeSize / 2; i++)
	{
		std::vector<PixelCoord>* vectorAngles = &radiusAngleLut[i];

		sort(vectorAngles->begin(), vectorAngles->end(), HandlePxlMap(interpolationMap2D));

		int n = vectorAngles->size();

		int m = 0;

		for (auto it = vectorAngles->begin(); it != vectorAngles->end(); /* NOTHING */)
		{
			if (it == vectorAngles->begin())
			{
				it++;
			}
			PixelCoord p = *it;
			PixelCoord q = *(it - 1);

			if (abs(interpolationMap2D[p.x][p.y].angle -
				interpolationMap2D[q.x][q.y].angle) < 11.0
				)
			{
				if (interpolationMap2D[p.x][p.y].radius != i)
				{
					vectorAngles->erase(it);
				}
				else
				{
					vectorAngles->erase(it - 1);
				}
			}
			else
			{
				++it;
			}
		}
	}
	// find left and right interpolation neighbor for each pixel
	int lhs, rhs;
	float distance0, distance1;

	for (int i = 0; i < volumeSize; i++)
	{
		for (int j = 0; j < volumeSize; j++)
		{
			if (interpolationMap2D[i][j].tobeInterpolated == true)
			{
				PixelProperties* toBeInterpolatedPixel = &interpolationMap2D[i][j];

				std::vector<PixelCoord>* pxlsOnRadius = &radiusAngleLut[interpolationMap2D[i][j].radius];//-1

				for (int i = 0; i < pxlsOnRadius->size(); i++)
				{
					PixelCoord pxl = pxlsOnRadius->at(i);

					if (interpolationMap2D[pxl.x][pxl.y].angle < toBeInterpolatedPixel->angle)
					{
						if (i == pxlsOnRadius->size() - 1)
						{
							//toBeInterpolatedPixel->angle is the biggest angle on the radius
							lhs = i;
							rhs = 0;
							distance0 = abs(toBeInterpolatedPixel->angle - interpolationMap2D[pxlsOnRadius->at(lhs).x][pxlsOnRadius->at(lhs).y].angle);
							distance1 = 390.0 - abs(toBeInterpolatedPixel->angle - interpolationMap2D[pxlsOnRadius->at(rhs).x][pxlsOnRadius->at(rhs).y].angle);

							toBeInterpolatedPixel->interpolationPartner0 = pxlsOnRadius->at(rhs);
							toBeInterpolatedPixel->interpolationPartner1 = pxlsOnRadius->at(lhs);
							float distance = abs(360.0 - abs(interpolationMap2D[pxlsOnRadius->at(rhs).x][pxlsOnRadius->at(rhs).y].angle - interpolationMap2D[pxlsOnRadius->at(lhs).x][pxlsOnRadius->at(lhs).y].angle));

							toBeInterpolatedPixel->weight0 = abs(distance0 / distance);
							toBeInterpolatedPixel->weight1 = 1 - toBeInterpolatedPixel->weight0;
						}
						else
							continue;
					}
					else
					{
						float  offset = 0;
						rhs = i;
						lhs = i - 1;
						if ((i - 1) > 0)
						{
							lhs = i - 1;
							distance0 = abs(toBeInterpolatedPixel->angle - interpolationMap2D[pxlsOnRadius->at(lhs).x][pxlsOnRadius->at(lhs).y].angle);
						}
						else
						{
							//smaller then  the smallest angle -> circle -> jump to biggest angle
							offset = 360.0;
							lhs = pxlsOnRadius->size() - 1;
							distance0 = 360.0 - abs(toBeInterpolatedPixel->angle - interpolationMap2D[pxlsOnRadius->at(lhs).x][pxlsOnRadius->at(lhs).y].angle);
						}
						if (i < pxlsOnRadius->size())
						{
							rhs = i;
							distance1 = abs(toBeInterpolatedPixel->angle - interpolationMap2D[pxlsOnRadius->at(rhs).x][pxlsOnRadius->at(rhs).y].angle);
						}

						toBeInterpolatedPixel->interpolationPartner0 = pxlsOnRadius->at(rhs);
						toBeInterpolatedPixel->interpolationPartner1 = pxlsOnRadius->at(lhs);
						float distance = abs(offset - abs(interpolationMap2D[pxlsOnRadius->at(rhs).x][pxlsOnRadius->at(rhs).y].angle - interpolationMap2D[pxlsOnRadius->at(lhs).x][pxlsOnRadius->at(lhs).y].angle));

						toBeInterpolatedPixel->weight0 = abs(distance0 / distance);
						toBeInterpolatedPixel->weight1 = 1 - toBeInterpolatedPixel->weight0;

						break;
					}
				}
			}
		}
	}
}

std::vector<std::vector<InterpolationMap::PixelProperties>> InterpolationMap::getinterpolationMap2D()
{
	return interpolationMap2D;
}

vtkSmartPointer<vtkImageData> InterpolationMap::getVesselMap()
{
	return vesselMap;
}

void InterpolationMap::setAllVoxel2Value(int _value)
{
	int* dim = vesselMap->GetDimensions();
	for (unsigned int x = 0; x < dim[0]; x++)
	{
		for (unsigned int y = 0; y < dim[1]; y++)
		{
			for (unsigned int z = 0; z < dim[2]; z++)
			{
				unsigned char* pixel = static_cast<unsigned char*>(vesselMap->GetScalarPointer(x, y, z));

				pixel[0] = _value;
				pixel[1] = _value;
				pixel[2] = _value;
			}
		}
	}
	vesselMap->Modified();
}

float InterpolationMap::dotProduct(float vect_A[], float vect_B[])
{
	float product = 0;
	int n = sizeof(vect_A);
	// Loop for calculate cot product 
	for (int i = 0; i < 2; i++)
	{
		product = product + vect_A[i] * vect_B[i];
	}
	return product;
}

float InterpolationMap::getAngle(float vect_A[], float vect_B[])
{
	return (180.0 / 3.141592653589793238463) *acosf(dotProduct(vect_A, vect_B) / (l2_norm(vect_A) * l2_norm(vect_B)));
}

float InterpolationMap::l2_norm(float u[], int n) {
	double accum = 0.;
	for (int i = 0; i < n; ++i) {
		accum += u[i] * u[i];
	}
	return sqrt(accum);
}

float InterpolationMap::distanceCenter2Pixel(float _center[2], float _pixel2[2])
{
	return (float)std::sqrt(std::pow((_center[0] - (_pixel2[0])), 2) + std::pow((_center[1] - (_pixel2[1])), 2));
}