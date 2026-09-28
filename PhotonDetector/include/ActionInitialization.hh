#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "globals.hh"
#include "G4VUserActionInitialization.hh"

class MyActionInitialization : public G4VUserActionInitialization
{
public:
    explicit MyActionInitialization(const G4String& outputFileName, G4bool debug, G4int trackGlueCopyNo);
     ~MyActionInitialization();
    //  ~MyActionInitialization();
    // virtual void Build() const;


    virtual void BuildForMaster() const override;
    virtual void Build() const override;

private:
    G4String fOutputFileName;
    G4bool fDebug = false;
    G4int fTrackGlueCopyNo = -1;
};

#endif
