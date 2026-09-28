#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "G4ThreeVector.hh"
#include "G4UserEventAction.hh"

#include <map>
#include <set>
#include <string>

class G4Event;
class MyRunAction;

class MyEventAction : public G4UserEventAction
{
  public:
    explicit MyEventAction(MyRunAction*);
    ~MyEventAction() override;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    std::map<G4int, G4int> parentParticle;
    std::set<std::string> loggedPhotonBoundaries;
};

#endif
