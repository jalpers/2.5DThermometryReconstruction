//----------------------------------------------------------------------------------
//! Class ThermometryReconstruction.
/*!
// \file
// \author  Julian Alpers
// \date    2020-11-13
//
// Basic class for reconstruction of the 3D thermometry map.
*/
//----------------------------------------------------------------------------------
#ifndef THERMOMETRYRECONSTRUCTION_H
#define THERMOMETRYRECONSTRUCTION_H
/*
*	Include QT header.
*/
#include<InterpolationMap.h>

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
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

#include <vtkImageShiftScale.h>

#include <vtkNamedColors.h>
#include <vtkContourValues.h>
/*
*	Own includes.
*/
#include <DicomHandler.h>
	
#include<DataVolume.h>

QT_BEGIN_NAMESPACE
namespace Ui { class ThermometryReconstruction; }
QT_END_NAMESPACE

class ThermometryReconstruction : public QMainWindow
{
    Q_OBJECT

public:
    ThermometryReconstruction(QWidget *parent = nullptr);
    ~ThermometryReconstruction();


private slots:
	
	void updateImage();
	/*!
	*	\brief Load specified dicom file.
	*	\param q_c_dicomFilePath Path to the dicom file.
	* 	\param q_c_viewer Specification of the desired viewer to change its content.
	*	\return none
	*/
	void slotButtonBrowse();
	/*!
	*	\brief Slot function connected to the "Define reference" button.
	*	\param none
	*	\return none
	*/
	void slotButtonDefineReference();
	/*!
	*	\brief Slot function connected to the "Compute heat map" button.
	*	\param none
	*	\return none
	*/
	void slotButtonComputeHeatMap();

private:
	bool isCropped;
    Ui::ThermometryReconstruction *ui;
    QFileDialog* m_q_dialog ;			                    //!< Pointer to the dialog to browse files and directories.
	QTimer* timer;

	QString q_c_dicomFilePath ;
	QString tubeFilePath;
	int fileID;
	int timestep;
	QStringList angleList =  { "0","22_5","45","67_5","90","112_5","135","157_5" };
	DataVolume volume;
	DataVolume referenceVolume;
	
	InterpolationMap inPo;
};
#endif // THERMOMETRYRECONSTRUCTION_H