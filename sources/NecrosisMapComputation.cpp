/*
*	Include class header.
*/
#include <NecrosisMapComputation.h>
/*
*	Class constructor.
*/
NecrosisMapComputation::NecrosisMapComputation()
{}
/*
*	Class deconstructor.
*/
NecrosisMapComputation::~NecrosisMapComputation()
{}
/////////////////////////////////////////////////////////////
//					computeHeatMap                         //
/////////////////////////////////////////////////////////////
void NecrosisMapComputation::computeHeatMap(
	vtkSmartPointer<vtkImageData> p_vtk_phaseReferece, 
	vtkSmartPointer<vtkImageData> p_vtk_phase, 
	vtkSmartPointer<vtkImageData> p_vtk_heatMap,
	float f_magneticFieldStrength,
	float f_echoTime)
{
	/*
	*	Define temporary variables.
	*/
	vtkSmartPointer<vtkImageData> p_vtk_refereceScaled;
	vtkSmartPointer<vtkImageData> p_vtk_phaseScaled;
	vtkSmartPointer<vtkImageData> p_vtk_differenceImage;
	vtkSmartPointer<vtkImageData> p_vtk_heatMap_temp;
	double d_newMinimum = -M_PI;
	double d_newMaximum = M_PI;
	double d_gyromagneticRatio = 42.577;
	double d_prfChangeConstant = 0.01;
	/*
	*	Rescale reference image to [-pi,pi].
	*/
	double d_referenceMinimum = p_vtk_phaseReferece->GetScalarRange()[0];
	double d_referenceMaximum = p_vtk_phaseReferece->GetScalarRange()[1];
	double d_referenceScaleFactor = (d_newMaximum - d_newMinimum) / (d_referenceMaximum - d_referenceMinimum);
	vtkSmartPointer<vtkImageShiftScale> vtk_referenceScale = vtkSmartPointer<vtkImageShiftScale>::New();
	vtk_referenceScale->SetInputData(p_vtk_phaseReferece);
	vtk_referenceScale->SetOutputScalarTypeToDouble();
	vtk_referenceScale->SetShift(-d_referenceMinimum);
	vtk_referenceScale->SetScale(d_referenceScaleFactor);
	vtk_referenceScale->SetShift(d_newMinimum);
	vtk_referenceScale->Update();
	p_vtk_refereceScaled = vtk_referenceScale->GetOutput();
	/*
	*	Rescale current phase image to [-pi,pi].
	*/
	double d_phaseMinimum = p_vtk_phase->GetScalarRange()[0];
	double d_phaseMaximum = p_vtk_phase->GetScalarRange()[1];
	double d_phaseScaleFactor = (d_newMaximum - d_newMinimum) / (d_phaseMaximum - d_phaseMinimum);
	vtkSmartPointer<vtkImageShiftScale> vtk_phaseScale = vtkSmartPointer<vtkImageShiftScale>::New();
	vtk_phaseScale->SetInputData(p_vtk_phase);
	vtk_phaseScale->SetOutputScalarTypeToDouble();
	vtk_phaseScale->SetShift(-d_phaseMinimum);
	vtk_phaseScale->SetScale(d_phaseScaleFactor);
	vtk_phaseScale->SetShift(d_newMinimum);
	vtk_phaseScale->Update();
	p_vtk_phaseScaled = vtk_phaseScale->GetOutput();
	/*
	*	Compute difference between images. Output = Input1 - Input2
	*/
	vtkSmartPointer<vtkImageMathematics> vtk_phaseDifference = vtkSmartPointer<vtkImageMathematics>::New();
	vtk_phaseDifference->SetOperationToSubtract();
	vtk_phaseDifference->SetInput1Data(p_vtk_phaseScaled);
	vtk_phaseDifference->SetInput2Data(p_vtk_refereceScaled);
	vtk_phaseDifference->Update();
	p_vtk_differenceImage = vtk_phaseDifference->GetOutput();
	/*
	*	Compute Heatmap according to the Proton Resonance Frequency Shift (PRFS) Method:
	*	Delta_T = (Phase - Reference) / (gyromagneticRatio * PRFchangeCoefficient * MagneticFieldStrength * EchoTime)
	*	gyromagneticConstant = 42.577 MHz*T^(-1)
	*	PRFchangeCoefficient = 0.01ppm = 0.01e^-6
	*	MagneticFieldStrength = f_p_magneticFieldStrength
	*	EchoTime = f_p_echoTime
	*/
	int* dims = p_vtk_heatMap->GetDimensions();
	double d_quotient = d_gyromagneticRatio * d_prfChangeConstant * f_magneticFieldStrength * f_echoTime;
	for (int z = 0; z < dims[2]; z++)
	{
		for (int y = 0; y < dims[1]; y++)
		{
			for (int x = 0; x < dims[0]; x++)
			{
				double* pixel = static_cast<double*>(p_vtk_heatMap->GetScalarPointer(x, y, z));
				pixel[0] = pixel[0] / d_quotient;
			}
		}
	}
	/*
	Print some information. Note that Difference Min/Max
	*/
	std::cout << "#################################\n";
	std::cout << "Old Min/Max: " << d_referenceMinimum << " / " << d_referenceMaximum << "\n";
	std::cout << "Scale Factor Reference: " << d_referenceScaleFactor << "\n";
	std::cout << "New Reference Min/Max: " << p_vtk_refereceScaled->GetScalarRange()[0] << " / " << p_vtk_refereceScaled->GetScalarRange()[1] << "\n";
	std::cout << "Scale Factor Phase: " << d_phaseScaleFactor << "\n";
	std::cout << "New Phase Min/Max: " << p_vtk_phaseScaled->GetScalarRange()[0] << " / " << p_vtk_phaseScaled->GetScalarRange()[1] << "\n";
	std::cout << "Difference Min/Max: " << p_vtk_differenceImage->GetScalarRange()[0] << " / " << p_vtk_differenceImage->GetScalarRange()[1] << "\n";
	std::cout << "#################################\n";
}