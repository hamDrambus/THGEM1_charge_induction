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

  // TApplication app("app", &argc, argv);
  // Setup the gas.
  MediumMagboltz* gas = new MediumMagboltz();
  gas->SetTemperature(87.3);
  gas->SetPressure(1.0256*AtmosphericPressure); // To have correct gas density, as garfield uses ideal gas formula
	//gas->SetNumberDensity(8.62e19); // Not avaliable for gas medium
  gas->SetComposition("Ar", 1);

  // Set the field range to be covered by the gas table.
  const int nFields = 111;
  const double emin = 100;
  const double emax = 45000;
  const bool useLog = false;
  gas->SetFieldGrid(emin, emax, nFields, useLog);

  const int ncoll = 20;
	bool verbose = true;
  // Switch on debugging to print the Magboltz output.
  gas->EnableDebugging();
  gas->GenerateGasTable(ncoll, verbose);
  gas->DisableDebugging();

  gas->SetMaxElectronEnergy(5);
  gas->EnableEnergyRangeAdjustment(true);
  gas->EnableAnisotropicScattering();
  // Save the table.
	std::string out_file = "../Ar.gas";
	ensure_file(out_file);
  gas->WriteGasFile(out_file);

  // app.Run(kTRUE);

}
