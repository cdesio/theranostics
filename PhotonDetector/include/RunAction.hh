#ifndef RUNACTION_HH
#define RUNACTION_HH


#include "DetectorConstruction.hh"
#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4MTRunManager.hh"
#include "G4AnalysisManager.hh"
#include "PrimaryGeneratorAction.hh"
#include "G4GenericAnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4GeneralParticleSource.hh" 
#include "G4VUserPrimaryGeneratorAction.hh"

class MyRunAction : public G4UserRunAction {

    public:
     MyRunAction();
     ~MyRunAction();

     virtual void BeginOfRunAction(const G4Run*);
     virtual void EndOfRunAction(const G4Run*);

	private:		


};






#endif
