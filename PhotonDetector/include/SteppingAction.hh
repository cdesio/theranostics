#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "EventAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4UserSteppingAction.hh"
#include "globals.hh"

#include <fstream>
#include <map>

class MyDetectorConstruction;

class MySteppingAction : public G4UserSteppingAction
{
  public:
    MySteppingAction(MyEventAction* eventAction,
                     const G4String& outputFileName,
                     G4bool debug,
                     G4int trackGlueCopyNo);
    ~MySteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

  private:
    void EnsureOriginAssigned(const G4Track* track);
    void LogPhotonBoundary(const G4Step* step,
                           const G4String& boundaryTag,
                           const G4String& volumeName,
                           const G4StepPoint* point) const;
    G4String BuildPhotonBoundaryKey(G4int eventID,
                                    G4int trackID,
                                    G4int stepNumber,
                                    const G4String& boundaryTag,
                                    const G4String& volumeName) const;
    G4bool IsTrackedPhotonVolume(const G4String& volumeName) const;
    G4String ResolveOriginLabel(G4int originCode) const;
    G4String ResolveOriginIsotope(G4int originCode) const;

    MyEventAction* fEventAction = nullptr;
    const MyDetectorConstruction* fDetector = nullptr;
    G4bool fDebug = false;
    G4int fTrackGlueCopyNo = -1;

    mutable std::ofstream fPhotonBoundaryLog;

    std::map<G4String, G4int> particleOriginMap{
        {"Ra224", 1},
        {"Rn220", 2},
        {"Po216", 3},
        {"Pb212", 4},
        {"Bi212", 5},
        {"Tl208", 6},
        {"Po212", 7},
        {"Pb208", 8},
        {"alphaRa224", 9},
        {"alphaRn220", 10},
        {"alphaPo216", 11},
        {"alphaBi212", 12},
        {"alphaPo212", 13},
        {"e-Rn220", 14},
        {"e-Po216", 15},
        {"e-Pb212", 16},
        {"e-Bi212", 17},
        {"e-Tl208", 18},
        {"e-Po212", 19},
        {"e-Pb208", 20},
        {"gammaRn220", 21},
        {"gammaPo216", 22},
        {"gammaPb212", 23},
        {"gammaBi212", 24},
        {"gammaTl208", 25},
        {"gammaPo212", 26},
        {"gammaPb208", 27},
        {"Ra226", 43},
        {"Rn222", 44},
        {"Po218", 45},
        {"Pb214", 46},
        {"Bi214", 47},
        {"Po214", 48},
        {"Pb210", 49},
        {"Bi210", 50},
        {"Po210", 51},
        {"Pb206", 52},
        {"Tl210", 81},
        {"Tl206", 82},
        {"Hg206", 83},
        {"At218", 90},
        {"Rn218", 91},
        {"alphaRa226", 53},
        {"alphaRn222", 54},
        {"alphaPo218", 55},
        {"alphaPo214", 56},
        {"alphaPo210", 57},
        {"e-Ra226", 58},
        {"e-Rn222", 59},
        {"e-Po218", 60},
        {"e-Pb214", 61},
        {"e-Bi214", 62},
        {"e-Po214", 63},
        {"e-Pb210", 64},
        {"e-Bi210", 65},
        {"e-Po210", 66},
        {"e-Pb206", 67},
        {"e-Tl210", 84},
        {"e-Tl206", 85},
        {"e-Hg206", 86},
        {"e-At218", 92},
        {"e-Rn218", 93},
        {"gammaRa226", 68},
        {"gammaRn222", 69},
        {"gammaPo218", 70},
        {"gammaPb214", 71},
        {"gammaBi214", 72},
        {"gammaPo214", 73},
        {"gammaPb210", 74},
        {"gammaBi210", 75},
        {"gammaPo210", 76},
        {"gammaPb206", 77},
        {"gammaTl210", 87},
        {"gammaTl206", 88},
        {"gammaHg206", 89},
        {"gammaAt218", 94},
        {"gammaRn218", 95},
        {"e+", 28},
        {"gamma", 79}};

    std::map<G4int, G4String> reverseParticleOriginMap{
        {1, "Ra224"},
        {2, "Rn220"},
        {3, "Po216"},
        {4, "Pb212"},
        {5, "Bi212"},
        {6, "Tl208"},
        {7, "Po212"},
        {8, "Pb208"},
        {43, "Ra226"},
        {44, "Rn222"},
        {45, "Po218"},
        {46, "Pb214"},
        {47, "Bi214"},
        {48, "Po214"},
        {49, "Pb210"},
        {50, "Bi210"},
        {51, "Po210"},
        {52, "Pb206"},
        {81, "Tl210"},
        {82, "Tl206"},
        {83, "Hg206"},
        {90, "At218"},
        {91, "Rn218"}};

    std::map<G4int, G4String> originCodeToLabel{
        {1, "Ra224"},
        {2, "Rn220"},
        {3, "Po216"},
        {4, "Pb212"},
        {5, "Bi212"},
        {6, "Tl208"},
        {7, "Po212"},
        {8, "Pb208"},
        {9, "alphaRa224"},
        {10, "alphaRn220"},
        {11, "alphaPo216"},
        {12, "alphaBi212"},
        {13, "alphaPo212"},
        {14, "e-Rn220"},
        {15, "e-Po216"},
        {16, "e-Pb212"},
        {17, "e-Bi212"},
        {18, "e-Tl208"},
        {19, "e-Po212"},
        {20, "e-Pb208"},
        {21, "gammaRn220"},
        {22, "gammaPo216"},
        {23, "gammaPb212"},
        {24, "gammaBi212"},
        {25, "gammaTl208"},
        {26, "gammaPo212"},
        {27, "gammaPb208"},
        {28, "e+"},
        {43, "Ra226"},
        {44, "Rn222"},
        {45, "Po218"},
        {46, "Pb214"},
        {47, "Bi214"},
        {48, "Po214"},
        {49, "Pb210"},
        {50, "Bi210"},
        {51, "Po210"},
        {52, "Pb206"},
        {53, "alphaRa226"},
        {54, "alphaRn222"},
        {55, "alphaPo218"},
        {56, "alphaPo214"},
        {57, "alphaPo210"},
        {58, "e-Ra226"},
        {59, "e-Rn222"},
        {60, "e-Po218"},
        {61, "e-Pb214"},
        {62, "e-Bi214"},
        {63, "e-Po214"},
        {64, "e-Pb210"},
        {65, "e-Bi210"},
        {66, "e-Po210"},
        {67, "e-Pb206"},
        {68, "gammaRa226"},
        {69, "gammaRn222"},
        {70, "gammaPo218"},
        {71, "gammaPb214"},
        {72, "gammaBi214"},
        {73, "gammaPo214"},
        {74, "gammaPb210"},
        {75, "gammaBi210"},
        {76, "gammaPo210"},
        {77, "gammaPb206"},
        {79, "gamma"},
        {81, "Tl210"},
        {82, "Tl206"},
        {83, "Hg206"},
        {84, "e-Tl210"},
        {85, "e-Tl206"},
        {86, "e-Hg206"},
        {87, "gammaTl210"},
        {88, "gammaTl206"},
        {89, "gammaHg206"},
        {90, "At218"},
        {91, "Rn218"},
        {92, "e-At218"},
        {93, "e-Rn218"},
        {94, "gammaAt218"},
        {95, "gammaRn218"}};
};

#endif
