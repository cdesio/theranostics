#include "StackingAction.hh"
#include "G4Track.hh"
#include "G4NeutrinoE.hh"

MyStackingAction::MyStackingAction(){ }
MyStackingAction::~MyStackingAction(){ }
G4ClassificationOfNewTrack MyStackingAction::ClassifyNewTrack(const G4Track* track){
	RE01TrackInformation* trackInfo=new RE01TrackInformation(track);
	trackInfo->SetSourceTrackInformation(track);
	track->SetUserInformation(trackInfo);

	return fUrgent;
 }
