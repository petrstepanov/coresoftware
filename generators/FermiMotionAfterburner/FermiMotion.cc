// Discribtion:Thie code is used to add Fermimotion p_F to spectator neutrons
//Modified from the flowafterburner code, thx!
//
//AuthorList:
// initial code 2020

//include the header file here
#include "FermiMotion.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>


#include <HepMC/GenEvent.h>
#include <HepMC/GenParticle.h>  // for GenParticle
#include <HepMC/GenRanges.h>
#include <HepMC/GenVertex.h>      // for GenVertex, GenVertex::part...
#include <HepMC/HeavyIon.h>       // for HeavyIon
#include <HepMC/IteratorRange.h>  // for children, descendants
#include <HepMC/SimpleVector.h>   // for FourVector

#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Vector/LorentzVector.h>

#include <cmath>
#include <map>  // for map
#include <iostream>
#include <iterator>                           // for operator!=, reverse_ite...
#include <set>                                // for set, _Rb_tree_const_ite...
#include <string>
#include <utility>


//____________________________________________________________________________..


using namespace HepMC;
namespace CLHEP
{
class HepRandomEngine;
}
namespace HepMC { class GenEvent; }
//this method is use to find out the spectator neutron loss prob
//using the parameterization in the PHENIX Glauber
//Monte Carlo code written by Klaus Reygers to model 
//the loss of forward
//neutrons into the ZDC due to larger fragments

//Assume Au for now
//make sure b is in fm
double ploss(double b){
  // para
  double p0 = 0.3305;
  double p1 = 0.0127;
  double p2 = 17.;
  double p3 = 2.;
  double ploss = p0+b*p1+exp((b-p2)/p3);
  
  return ploss;
}

//this method is use to generate and random p_F
//along a random direction and add it to the momentum
//assume Au for now
CLHEP::HepLorentzVector pwithpF(CLHEP::HepLorentzVector p,CLHEP::HepRandomEngine *engine, int id){
  //id should be either 2112 or 2212
  if( ! ( (id == 2112)||(id==2212) ) ){
    std::cout<<"wrong pid"<<std::endl;
    return p;
  }
  //find pF max using Thomas-Fermi model, assume using Au.
  float pFmax=0.28315;
  if(id==2212) pFmax=0.23276;
  //now generate the random p assuming probability is propotional to p^2dp
  //CLHEP::RandGeneral seems to be a better way to do it
  float pF=pFmax*pow(CLHEP::RandFlat::shoot(engine),1.0/3.0);
  float cotheta=(CLHEP::RandFlat::shoot(engine)-0.5) * 2;
  float phi=CLHEP::RandFlat::shoot(engine) * 2 * M_PI;
  float pFx=pF * sqrt(1-cotheta * cotheta) * cos(phi);
  float pFy=pF * sqrt(1-cotheta * cotheta) * sin(phi);
  float pFz=pF * cotheta;
  //now add the pF to p
  float px=p.px()+pFx;
  float py=p.py()+pFy;
  float pz=p.pz()+pFz;
  //calculate the total energy
  float const nrm=0.938;
  float e=sqrt(px*px+py*py+pz*pz + nrm*nrm);

  CLHEP::HepLorentzVector pwithpF(px,py,pz,e);
  return pwithpF;
}


int FermiMotion (HepMC::GenEvent *event, CLHEP::HepRandomEngine *engine){
  //find ploss
  //std::cout<<"getting b"<<std::endl;
  HepMC::HeavyIon *hi = event->heavy_ion();
  double b = hi->impact_parameter();
  double pnl=ploss(b);
  //now loop over all particles and find spectators

 std::cout<<"looping over particles"<<std::endl;
 for ( GenEvent::particle_const_iterator p = event->particles_begin(),prev=event->particles_end();  p != event->particles_end(); prev=p, ++p ){
    int id=(*p)->pdg_id();
    bool havedelete=false;
    //if not neutron or proton, skip
    if( ! ( (id == 2112)||(id==2212) ) ) continue;

    //spectator neutron&protons should have px==0&&py==0
    HepMC::GenParticle *n = (*p);
    float p_x=n->momentum().px();
    float p_y=n->momentum().py();
    if(!(p_x==0&&p_y==0 )) continue;
  
    //remove neutrons bound to large fragment    
    if(id==2112){

      if(pnl>CLHEP::RandFlat::shoot(engine)){
	//remove particle here
	
	((*p)->production_vertex())->remove_particle(*p);
	
	havedelete=true;
      }
      
    }
    
    //add pF to the remaining
    
   CLHEP::HepLorentzVector p0(n->momentum().px(),n->momentum().py(),n->momentum().pz(),n->momentum().e());
   CLHEP::HepLorentzVector newp= pwithpF( p0, engine, id);
   (*p)->set_momentum(newp );   
   //remember the index go down one after remove
   
   if(havedelete){
     if(prev != event->particles_end()) p=prev;
   } 
 }
 
 
 return 0;
}








