#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
//#include <curses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__WIN32__)
#define NOMINMAX
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#endif

#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TH1F.h>
#include <TH2D.h>

#include "MediumMagboltz.hh"
#include "SolidBox.hh"
#include "GeometrySimple.hh"
#include "ComponentElmer.hh"
#include "ViewGeometry.hh"
#include "ViewField.hh"
#include "ViewDrift.hh"
#include "ViewMedium.hh"
#include "ComponentConstant.hh"
#include "Sensor.hh"
#include "TrackHeed.hh"
#include "Plotting.hh"
#include "Random.hh"
#include "GarfieldConstants.hh"
#include "FundamentalConstants.hh"

using namespace Garfield;

void ensure_file(std::string fname); //makes sure file can be created later on
void ensure_folder(std::string folder);

void ensure_file(std::string fname)
{
  std::string folder = fname;
  while ((folder.back() != '\\') &&(folder.back()!='/') &&!folder.empty())
    folder.pop_back();
  if (!folder.empty())
    folder.pop_back();
  ensure_folder(folder);
}

void ensure_folder(std::string folder)
{
  if (folder.empty())
    return;
#if defined(_WIN32)||defined(_WIN64)
  DWORD ftyp = GetFileAttributesA(folder.c_str());
  if (!(ftyp & FILE_ATTRIBUTE_DIRECTORY) || ftyp == INVALID_FILE_ATTRIBUTES) {
    int code = system(("mkdir \"" + folder + "\"").c_str());
    if (code)
      std::cout << "mkdir error: " << GetLastError() << std::endl;
  }
#else //defined(_WIN32)||defined(_WIN64)
  struct stat st;
  if (-1==stat(folder.c_str(), &st)) {
    int err = errno;
    switch (err) {
    case (EACCES): {
      std::cout<<"Access error"<<std::endl;
      break;
    }
    case (ENAMETOOLONG): {
      std::cout<<"Path is too long"<<std::endl;
      break;
    }
    case (ENOENT) :
    case (ENOTDIR): {
      int code = system(("mkdir -p \"" + folder + "\"").c_str());
      if (code)
        std::cout << "mkdir -p error: " << code << std::endl;
      break;
    }
    default:{
      std::cout<<"stat(\""<<folder<<"\") returned -1; errno == "<<err<<std::endl;
      break;
    }
    }
  } else {
    if (!S_ISDIR(st.st_mode)) {
      int code = system(("mkdir -p \"" + folder + "\"").c_str());
      if (code)
        std::cout << "mkdir -p error: " << code << std::endl;
    }
  }
#endif //defined(_WIN32)||defined(_WIN64)
}

int main(int argc, char * argv[]) {
  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();

  // Load a gas file which includes excitation and ionisation rates.
  MediumMagboltz* gas = new MediumMagboltz();
  gas->LoadGasFile("../Ar.gas");

  /// Select the extrapolation method for fields below/above the table range.
  /// Possible options are "constant", "linear", and "exponential"
  gas->SetInterpolationMethodVelocity(2);
	gas->SetInterpolationMethodDiffusion(2);
	gas->SetInterpolationMethodTownsend(2);
  gas->SetExtrapolationMethodVelocity("constant", "linear");
  gas->SetExtrapolationMethodDiffusion("linear", "constant");
  gas->SetExtrapolationMethodTownsend("constant", "linear");
  gas->PrintGas();

  ViewMedium view1;
  view1.SetMedium(gas);

  // Plot the Townsend coefficient (without Penning transfer).
  TCanvas c1("Townsend_coeff", "Townsend_coeff", 800, 800);
  view1.SetCanvas(&c1);
  view1.PlotElectronTownsend();

  TCanvas c2("Drift_velocity", "Drift_velocity", 800, 800);
  view1.SetCanvas(&c2);
  view1.PlotElectronVelocity();

  TCanvas c3("Attachment", "Attachment", 800, 800);
  view1.SetCanvas(&c3);
  view1.PlotElectronAttachment();

  TCanvas c4("Diffusion", "Diffusion", 800, 800);
  view1.SetCanvas(&c4);
  view1.PlotElectronDiffusion();

  std::cout<<"Finished.\n";
  app.Run(true);
  return 0;
}
