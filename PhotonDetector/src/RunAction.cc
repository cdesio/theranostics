#include "RunAction.hh"
#include "G4ios.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "G4SDManager.hh"
#include "G4Threading.hh"
#include "XVIReadoutSD.hh"

MyRunAction::MyRunAction(const G4String& outputFileName)
    : fOutputFileName(outputFileName) {

  G4AnalysisManager *analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetDefaultFileType("root");
  analysisManager->SetVerboseLevel(0);
  analysisManager->SetNtupleMerging(true);
  analysisManager->SetFirstNtupleId(0);

}
MyRunAction::~MyRunAction() {}

void MyRunAction::BeginOfRunAction(const G4Run*) {
    G4AnalysisManager *man = G4AnalysisManager::Instance();

    auto* readout = dynamic_cast<XVIReadoutSD*>(
        G4SDManager::GetSDMpointer()->FindSensitiveDetector("/XVI/readout", false));
    if (readout != nullptr)
    {
        readout->Reset();
    }


 
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

    // In MT mode the master owns an empty SD instance. Only the worker may
    // write, otherwise the master's zero image overwrites the scored image.
    auto* readout = dynamic_cast<XVIReadoutSD*>(
        G4SDManager::GetSDMpointer()->FindSensitiveDetector("/XVI/readout", false));
    if ((!G4Threading::IsMultithreadedApplication() || !IsMaster()) &&
        readout != nullptr)
    {
        readout->WritePanelImages(fOutputFileName);
    }

    // Write and close file
    man->Write();
    man->CloseFile();
}
