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
#include "ViewSignal.hh"
#include "ViewFEMesh.hh"
#include "ComponentConstant.hh"
#include "Sensor.hh"
#include "TrackHeed.hh"
#include "Plotting.hh"
#include "AvalancheMC.hh"
#include "Random.hh"
#include "RandomEngineRoot.hh"
#include "GarfieldConstants.hh"

using namespace Garfield;

//Garfield sizes are in cm
#define _VISUAL 0
#define _X_SIZE 0.0225
#define _Y_SIZE 0.03897114317
#define _Z_CATHODE -0.32
#define _Z_ANODE 0.32
#define MESH_ std::string("../v00.02_THGEM1/")
#define RESULT_ std::string("../Elmer_v00.02/case_v01.result")
#define RESULTW_ std::string("../Elmer_v00.02/case_v01_weight.result")

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
#if defined(__WIN32__)
	if (!folder.empty()) {
		DWORD ftyp = GetFileAttributesA(folder.c_str());
		if (!(ftyp & FILE_ATTRIBUTE_DIRECTORY) || ftyp == INVALID_FILE_ATTRIBUTES) {
			int code = system(("mkdir \"" + folder + "\"").c_str());
			if (code)
				std::cout << "mkdir error: " << GetLastError() << std::endl;
		}
	}
#else
	struct stat st;
	stat(folder.c_str(), &st);
	if (!S_ISDIR(st.st_mode)) {
		int code = system(("mkdir \"" + folder + "\"").c_str());
		if (code)
			std::cout << "mkdir error: " << code << std::endl;
	}
#endif //_WIN32__
}

void plot_field(ComponentFieldMap* fm, std::string filename, double x0 ,double y0,double z0 ,double x1,double y1 ,double z1, int Num,std::string name="", double L_fine=0, int Num_fine=0)
{
  ensure_file(filename);
  double dphi=0;
  double L = sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)+(z1-z0)*(z1-z0));
  double s=0,x=x0,y=y0,z=z0;
  double ex,ey,ez,ee;
  Medium* m;
  int status;
  int LN=(Num-Num_fine)/2.0;
  int RN=Num-LN;
  double ds=0;
  if ((L_fine<=0)||(Num_fine>=Num)||(L_fine>=s)||(RN<LN)||(Num_fine<=0)||(LN>Num)||(LN<0))
  {
    Num_fine=0;
    L_fine=0;
    LN=Num;
    RN=-Num;
  }
  std::ofstream flll(filename,std::ios_base::out);
	//flll<<"//z\tx\ty\tL\tEabs\tEx\tEy\tEz"
  for (int hh=0;hh<=Num;hh++) //hh<=Num in order to cover [x0;x1], not [x0;x1)
  {
     x=(x1-x0)*s/L+x0;
     y=(y1-y0)*s/L+y0;
     z=(z1-z0)*s/L+z0;
     fm->ElectricField(x,y,z,ex,ey,ez,m,status);
     ee=std::sqrt(ex*ex+ey*ey+ez*ez);
     if (ee>0) //there was a bug: because of -0.0 comparison in elmer component
     //resulting in appearing of 0 field in arbitrary points - fixed by me, yet better to leave this condition
     {
        if((x0==x1)&&(y0==y1))
           flll<<z<<'\t';
        else if ((z0==z1)&&(y0==y1))
           flll<<x<<'\t';
        else if ((x0==x1)&&(z0==z1))
           flll<<y<<'\t';
        else
           flll<<s<<'\t';
        flll<<ee<<'\t'<<ex<<'\t'<<ey<<'\t'<<ez<<std::endl;
     }
     if ((hh<LN)||(hh>RN))
     {
        ds=(L-L_fine)/(Num-LN-RN-1);
        dphi+=(ds/L)*(ex*(x1-x0)+ey*(y1-y0)+ez*(z1-z0));
        s+=ds;
     }
     else
     {
        ds=L_fine/(RN-LN+1);
        dphi+=ex*ds*(x1-x0)/L+ey*ds*(y1-y0)/L+ez*ds*(z1-z0)/L;
        s+=ds;
     }
  }
  if (name.size())
      std::cout<<"Int[E(r)*dr] on "<<name<<" = "<<dphi<<std::endl;
  flll.close();
}

void plot_field_lines(ComponentFieldMap* fm, int N_lines, double x_start, double x_finish) {
	if (N_lines <= 0)
		return;
	TCanvas* c_field = new TCanvas("Field lines xz", "Field lines xz", 800, 800);
	Sensor* sensor = new Sensor();
	sensor->AddComponent(fm);
	sensor->SetArea(-5.5*_X_SIZE,-5.5*_Y_SIZE,_Z_CATHODE, 5.5*_X_SIZE, 5.5*_Y_SIZE, _Z_ANODE);

	ViewDrift* viewDr = new ViewDrift();
  viewDr->SetArea(-5.5*_X_SIZE, -5.5*_Y_SIZE, _Z_CATHODE, 5.5*_X_SIZE, 5.5*_Y_SIZE, _Z_ANODE);
  viewDr->SetCanvas(c_field);
  //viewDr->EnableDebugging();

  AvalancheMC *aval = new AvalancheMC();
  //aval->EnableDiffusion();
  aval->DisableDiffusion();
  aval->EnableDebugging(false);
  aval->SetSensor(sensor);
  aval->SetTimeSteps(0.01);
	aval->EnablePlotting(viewDr);

  ViewFEMesh* vMsh = new ViewFEMesh();
  vMsh->SetCanvas(c_field);
  vMsh->SetComponent(fm);
  vMsh->SetArea(-5.5*_X_SIZE, -5.5*_Y_SIZE, -0.15, 5.5*_X_SIZE, 5.5*_Y_SIZE, 0.15);
  vMsh->SetPlane(0,-1,0,0,_Y_SIZE,0);
  vMsh->SetFillMesh(true);
  vMsh->SetColor(1,kGray);
  vMsh->SetColor(2,kYellow);
  vMsh->SetColor(3,kYellow);
  vMsh->EnableAxes();
  vMsh->EnableDebugging(true);
	vMsh->SetDrawViewRegion(false);
  vMsh->SetViewDrift(viewDr);

  for (int i=0; i<N_lines; ++i) {
		double x = (N_lines == 1 ? 0.5*(x_finish + x_start) : x_start + (x_finish - x_start)*i/(N_lines - 1));
    aval->DriftElectron(x, _Y_SIZE, -0.149, 0);
  }
	std::cout<<"plot_field_lines: vMsh->Plot()"<<std::endl;
  std::cout<<vMsh->Plot()<<std::endl;
	c_field->Update();
}

int main(int argc, char * argv[]) {
	// Created on 2022.09.12
	// Simulates response of THGEM1's electrode (induced charge signnal) on single
	// drifting electron when THGEM1 also produces an avalanche. Response in averaged
	// over all simulated electrons. Resulting signal should be comvoluted with time
	// distribution of initial electrons (S2 pulse-shape of EL signal, with slow components)
	// to obtain charge pulse-shape expected in the experiment.

	unsigned int seed = 42;
	double time_step = 0.5;
	unsigned int N_events = 1;
	try {
		if (argc > 1)
			N_events = std::stoul(argv[1]);
		if (argc > 2)
			time_step = std::stod(argv[2]);
		if (argc > 3)
			seed = std::stoul(argv[3]);
	} catch(std::invalid_argument &e) {
		std::cerr<<e.what()<<std::endl;
		std::cerr<<"Invalid argument, using defaults"<<std::endl;
		seed = 42;
		time_step = 0.5;
		N_events = 1;
	}
	catch(std::out_of_range &e) {
		std::cerr<<e.what()<<std::endl;
		std::cerr<<"Input value is out of range, using defaults"<<std::endl;
		seed = 42;
		time_step = 0.5;
		N_events = 1;
	}
	catch(std::exception &e) {
		std::cerr<<e.what()<<std::endl;
		std::cerr<<"Unknown exception, using defaults"<<std::endl;
		seed = 42;
		time_step = 0.5;
		N_events = 1;
	}

	gStyle->SetCanvasDefH(800);
	gStyle->SetCanvasDefW(1000);
	randomEngine.Seed(seed);

	argc = 1;
  TApplication app("app", &argc, argv);
  plottingEngine.SetDefaultStyle();
  ComponentElmer* fm = new ComponentElmer();
  bool is_loaded = fm->Initialise(MESH_+"mesh.header", MESH_+"mesh.elements",MESH_+ "mesh.nodes",MESH_+ "diels.dat",RESULT_,"mm");
	is_loaded = is_loaded && fm->SetWeightingField(RESULTW_, "THGEM1");
  if (is_loaded)
  	std::cout<<"Loaded mesh."<<std::endl;
  else {
		std::cout<<"Error: failed to load mesh or potential fields."<<std::endl;
		return 1;
  }
  fm->EnableMirrorPeriodicityX();
  fm->EnableMirrorPeriodicityY();
  for (int i=0; i < fm->GetNumberOfMaterials(); ++i)
     fm->NotDriftMedium(i);
	fm->DriftMedium(0);

	// Make a medium
	MediumMagboltz* gas = new MediumMagboltz();
  gas->LoadGasFile("Ar_v1.gas");
	gas->SetInterpolationMethodVelocity(2);
	gas->SetInterpolationMethodDiffusion(2);
	gas->SetInterpolationMethodTownsend(2);
  gas->SetExtrapolationMethodVelocity("constant", "linear");
  gas->SetExtrapolationMethodDiffusion("linear", "constant");
  gas->SetExtrapolationMethodTownsend("constant", "constant"); // constant at high fields effectively remedies hight E artifacts

  MediumMagboltz* met = new MediumMagboltz();
  met->SetComposition("co2", 100.);
  met->SetTemperature(293.15);
  met->SetPressure(760.);
  met->EnableDrift(false);
  MediumMagboltz* diel = new MediumMagboltz();
  diel->SetComposition("co2", 20.,"h2",80.);
  diel->SetTemperature(293.15);
  diel->SetPressure(760.);
  diel->EnableDrift(false);

  fm->SetMedium(0,gas);
  fm->SetMedium(1,diel);  //FR4
  fm->SetMedium(2,met);
  fm->SetMedium(3,met);

	plot_field(fm, "v00.02.01/center_axis_field.txt", _X_SIZE, _Y_SIZE, _Z_CATHODE + 0.02, _X_SIZE, _Y_SIZE, _Z_ANODE - 0.02, 3000, "axis z");
	//plot_field_lines(fm, 1, -4.8*_X_SIZE, 4.8*_X_SIZE);

	// Make a sensor
  Sensor* sensor = new Sensor();
  sensor->AddComponent(fm);
	sensor->AddElectrode(fm, "THGEM1");
  sensor->SetArea(-5.5*_X_SIZE,-5.5*_Y_SIZE,_Z_CATHODE, 5.5*_X_SIZE, 5.5*_Y_SIZE, _Z_ANODE);
	// Set the time bins.
	const unsigned int nTimeBins = 800;
	const double tstep = 0.5; // ns
	sensor->SetTimeWindow(0., tstep, nTimeBins);

  TCanvas* c_aval = new TCanvas("Avalanche xz", "Avalanche xz", 800, 800);
	ViewDrift* viewDr = new ViewDrift();
  viewDr->SetArea(-5.5*_X_SIZE, -5.5*_Y_SIZE, _Z_CATHODE, 5.5*_X_SIZE, 5.5*_Y_SIZE, _Z_ANODE);
  viewDr->SetCanvas(c_aval);
	//viewDr->EnableDebugging();

  AvalancheMC *aval = new AvalancheMC();
  aval->EnableDiffusion();
  //aval->DisableDiffusion();
  aval->EnableDebugging(false);
  aval->SetSensor(sensor);
  aval->SetTimeSteps(time_step);
	//aval->SetDistanceSteps(time_step);
	aval->EnableSignalCalculation();
	aval->EnableAvalancheSizeLimit(200);
	aval->SetTimeWindow(0, 400);

  ViewFEMesh* vMsh = new ViewFEMesh();
  vMsh->SetCanvas(c_aval);
  vMsh->SetComponent(fm);
  vMsh->SetArea(-5.5*_X_SIZE, -5.5*_Y_SIZE, -0.15, 5.5*_X_SIZE, 5.5*_Y_SIZE, 0.15);
  vMsh->SetPlane(0,-1,0,0,_Y_SIZE,0);
	//vMsh->SetPlane(0,0,1,0,0, 0.0225);
  vMsh->SetFillMesh(true);
  vMsh->SetColor(1,kGray);
  vMsh->SetColor(2,kYellow);
  vMsh->SetColor(3,kYellow);
  vMsh->EnableAxes();
  vMsh->EnableDebugging(false);
  vMsh->SetViewDrift(viewDr);
	vMsh->SetDrawViewRegion(false);

  double x1,y1,z1,t1,x2,y2,z2,t2;
  unsigned int NUM=1000;
  NUM = N_events;
  int interval = NUM/100;
  int counter=1;

	TCanvas* c_gain = new TCanvas("THGEM1 gain", "THGEM1 gain");
	TH1D* hist_gain = new TH1D("THGEM1 gain distribution", "THGEM1 gain distribution", 99, 0, 100);
	hist_gain->SetLineColor(kBlue+2);
	hist_gain->SetLineWidth(1);

  for (unsigned int i = 0; i < NUM; ++i) {
		if (NUM > 5) {
			if (i == NUM - 2)
				aval->EnablePlotting(viewDr);
		} else {
			aval->EnablePlotting(viewDr);
		}
		double x=-_X_SIZE+RndmUniform()*(_X_SIZE*2),y=-_Y_SIZE+_Y_SIZE*2*RndmUniform();
		aval->AvalancheElectron(x, y, -0.129, 0);
		int n_electrons = aval->GetNumberOfElectronEndpoints();
		hist_gain->Fill(n_electrons);
		if (interval*counter < i) {
			std::cout<<"Done "<<(int)(100.0*i/NUM)<<"%\n";
			++counter;
		}
  }
	c_gain->cd();
	hist_gain->Draw();
	//c_gain->Update();
  std::cout<<"vMsh->Plot(): ";
  std::cout<<vMsh->Plot()<<std::endl;

	ViewSignal signalView;
	signalView.SetSensor(sensor);
	signalView.PlotSignal("THGEM1");
	//signalView.GetHistogram()->SetLineWidth(1.5);
	//signalView.GetHistogram()->DrawCopy("same");

	//c_aval->Update();
  std::cout<<"app.Run"<<std::endl;
  app.Run(kTRUE);
  return 0;
}
