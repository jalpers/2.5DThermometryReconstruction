# 2.5DThermometryReconstruction

This repository is related to the MICCAI paper "2.5D Thermometry Maps for MRI-guided Tumor Ablation" by Alpers and Reimert et al.

Used dependencies:
1) QT 5.12.1 https://download.qt.io/official_releases/qt/5.12/5.12.1/ 
	a) Used version msvc2017_64
2) VTK-8.2.0 https://vtk.org/download/
	a) There is a bug in the version 8.2.0, which leads to cmake not locating the vtkDICOM package. To fix the bug please locate the downloaded vtk folder and go to Remote/CmakeLists.txt. Replace the file with the one located in the folder "vtkRemote CMake Change" from this repository. The bugfix is described in the following vtk commit https://gitlab.kitware.com/vtk/vtk/-/commit/0a90fe94
	b) After configuration check the Module_vtkDicom and vtk_Group_Qt
3) Visual Studio 2019

Additional parameters:
1) Change line 17 in the cmake file "set( ENVIRONMENT_VARIABLES" to point to your local dependencies QT and VTK. Otherwise you will get missing dll errors upon running the program.
	a) QT standard path "Qt\5.12.1\msvc2015_64\lib\cmake\Qt5"
	b) VTK standard path "VTK-8.2.0\build"