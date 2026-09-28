#include "PrimaryGeneratorAction.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

MyPrimaryGeneratorAction::MyPrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction()
{
 particleSource = new G4GeneralParticleSource();   
}

MyPrimaryGeneratorAction::~MyPrimaryGeneratorAction()
{
	delete particleSource;

}

void MyPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	particleSource->GeneratePrimaryVertex(anEvent);

  // Debug marker: draw a small red circle at the primary vertex in the GUI.
  G4VVisManager* visManager = G4VVisManager::GetConcreteInstance();
  if (visManager) {
    G4ThreeVector pos = particleSource->GetParticlePosition();
    G4Circle marker(pos);
    marker.SetScreenSize(6.0);
    marker.SetFillStyle(G4Circle::filled);
    G4VisAttributes attribs(G4Colour(1.0, 0.0, 0.0));
    marker.SetVisAttributes(attribs);
    visManager->Draw(marker);
  }

}
