#include "TrackInformation.hh"


MyTrackInformation::MyTrackInformation(const G4Track* aTrack)
  : G4VUserTrackInformation()
{
    fOriginalTrackID = aTrack->GetTrackID();
fTrackingStatus=0;	
}
MyTrackInformation::~MyTrackInformation(){}
