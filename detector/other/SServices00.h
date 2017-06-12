#include <utility>
#include <vector>

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
 
using namespace std;
using namespace DD4hep;
using namespace dd4hep;
using namespace DD4hep::Geometry;

//=====================================
// class for  BuildTPCEndplateServices
//=====================================
//using vector of double pair to hold the value 
//of "tpcEndplateServicesRing_R" and "tpcEndplateServicesRing_ro".
//Which provide more flexible for handling the modification in the number
//of TPC cooling rings. This class can handle different number of rings.

class BuildTPCEndplateServices{
public:
  BuildTPCEndplateServices(){};
  ~BuildTPCEndplateServices(){};

  void setMaterial (Material copper4cooling) {cooling_Material = copper4cooling;};
  void sethalfZ (double TPC_Ecal_Hcal_barrel_halfZ){TPC_barrel_halfZ = TPC_Ecal_Hcal_barrel_halfZ ;};
  void settpcEndplateServicesRing_R_ro( double tpcEndplateServicesRing_R, double tpcEndplateServicesRing_ro) {
    tpcEndplateServicesRing_R_ro.push_back(make_pair(tpcEndplateServicesRing_R,tpcEndplateServicesRing_ro));
  };

  // to build TPC cooling rings into the service assembly 
  bool DoBuildTPCEndplateServices(PlacedVolume &pVol,Assembly &envelope){
    for( vector< pair<double,double> >::const_iterator it = tpcEndplateServicesRing_R_ro.begin() ; 
	 it != tpcEndplateServicesRing_R_ro.end(); ++it){
      Torus solidTube(it->first, 0, it->second, 0, 2*M_PI);
      double z_position = TPC_barrel_halfZ + it->second;
      Volume EnvTube("service_tube",solidTube,cooling_Material);
      //EnvTube.setVisAttributes("RedVis");
      Position pos1(0,0,z_position);
      pVol = envelope.placeVolume(EnvTube,pos1);
      Position pos2(0,0,-z_position);
      pVol = envelope.placeVolume(EnvTube,pos2);
    } 
    return true;
  };

private:
  Material cooling_Material;
  double TPC_barrel_halfZ;
  vector< pair<double,double> > tpcEndplateServicesRing_R_ro;  
};



//=====================================
// class for  BuildEcalBarrelServices
//=====================================
class BuildEcalBarrelServices {

public:
  BuildEcalBarrelServices(){};
  ~BuildEcalBarrelServices(){};

  void setMaterialAir (Material air4container) {air = air4container;};
  void setMaterialAluminium (Material al) {aluminium = al;};
  void setMaterialPolyethylene (Material PE) {polyethylene = PE;};
  void setMaterialCopper (Material Cu) {copper = Cu;};

  void sethalfZ (double TPC_Ecal_Hcal_barrel_halfZ){Ecal_barrel_halfZ = TPC_Ecal_Hcal_barrel_halfZ ;};
  void setTopDimX (double dim_x){top_dim_x = dim_x ;};
  void setRailHeight (double height){RailHeight = height ;};
  void setOutRadius (double outer_radius){Ecal_outer_radius = outer_radius ;};
  void setModuleThickness (double thickness){ module_thickness = thickness ;};

  void setRailSeparation (double separation) {RailSeparation = separation;};
  void setRailWidth (double width) {RailWidth = width;};

  void setZMinus_FirstInterrail_PE_Thickness  (double PE_Thickness_ZMF) {ZMinus_FirstInterrail_PE_Thickness = PE_Thickness_ZMF;};
  void setZMinus_FirstInterrail_Cu_Thicknesst  (double Cu_Thickness_ZMF) {ZMinus_FirstInterrail_Cu_Thickness = Cu_Thickness_ZMF;};

  void setZPlus_FirstInterrail_PE_Thickness  (double PE_Thickness_ZPF) {ZPlus_FirstInterrail_PE_Thickness = PE_Thickness_ZPF;};
  void setZPlus_FirstInterrail_Cu_Thickness  (double Cu_Thickness_ZPF) {ZPlus_FirstInterrail_Cu_Thickness = Cu_Thickness_ZPF;};

  void setRailSeparationChanged(void) {RailSeparationChanged = false;};

  // to fill the detail service layers into the container
  bool FillEcalBarrelServicesContainer(PlacedVolume &pVol, Volume &pContainerLogical){

    const int NRAILS=2;
    if ( RailSeparation*(NRAILS-1) > top_dim_x - RailWidth ) {
      cout << "WARNING, requested rail separation too large! " << endl;
      cout << "Must be smaller than " << (top_dim_x - RailWidth)/(NRAILS-1) << " cm" << endl;
      assert(0);
    }

    float railPositions[NRAILS]; // positions wrt module centre: equally spaced, centered on module
    for (int i=0; i<NRAILS; i++) {
      railPositions[i] = i*RailSeparation - RailSeparation*(NRAILS-1)/2.;
    }
   
    Box railSolid(RailWidth/2., RailHeight/2., Ecal_barrel_halfZ); 

    Volume railLogical("railLogical", railSolid, aluminium);

    for(int i=0; i<NRAILS; i++)
      {
	Position pos(railPositions[i],0,0);
	pVol = pContainerLogical.placeVolume(railLogical,pos);
      }
    
  double moduleLength = Ecal_barrel_halfZ * 2. / 5.;

  double internalZoneWidth = RailSeparation - RailWidth - 0.1; // space between rails (in x-dir) (minus a small gap)

  // now fit fit service materials into the zone between rails

  const int NMODULES_IN_Z=5;
  for(int i=0; i<NMODULES_IN_Z; i++){ // these are the 5 modules in z
    
    float pe_thick = ZMinus_FirstInterrail_PE_Thickness;
    float cu_thick = ZMinus_FirstInterrail_Cu_Thickness;
    if ( i>2 ) {
      pe_thick = ZPlus_FirstInterrail_PE_Thickness;
      cu_thick = ZPlus_FirstInterrail_Cu_Thickness;
    }

    // polyethylene
    Box PESolid( internalZoneWidth/2. , pe_thick / 2., moduleLength / 2.); 
    
    string PELogical_name  = "PELogical"+_toString(i,"_%d");
    Volume PELogical(PELogical_name,PESolid,polyethylene);
    //PELogical.setVisAttributes("GreenVis");

    Position posPE( 0 , 
		    -RailHeight/2. + pe_thick/2.,
		    -Ecal_barrel_halfZ + moduleLength*(i+1/2.));
    
    pVol = pContainerLogical.placeVolume(PELogical,posPE);
    
    // Cu_1
    Box Cu_1_Solid( internalZoneWidth/2., cu_thick / 2., moduleLength / 2.); 
    
    string Cu_1_Logical_name  = "Cu_1_Logical"+_toString(i,"_%d");
    Volume Cu_1_Logical(Cu_1_Logical_name, Cu_1_Solid,copper);
    //Cu_1_Logical.setVisAttributes("BlueVis");

    Position posCu1( 0 , 
		     -RailHeight/2. + pe_thick + cu_thick/2.,
		     -Ecal_barrel_halfZ + moduleLength*(i+1/2.));
    
    pVol = pContainerLogical.placeVolume(Cu_1_Logical,posCu1);

  }

    return true;
  };

  // to build Ecal Barrel service into the service assembly 
  bool DoBuildEcalBarrelServices(PlacedVolume &pVol,Assembly &envelope){

    Box ContainerSolid(top_dim_x/2., RailHeight/2., Ecal_barrel_halfZ); 
    Volume containerLogical("EcalBarrelServicesContainerLogical",ContainerSolid,air);
    //containerLogical.setVisAttributes("SeeThrough");
    //containerLogical.setVisAttributes("MagentaVis");

    if(!FillEcalBarrelServicesContainer(pVol,containerLogical))
      return false;

    for (int stave_id = 1; stave_id < 9 ; stave_id++)
      {
	double phirot = (stave_id-1) * M_PI/4.;
	RotationZYX rot(-phirot,0,0);
	//Position  stavePosition(module_thickness * sin(M_PI/4.), Ecal_outer_radius + RailHeight/2., 0);
	Position  stavePosition( (Ecal_outer_radius + RailHeight/2.)*sin(phirot) - module_thickness * sin(M_PI/4.)*cos(-phirot), 
				 (Ecal_outer_radius + RailHeight/2.)*cos(phirot) - module_thickness * sin(M_PI/4.)*sin(-phirot), 
				0 );
	Transform3D tran3D(rot,stavePosition);
	pVol = envelope.placeVolume(containerLogical,tran3D);
      }
    
    return true;
  };

private:
  double Ecal_barrel_halfZ;
  double top_dim_x;
  double RailHeight;
  double Ecal_outer_radius;
  double module_thickness;

  //  double RailDistanceToRight;
  double RailSeparation;
  double RailWidth;
  bool   RailSeparationChanged;

//-Z thicknesses
  double ZMinus_FirstInterrail_PE_Thickness;
  double ZMinus_FirstInterrail_Cu_Thickness;

//+Z thicknesses 
  double ZPlus_FirstInterrail_PE_Thickness;
  double ZPlus_FirstInterrail_Cu_Thickness;

  Material air;
  Material aluminium;
  Material polyethylene;
  Material copper;

};






//=====================================
// class for BuildEcalBarrel_EndCapServices
//=====================================
class BuildEcalBarrel_EndCapServices {

public:
  BuildEcalBarrel_EndCapServices(){};
  ~BuildEcalBarrel_EndCapServices(){};

  void setMaterialAir (Material air4container) {air = air4container;};
  void setMaterialPolyethylene (Material PE) {polyethylene = PE;};
  void setMaterialCopper (Material Cu) {copper = Cu;};

  void sethalfZ (double TPC_Ecal_Hcal_barrel_halfZ){Ecal_barrel_halfZ = TPC_Ecal_Hcal_barrel_halfZ ;};
  void setTopDimX (double dim_x){top_dim_x = dim_x ;};
  void setModuleThickness (double thickness){ module_thickness = thickness ;};
  void setInnerServicesWidth (double width){InnerServicesWidth = width;};
  void setEcal_cables_gap(double gap){Ecal_cables_gap = gap;};
  void setOutRadius (double outer_radius){Ecal_outer_radius = outer_radius ;};

  void setZMinus_PE_Thickness (double PE_Thickness_ZM) {ZMinus_PE_Thickness = PE_Thickness_ZM;};
  void setZMinus_Cu_Thickness (double Cu_Thickness_ZM) {ZMinus_Cu_Thickness = Cu_Thickness_ZM;};

  void setZPlus_PE_Thickness (double PE_Thickness_ZP) {ZPlus_PE_Thickness = PE_Thickness_ZP;};
  void setZPlus_Cu_Thickness (double Cu_Thickness_ZP) {ZPlus_Cu_Thickness = Cu_Thickness_ZP;};


  // to build Ecal Barrel service into the service assembly 
  bool DoBuildEcalBarrel_EndCapServices(PlacedVolume &pVol,Assembly &envelope){

    double containerThickness = ZMinus_PE_Thickness + ZMinus_Cu_Thickness;
    double z_position = -Ecal_barrel_halfZ -containerThickness/2.;
    double container_x_dim = top_dim_x/2. + module_thickness*sin(pi/4.) - InnerServicesWidth/2.;
    double Cu_Thickness = ZMinus_Cu_Thickness;
    double PE_Thickness = ZMinus_PE_Thickness;

    for(int i=0; i<=1; i++){

      if(Ecal_cables_gap < containerThickness) return false;
	//Control::Abort(
	//	       "EcalBarrel_EndCap services: Cu+PE thicknesses exceed Ecal_cables_gap",
	//	       MOKKA_ERROR_BAD_DATABASE_PARAMETERS);

      Box ContainerSolid(container_x_dim/2., module_thickness/2., containerThickness/2.); 

      string containerLogical_name  = "containerLogical"+_toString(i,"_%d");
      Volume containerLogical(containerLogical_name,ContainerSolid,air);


      Box PESolid(container_x_dim/2., module_thickness/2., PE_Thickness/2.);
 
      string PELogical_name  = "PELogical"+_toString(i,"_%d");
      Volume PELogical(PELogical_name,PESolid,polyethylene);

      Position  PosPE(0,0,containerThickness/2. - PE_Thickness/2.);
      
      pVol = containerLogical.placeVolume(PELogical,PosPE);
      
      Box Cu_Solid(container_x_dim/2., module_thickness/2., Cu_Thickness/2.); 
      
      string Cu_Logical_name  = "Cu_Logical"+_toString(i,"_%d");
      Volume Cu_Logical(Cu_Logical_name,Cu_Solid,copper);
      
      Position  PosCu(0,0,-containerThickness/2. + Cu_Thickness/2.);
      
      pVol = containerLogical.placeVolume(Cu_Logical,PosCu);

      
      for (int stave_id = 1; stave_id < 9 ; stave_id++){
	
	double phirot = (stave_id-1) * M_PI/4;
	
	RotationZYX rot;
	Position stavePosition;
	
        if(z_position > 0) {
	  RotationZYX rot_P(-phirot,0,0);
	  rot = rot_P;
	  Position stavePosition_P((Ecal_outer_radius - module_thickness/2.)*sin(phirot)+(InnerServicesWidth/2. + container_x_dim/2.)*cos(-phirot),
				   (Ecal_outer_radius - module_thickness/2.)*cos(phirot)+(InnerServicesWidth/2. + container_x_dim/2.)*sin(-phirot), 
				   z_position);

	  stavePosition = stavePosition_P;
	}
	else
	  {
	    RotationZYX rot_M(phirot,M_PI,0);
	    rot = rot_M;
	    Position stavePosition_M((Ecal_outer_radius - module_thickness/2.)*sin(phirot)+(-InnerServicesWidth/2. - container_x_dim/2.)*cos(-phirot),
				     (Ecal_outer_radius - module_thickness/2.)*cos(phirot)+(-InnerServicesWidth/2. - container_x_dim/2.)*sin(-phirot),
				     z_position);
	    
	    stavePosition = stavePosition_M;
	  }
	
	Transform3D tran3D(rot,stavePosition);
	pVol = envelope.placeVolume(containerLogical,tran3D);
	
      }
      
      containerThickness = ZPlus_PE_Thickness + ZPlus_Cu_Thickness;
      z_position = Ecal_barrel_halfZ + containerThickness/2.;
      
      Cu_Thickness = ZPlus_Cu_Thickness;
      PE_Thickness = ZPlus_PE_Thickness;
      
    }


    return true;
  };


private:
  double Ecal_barrel_halfZ;
  double top_dim_x;
  double module_thickness;
  double InnerServicesWidth;
  double Ecal_cables_gap;
  double Ecal_outer_radius;

//-Z thicknesses
  double ZMinus_PE_Thickness;
  double ZMinus_Cu_Thickness;

//+Z thicknesses 
  double ZPlus_PE_Thickness;
  double ZPlus_Cu_Thickness;

  Material air;
  Material polyethylene;
  Material copper;


};
