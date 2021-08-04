#include "DataVolume.h"
#include <math.h> 
#include<algorithm>
#include<string>

#define mytype uchar 
DataVolume::DataVolume()
{
	volume = vtkSmartPointer<vtkImageData>::New();
}

DataVolume::DataVolume(QString _fileName)
{
	DataVolume();
	timestep = 0;
	init(_fileName);
}

DataVolume::DataVolume(vtkSmartPointer<vtkImageData> _imageData)
{
	qDebug() << "Constructor Data Volume -Parameter";
	volume = _imageData;
}

DataVolume::~DataVolume()
{
	
}

void DataVolume::init(QString _fileName)
{
	fileName = _fileName;

	//get properties of orthogonal images to define volume.
	QString filename0 = _fileName + "/" + "0" + "/0.IMA";
	QString filename90 = _fileName + "/" + "90" + "/0.IMA";

	vtkSmartPointer<vtkDICOMReader> reader = vtkSmartPointer<vtkDICOMReader>::New();
	
	reader->SetFileName(filename0.toStdString().c_str());
	reader->Update();

	DicomHandler dHandler;
	DicomHandler::dicomDataProperties properties0;
	dHandler.getImageDataProperties(reader, &properties0);
	
	CoordinatesConverter converter(properties0);

	reader->SetFileName(filename90.toStdString().c_str());
	reader->Update();
	DicomHandler::dicomDataProperties properties90;
	dHandler.getImageDataProperties(reader, &properties90);

	volumeSize = properties90.dimension[0];
	
	CoordinatesConverter::CoordinateSystemProperties volumeCoordSystem;
	for (int i = 0; i < 3; i++)
	{
		volumeCoordSystem.maxExtend[i] = volumeSize - 1;//- 1
		volumeCoordSystem.minExtend[i] = 0;
		volumeCoordSystem.origin[i] = 0;
	}
	
	volume = vtkSmartPointer<vtkImageData>::New();

	volume->SetDimensions(volumeSize, volumeSize, volumeSize);

	volume->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	volume->SetSpacing(properties0.pixelSpacing[0], properties0.pixelSpacing[1], properties0.pixelSpacing[0]);
	volume->SetOrigin(properties90.imagePosition[0], properties0.imagePosition[1], properties0.imagePosition[2]);

	CoordinatesConverter::CoordinateSystemProperties worldCoordSystem;
	//get extend of World Coordinate System
	converter.getCoordinateSystemProperties(properties90, &worldCoordSystem);

	CoordinatesConverter::TransformationMatrixFeatures world2VolumeTransform;
	converter.getTransMatrixFeatures(volumeCoordSystem, worldCoordSystem, &world2VolumeTransform);

	world2VolumeConverter = CoordinatesConverter(world2VolumeTransform);
	qDebug ()<< "_________________________________________________________________________________________";
	qDebug() << "trans" << properties90.imagePosition[0] << properties0.imagePosition[1] << properties0.imagePosition[2];
	qDebug() << "matrix";
	world2VolumeConverter.getTransformMatrix()->Print(std::cout);
	qDebug() << world2VolumeConverter.getTransformMatrix()->GetElement(1, 3);
	volume->SetOrigin(properties90.imagePosition[0],  world2VolumeConverter.getTransformMatrix()->GetElement(1, 3), world2VolumeConverter.getTransformMatrix()->GetElement(2, 3));
	volume->Modified();

	setAllVoxel2Value(0);
}

void DataVolume::writeToFile(QString _targetfileName)
{
	DicomHandler dhandler;

	vtkSmartPointer<vtkDICOMWriter> writer_d = vtkSmartPointer<vtkDICOMWriter>::New();
	// Create a generator for MR images.
	vtkNew<vtkDICOMMRGenerator> generator;
	
	vtkSmartPointer<vtkMatrix4x4> p = vtkSmartPointer<vtkMatrix4x4>::New();
	p->GlobalWarningDisplayOff();
	p->Identity();
	p->Modified();

	generator->Modified();
	writer_d->SetGenerator(generator.GetPointer());

	// Set the output filename format as a printf-style string.
	writer_d->SetFilePattern("%s/IM-0001-%04.3d.dcm");
	
	std::string l = _targetfileName.toStdString() +"/output";
	writer_d->SetFilePrefix(l.c_str());

	writer_d->SetInputData(volume);

	writer_d->Modified();
	writer_d->Write();

	qDebug() << "write Dicom -Done";
}

void DataVolume::addSlice(QString _angle, QString _timeStep)
{
	qDebug() << "DataVolume::addSlice";

	int index = slices_angle.lastIndexOf(_angle);
	DicomHandler dicomHandler;
	QString fileDataName = fileName + "/" + _angle + "/"  + _timeStep + ".IMA";
	
	vtkSmartPointer<vtkImageData> imageDicom = dicomHandler.loadDicom(fileDataName);
	DicomHandler::dicomDataProperties dicomHeader;
	dicomHandler.getImageDataProperties(fileDataName, &dicomHeader);
	CoordinatesConverter voxel2WorldConverter(dicomHeader);
	
	Slice actSlice = Slice(imageDicom, voxel2WorldConverter, timestep);

	if (slicesBuffer[index].timestep >= 0)
	{
		slicesBuffer[index] = actSlice;
	}
	else
		slicesBuffer[index] = actSlice;
	timestep++;

	float* voxelVolumePosition = new float[3];
	float* voxelWorldPosition = new float[3];

	int* dim = dicomHeader.dimension;
	for (unsigned int x = 0; x < dim[0] ; x++)
	{
		for (unsigned int y = 0; y < dim[0]; y++)
		{
			voxelWorldPosition = voxel2WorldConverter.transform(x, dim[1] - y - 1);
			voxelVolumePosition = world2VolumeConverter.transformTranslationFirst(voxelWorldPosition[0], voxelWorldPosition[1], voxelWorldPosition[2]);

			mytype* pixelImage = static_cast<mytype*>(actSlice.data->GetScalarPointer(x, y, 0));
			mytype* voxelVolume = static_cast<mytype*>(volume->GetScalarPointer((int)round(voxelVolumePosition[0]), (int)round(voxelVolumePosition[1]), (int)round(voxelVolumePosition[2])));

			voxelVolume[0] = pixelImage[0];	
		}
	}
	volume->Modified();
}

void DataVolume::display(vtkSmartPointer<vtkRenderWindow> _q_vtk_renderWindow, bool _isCropped, bool _isCameraAbove)
{
	qDebug() << "DataVolume::display";
	int low = 65;//10
	int high = 200;//220
	/*
	*	vtkNamedColors bietet eine einfache Farbdatenbank mit entsprechenden Namen, die man sich einfach ausgeben lassen kann.
	*	Beim Setzen der Hintergrundfarbe habe ich zum Beispiel die Farbe "Wheat" genommen.
	*/
	vtkSmartPointer<vtkNamedColors> namedColors = vtkSmartPointer<vtkNamedColors>::New();
	/*
	*	Hier wird das entsprechende RenderWindow und der dazu gehörige Renderer geholt.
	*/
	vtkSmartPointer<vtkRenderer> q_vtk_renderer = vtkSmartPointer<vtkRenderer>::New();
	_q_vtk_renderWindow->AddRenderer(q_vtk_renderer);
	/*
	*	Hier gibts den Interactor und den 3D InteractorStyle.
	*/
	vtkSmartPointer<vtkRenderWindowInteractor> q_vtk_interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkInteractorStyleTrackballCamera> q_vtk_interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	q_vtk_interactor->SetInteractorStyle(q_vtk_interactorStyle);
	q_vtk_interactor->SetRenderWindow(_q_vtk_renderWindow);
	/*
	*	Als Mapper hab ich mich in der Fülle von Möglichkeiten für den OpenGL GPU Volume Cast Mapper entschieden, der in der Lage ist
	*	IsoSurfaces vernünftig darzustellen weil die entsprechenden Shader bereits integriert sind.
	*/
	vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
	if (_isCropped == true)
	{
		vtkSmartPointer<vtkImageData> croppedVolume = vtkSmartPointer<vtkImageData>::New();
		crop(volume, croppedVolume, 0.80f);
		volumeMapper->SetInputData(croppedVolume);
	}
	else
	{
		volumeMapper->SetInputData(volume);
	}
	/*
	*	Die automatische Sample Distanz könnte je nach Auflösung des Datensatzes zu Problemen führen. Ich hab sie jetzt auf 0.5 gesetzt aber
	*	da könnt ihr einfach mal ein bisschen rumspielen mit den Einstellungen. Wann sieht es gut aus und ist trotzdem performant?
	*	Anschließend wird der BlendMode auf IsoSurface gesetzt. Da könnt ihr aber auch nochmal rumspielen, vielleicht findet ihr was cooleres.
	*/
	volumeMapper->AutoAdjustSampleDistancesOff();
	volumeMapper->SetSampleDistance(0.5);
	volumeMapper->SetBlendModeToIsoSurface();
	/*
	*	Jetzt wird es ekelhaft. Um vernünftig zu visualisieren müsst ihr eine richtige Transferfunktion definieren für die Isowerte. Das geschieht mit
	*	zwei sogenannten LookupTables. In der vtkColorTransferFunction definiert ihr, welcher Voxelwert welche Farbe bekommt. In diesem Fall ist alles
	*	mit dem Wert 10 grün und alles mit dem Wert 220 rot.
	*/
	vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
	color->RemoveAllPoints();
	for(int i = 0; i < low; i++)
		color->AddRGBPoint(i, 0.0, 1.0, 0.0);
	for (int j = low; j < high; j++)
		color->AddRGBPoint(j, 1.0, 0.0, 0.0);
	/*
	*	Mit der vtkPiecewiseFunction definiert ihr die Opacität eines entsprechenden Wertes. Die Punkte, die ihr hier eintragt müssen identisch sein mit den
	*	Werten in der vtkColorTransferFunction. Hier haben Voxel mit dem Wert 10 eine Opazität von 30% und Voxel mit einem Wert von 220 eine Opazität von 60%.
	*/
	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity->RemoveAllPoints();

	for (int i = 0; i < low; i++)
		compositeOpacity->AddPoint(i, 0.0);
	for (int j = low; j < high; j++)
		compositeOpacity->AddPoint(j, 1.0);
	/*
	*	Jetzt müssen wir die VolumeProperty definieren, dazu stellen wir das Shading an und eine einfache lineare interpolation. Bei der Interpolation gibt
	*	es bestimmt auch noch andere Möglichkeiten. SetColor bekommt die vtkColorTransferFunction und setScalarOpacity bekommt die
	*	vtkPiecewiseFunction.
	*/
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOn();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->SetColor(color);
	volumeProperty->SetScalarOpacity(compositeOpacity);
	/*
	*	Dann müssen wir das Volumen definieren mit dem entsprechenden Mapper und den vorab eingestellenten Properties.
	*/
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);
	/*
	*	Jetzt schmeißen wir alles in den Renderer und setzen am Ende das DepthPeeling auf True, damit der Renderer weiß
	*	in welcher Reihenfolge er die Strukturen visualisieren muss.
	*/
	q_vtk_renderer->AddVolume(volume);
	q_vtk_renderer->SetBackground(namedColors->GetColor3d("Wheat").GetData());
	
	if (_isCameraAbove)
	{
		q_vtk_renderer->GetActiveCamera()->SetFocalPoint(volume->GetCenter());
		q_vtk_renderer->GetActiveCamera()->SetPosition(0, 200, 0);
	}

	q_vtk_renderer->SetUseDepthPeeling(true);
	/*
	*	Ganz am Ende weisen wir unseren entsprechenden Isowerten auch noch die Isovalues zu für die interne Verarbeitung.
	*	Hier repräsentiert unsere IsoSurface 0 den Wert 10 und unsere IsoSurface 1 den Wert 220. Auch diese Werte müssen
	*	identisch sein mit den Werten in der Transfer Funktion.
	*/
	volumeProperty->GetIsoSurfaceValues()->SetValue(0, low);
	volumeProperty->GetIsoSurfaceValues()->SetValue(2, high);

	_q_vtk_renderWindow->Render();
	qDebug() << "DataVolume::display - Done";
}


 
void DataVolume::interpolate(std::vector<std::vector<InterpolationMap::PixelProperties>>interpolationMap2D, vtkSmartPointer<vtkImageData> _vesselMap)//
{
	for (int x = 0; x < volumeSize; x++)
	{
		for (int y = 0; y < volumeSize; y++)
		{
			for (int z = 0; z < volumeSize; z++)
			{
				
				mytype* voxelVolume = static_cast<mytype*>(volume->GetScalarPointer(x, y, z));
				
				if (interpolationMap2D[x][z].tobeInterpolated == true)
				{
					//http://www.doc.ic.ac.uk/~eedwards/teaching/2012/AGV/VolRenCoursework/VolumeRender.cpp
					
					mytype* interpoloteVoxel0 = static_cast<mytype*>(volume->GetScalarPointer(interpolationMap2D[x][z].interpolationPartner0.x, y, interpolationMap2D[x][z].interpolationPartner0.y));
					mytype* interpoloteVoxel1 = static_cast<mytype*>(volume->GetScalarPointer(interpolationMap2D[x][z].interpolationPartner1.x, y, interpolationMap2D[x][z].interpolationPartner1.y));

					voxelVolume[0] = interpoloteVoxel0[0] * interpolationMap2D[x][z].weight0 + interpolationMap2D[x][z].weight1 * interpoloteVoxel1[0];
				}
				{
					mytype* vesP = static_cast<mytype*>(_vesselMap->GetScalarPointer(x, y, z));
					if (vesP[0] == 1)
					{	
						volume->SetScalarComponentFromDouble(x, y, z, 0, 21.0);//21.
					}
				}
			}
		}
	}
}

void DataVolume::crop(vtkSmartPointer<vtkImageData> volume, vtkSmartPointer<vtkImageData> croppedvolume, float percentage)
{
	int newSize = (int) volumeSize * percentage;
	
	int SizeDiff = (int) (volumeSize - newSize)/2;

	croppedvolume->SetDimensions(newSize, volumeSize, newSize);
	croppedvolume->SetOrigin(0, 0, 0);

#if VTK_MAJOR_VERSION <= 5
#else
	croppedvolume->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
#endif

	for (int x = 0; x < newSize; x++)
	{
		for (int y = 0; y < volumeSize; y++)
		{
			for (int z = 0; z < newSize; z++)
			{
					mytype* voxelCroppedVolume = static_cast<mytype*>(croppedvolume->GetScalarPointer(x, y, z));
					mytype* voxelVolume = static_cast<mytype*>(volume->GetScalarPointer(x + SizeDiff, y , z + SizeDiff));
					voxelCroppedVolume[0] = voxelVolume[0];
			}
		}
	}
	croppedvolume->Modified();
}

void DataVolume::crop(float percentage)
{
	int newSize = (int)volumeSize * percentage;
	int SizeDiff = (int)(volumeSize - newSize) / 2;
	vtkSmartPointer<vtkImageData> tempVolume = vtkSmartPointer<vtkImageData>::New();
	tempVolume->DeepCopy(volume);
	volume->Delete();

	volume = vtkSmartPointer<vtkImageData>::New();
	volume->SetDimensions(newSize, newSize, newSize);
	volume->SetSpacing(tempVolume->GetSpacing());
	volume->SetOrigin(tempVolume->GetOrigin());
	
	volume->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

	for (int x = 0; x < newSize; x++)
	{
		for (int y = 0; y < newSize; y++)
		{
			for (int z = 0; z < newSize; z++)
			{
				mytype* voxelCroppedVolume = static_cast<mytype*>(volume->GetScalarPointer(x, y, z));
				mytype* voxelVolume = static_cast<mytype*>(tempVolume->GetScalarPointer(x + SizeDiff, y + SizeDiff, z + SizeDiff));
				voxelCroppedVolume[0] = voxelVolume[0];
			}
		}
	}
	volume->Modified();
}

vtkSmartPointer<vtkImageData> DataVolume::getImageData()
{
	return volume;
}

void DataVolume::setAllVoxel2Value(int _value)
{
	int* dim = volume->GetDimensions();
	for (unsigned int x = 0; x < dim[0]; x++)
	{
		for (unsigned int y = 0; y < dim[1]; y++)
		{
			for (unsigned int z = 0; z < dim[2]; z++)
			{
				unsigned char* pixel = static_cast<unsigned char*>(volume->GetScalarPointer(x, y, z));
				pixel[0] = _value;
				pixel[1] = _value;
				pixel[2] = _value;
				
			}
		}
	}
	volume->Modified();
}
