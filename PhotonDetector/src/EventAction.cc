#include "EventAction.hh"

#include "G4Event.hh"

MyEventAction::MyEventAction(MyRunAction*) {}

MyEventAction::~MyEventAction() = default;

void MyEventAction::BeginOfEventAction(const G4Event*)
{
    parentParticle.clear();
    loggedPhotonBoundaries.clear();
}

void MyEventAction::EndOfEventAction(const G4Event*)
{
    parentParticle.clear();
    loggedPhotonBoundaries.clear();
}
