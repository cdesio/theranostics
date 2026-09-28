#include "PhysicsList.hh"

MyPhysicsList::MyPhysicsList(){

	RegisterPhysics(new G4EmPenelopePhysics());// change it from G4EmPenelopePhysics to G4EmStandardPhysics_option4
	RegisterPhysics(new G4DecayPhysics());
	RegisterPhysics(new G4RadioactiveDecayPhysics());
	G4StepLimiterPhysics* stepLimitPhys = new G4StepLimiterPhysics();
   	stepLimitPhys->SetApplyToAll(true);
   	RegisterPhysics(stepLimitPhys);
	G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(100*eV, 1*GeV);
	SetDefaultCutValue(1*nm); // This means particles will only be generated if they have enough energy to travel at least 1 nanometer before stopping.
}
MyPhysicsList::~MyPhysicsList(){}
