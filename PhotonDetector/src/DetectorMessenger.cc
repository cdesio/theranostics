//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
/// \file DetectorMessenger.cc
/// \brief Implementation of the DetectorMessenger class
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorMessenger.hh"

#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAnInteger.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::DetectorMessenger(MyDetectorConstruction* det)
    : G4UImessenger(),
      fDetector(det),
      yGlue(nullptr),
      detectorDistance(nullptr),
      xviScintillatorThickness(nullptr)
{
  yGlue = new G4UIcmdWithADoubleAndUnit("/det/set_yGlue",this);
  yGlue->SetGuidance("Set y size of the glue");
  yGlue->SetParameterName("Size",false);
  yGlue->SetRange("Size>0.");
  yGlue->SetUnitCategory("Length");
  yGlue->AvailableForStates(G4State_PreInit,G4State_Idle);
  yGlue->SetToBeBroadcasted(false);

  detectorDistance = new G4UIcmdWithADoubleAndUnit("/det/set_detectorDistance",this);
  detectorDistance->SetGuidance("Set the gap from the water-box surface to the photon detector plates.");
  detectorDistance->SetParameterName("Distance",false);
  detectorDistance->SetRange("Distance>0.");
  detectorDistance->SetUnitCategory("Length");
  detectorDistance->AvailableForStates(G4State_PreInit,G4State_Idle);
  detectorDistance->SetToBeBroadcasted(false);

  xviScintillatorThickness =
      new G4UIcmdWithADoubleAndUnit("/det/set_xviScintillatorThickness",this);
  xviScintillatorThickness->SetGuidance("Set the full thickness of the XVI CsI layer.");
  xviScintillatorThickness->SetParameterName("Thickness",false);
  xviScintillatorThickness->SetRange("Thickness>0.");
  xviScintillatorThickness->SetUnitCategory("Length");
  xviScintillatorThickness->AvailableForStates(G4State_PreInit,G4State_Idle);
  xviScintillatorThickness->SetToBeBroadcasted(false);
  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorMessenger::~DetectorMessenger()
{

  delete yGlue;
  delete detectorDistance;
  delete xviScintillatorThickness;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{ 

  if( command == yGlue )
  {
     fDetector->set_yGlue(yGlue->GetNewDoubleValue(newValue));
  }
  else if( command == detectorDistance )
  {
     fDetector->SetDetectorDistance(detectorDistance->GetNewDoubleValue(newValue));
  }
  else if( command == xviScintillatorThickness )
  {
     fDetector->SetXVIScintillatorThickness(
         xviScintillatorThickness->GetNewDoubleValue(newValue));
  }

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
