#ifndef StackingAction_h
#define StackingAction_h 

#include "G4UserStackingAction.hh"
#include "globals.hh"
#include "EventAction.hh"
#include "RE01TrackInformation.hh"
class MyStackingAction : public G4UserStackingAction{
  public:
      MyStackingAction();
      ~MyStackingAction();
              
      virtual G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track*);        
};


#endif
