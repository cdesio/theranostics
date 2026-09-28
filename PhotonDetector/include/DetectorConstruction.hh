#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

#include <array>

class DetectorMessenger;

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    static constexpr G4int kPhotonDetectorCount = 6;
    static constexpr G4int kXVIBinsPerAxis = 1024;

    MyDetectorConstruction();
    ~MyDetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    void set_yGlue(G4double value);
    void SetDetectorDistance(G4double value);
    void SetXVIScintillatorThickness(G4double value);

    G4double GetGlueHalfThickness() const { return yGlue; }
    G4double GetDetectorDistance() const { return detectorDistance; }
    G4double GetXVIScintillatorThickness() const {
        return 2.0 * xviScintillatorHalfThickness;
    }
    G4double GetGlueXMin() const { return glueCenter.x() - xGlue; }
    G4double GetGlueXMax() const { return glueCenter.x() + xGlue; }
    G4double GetGlueYMin() const { return glueCenter.y() - yGlue; }
    G4double GetGlueYMax() const { return glueCenter.y() + yGlue; }
    G4double GetGlueZMin() const { return glueCenter.z() - zGlue; }
    G4double GetGlueZMax() const { return glueCenter.z() + zGlue; }

    G4double GetWaterXMin() const { return -xWater; }
    G4double GetWaterXMax() const { return xWater; }
    G4double GetWaterYMin() const { return -yWater; }
    G4double GetWaterYMax() const { return yWater; }
    G4double GetWaterZMin() const { return -zWater; }
    G4double GetWaterZMax() const { return zWater; }

  private:
    void DefineMaterials();

    G4double xWorld;
    G4double yWorld;
    G4double zWorld;

    G4double xWater;
    G4double yWater;
    G4double zWater;

    G4double xGlue;
    G4double yGlue;
    G4double zGlue;
    G4ThreeVector glueCenter;

    G4double xviPanelHalfSize;
    G4double xviScintillatorHalfThickness;
    G4double detectorDistance;

    G4Box* solidWorld = nullptr;
    G4Box* solidWater = nullptr;
    G4Box* solidGlue = nullptr;
    std::array<G4Box*, kPhotonDetectorCount> solidPhotonDetectors{};

    G4LogicalVolume* logicWorld = nullptr;
    G4LogicalVolume* logicWater = nullptr;
    G4LogicalVolume* logicGlue = nullptr;
    std::array<G4LogicalVolume*, kPhotonDetectorCount> logicPhotonDetectors{};

    G4VPhysicalVolume* physWorld = nullptr;
    G4VPhysicalVolume* physWater = nullptr;
    G4VPhysicalVolume* physGlue = nullptr;
    std::array<G4VPhysicalVolume*, kPhotonDetectorCount> physPhotonDetectors{};

    G4Material* worldMat = nullptr;
    G4Material* waterMat = nullptr;
    G4Material* glueMat = nullptr;
    G4Material* xviScintillatorMat = nullptr;

    DetectorMessenger* MyDetectorMessenger = nullptr;
};

#endif
