#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4VModularPhysicsList.hh"
#include "G4EmStandardPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmPenelopePhysics.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4ProductionCutsTable.hh"
#include "G4SystemOfUnits.hh"

class MyPhysicsList:public G4VModularPhysicsList{

	public:
		MyPhysicsList();
		~MyPhysicsList();


};



#endif
