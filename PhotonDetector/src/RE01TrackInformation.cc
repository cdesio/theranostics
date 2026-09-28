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
/// \file runAndEvent/RE01/src/RE01TrackInformation.cc
/// \brief Implementation of the RE01TrackInformation class
//
//
//

#include "RE01TrackInformation.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"    

G4ThreadLocal G4Allocator<RE01TrackInformation> *
                                   aTrackInformationAllocator = 0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
RE01TrackInformation::RE01TrackInformation()
  : G4VUserTrackInformation()
{
    fTrackID = 0;
    fParticleDefinition = 0;
    fSourceTrackID = -1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
RE01TrackInformation::RE01TrackInformation(const G4Track* aTrack)
  : G4VUserTrackInformation()
{
    fTrackID = aTrack->GetTrackID();
    fParticleDefinition = aTrack->GetDefinition();
    fSourceTrackID = -1;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
RE01TrackInformation
::RE01TrackInformation(const RE01TrackInformation* aTrackInfo)
  : G4VUserTrackInformation()
{
    fTrackID = aTrackInfo->fTrackID;
    fParticleDefinition = aTrackInfo->fParticleDefinition;
    fSourceTrackID = aTrackInfo->fSourceTrackID;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
RE01TrackInformation::~RE01TrackInformation()
{;}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
RE01TrackInformation& RE01TrackInformation
::operator =(const RE01TrackInformation& aTrackInfo)
{
    fTrackID = aTrackInfo.fTrackID;
    fParticleDefinition = aTrackInfo.fParticleDefinition;
    fSourceTrackID = aTrackInfo.fSourceTrackID;

    return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void RE01TrackInformation::SetSourceTrackInformation(const G4Track* aTrack)
{
    fSourceTrackID = aTrack->GetParentID();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
