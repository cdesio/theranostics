#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "StackingAction.hh"






MyActionInitialization::MyActionInitialization(const G4String& outputFileName, G4bool debug, G4int trackGlueCopyNo)
    : fOutputFileName(outputFileName), fDebug(debug), fTrackGlueCopyNo(trackGlueCopyNo) {}
MyActionInitialization::~MyActionInitialization(){}

void MyActionInitialization::Build() const{

      MyPrimaryGeneratorAction *generator = new MyPrimaryGeneratorAction ();
     SetUserAction(generator);

     MyRunAction *runAction = new MyRunAction();
	SetUserAction(runAction);

	MyEventAction *eventAction=new MyEventAction(runAction);
	SetUserAction(eventAction);

	MySteppingAction *steppingAction=new MySteppingAction(eventAction, fOutputFileName, fDebug, fTrackGlueCopyNo);
	SetUserAction(steppingAction);
	
	MyStackingAction *stackingAction=new MyStackingAction();
	SetUserAction(stackingAction);



}
void MyActionInitialization::BuildForMaster() const{
 
     MyRunAction *runAction = new MyRunAction();
     SetUserAction(runAction);
}
