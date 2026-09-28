#ifndef TRACKINFORMATION_HH
#define TRACKINFORMATION_HH
#include "G4VUserTrackInformation.hh"
#include "G4Track.hh"
#include <vector>

class MyTrackInformation : public G4VUserTrackInformation{
	public:
		
		MyTrackInformation(const G4Track* aTrack);
		~MyTrackInformation();
		G4int GetTrackID(){return fOriginalTrackID;}
		G4String GetParentParticle(){return parentParticle;}
		G4int GetTrackingStatus() const {return fTrackingStatus;}
	private:
		G4int fOriginalTrackID;  // Track ID of primary particle
		G4String parentParticle;
		G4int fTrackingStatus;
};

#endif

