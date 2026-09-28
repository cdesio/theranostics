#include "RunAction.hh"
#include "G4ios.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"

MyRunAction::MyRunAction() {

  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetVerboseLevel(0);
  analysisManager->SetNtupleMerging(true);
  analysisManager->SetFirstNtupleId(0);

}
MyRunAction::~MyRunAction() {}

void MyRunAction::BeginOfRunAction(const G4Run*) {
    G4AnalysisManager *man = G4AnalysisManager::Instance();


 
      // Your existing code for particle gun and position (optional)
       const MyPrimaryGeneratorAction* generatorAction = static_cast<const MyPrimaryGeneratorAction*>(
        G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
    G4GeneralParticleSource* particlegun;
    G4ThreeVector posIon;

       if (generatorAction) {
        particlegun = generatorAction->GetParticleGun();
        posIon = particlegun->GetParticlePosition();
        }
}

void MyRunAction::EndOfRunAction(const G4Run*) {

 
    G4AnalysisManager *man = G4AnalysisManager::Instance();
    const auto detConstruction = static_cast<const MyDetectorConstruction*>(
    G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    // Write and close file
    man->Write();
    man->CloseFile();
}
