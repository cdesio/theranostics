#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"
#include "G4Geantino.hh"
#include "G4IonTable.hh"
#include "G4GenericMessenger.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleDefinition.hh"
class MyPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
    public:
        MyPrimaryGeneratorAction();
       ~MyPrimaryGeneratorAction();

       virtual void GeneratePrimaries(G4Event* anEvent);
		    G4GeneralParticleSource*  GetParticleGun() const { return particleSource; }
    
    private:
        G4GeneralParticleSource* particleSource;
};

#endif
