#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"

#include "G4Colour.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4VisAttributes.hh"

#include <array>

MyDetectorConstruction::MyDetectorConstruction() : G4VUserDetectorConstruction()
{
    xWorld = 2 * m;
    yWorld = 2 * m;
    zWorld = 2 * m;

    xWater = 10.0 * cm;
    yWater = 10.0 * cm;
    zWater = 75.0 * cm;

    xGlue = 2.0 * cm;
    yGlue = 0.5 * mm;
    zGlue = 2.0 * cm;
    glueCenter = G4ThreeVector(0.0, yWater + yGlue, 0.0);

    xPlate = 75.0 * cm;
    yPlate = 10.0 * cm;
    zPlate = 0.5 * mm;
    detectorDistance = 20.0 * cm;
    detectorPadding = 5.0 * cm;

    MyDetectorMessenger = new DetectorMessenger(this);
    DefineMaterials();
}

MyDetectorConstruction::~MyDetectorConstruction()
{
    delete MyDetectorMessenger;
}

void MyDetectorConstruction::set_yGlue(G4double value)
{
    yGlue = value;
    glueCenter.setY(yWater + yGlue);
    if (G4RunManager::GetRunManager() != nullptr)
    {
        G4RunManager::GetRunManager()->GeometryHasBeenModified();
    }
}

void MyDetectorConstruction::SetDetectorDistance(G4double value)
{
    detectorDistance = value;
    if (G4RunManager::GetRunManager() != nullptr)
    {
        G4RunManager::GetRunManager()->GeometryHasBeenModified();
    }
}

void MyDetectorConstruction::SetDetectorPadding(G4double value)
{
    detectorPadding = value;
    if (G4RunManager::GetRunManager() != nullptr)
    {
        G4RunManager::GetRunManager()->GeometryHasBeenModified();
    }
}

void MyDetectorConstruction::DefineMaterials()
{
    G4NistManager* nist = G4NistManager::Instance();

    worldMat = nist->FindOrBuildMaterial("G4_AIR");
    waterMat = nist->FindOrBuildMaterial("G4_WATER");
    photonDetectorMat = nist->FindOrBuildMaterial("G4_Si");

    glueMat = new G4Material("CyanoacrylateGlue", 1.07 * g / cm3, 4);
    glueMat->AddElement(nist->FindOrBuildElement("C"), 6);
    glueMat->AddElement(nist->FindOrBuildElement("H"), 7);
    glueMat->AddElement(nist->FindOrBuildElement("N"), 1);
    glueMat->AddElement(nist->FindOrBuildElement("O"), 2);
}

G4VPhysicalVolume* MyDetectorConstruction::Construct()
{
    solidWorld = new G4Box("solidWorld", xWorld, yWorld, zWorld);
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld");
    physWorld = new G4PVPlacement(
        nullptr, G4ThreeVector(), logicWorld, "physWorld", nullptr, false, 0, true);

    solidWater = new G4Box("solidWater", xWater, yWater, zWater);
    logicWater = new G4LogicalVolume(solidWater, waterMat, "logicWater");
    physWater = new G4PVPlacement(
        nullptr, G4ThreeVector(), logicWater, "physWater", logicWorld, false, 0, true);

    solidGlue = new G4Box("solidGlue", xGlue, yGlue, zGlue);
    logicGlue = new G4LogicalVolume(solidGlue, glueMat, "logicGlue");
    physGlue = new G4PVPlacement(
        nullptr, glueCenter, logicGlue, "physGlue", logicWorld, false, 0, true);

    const std::array<G4String, kPhotonDetectorCount> names = {
        "physPhotonDetectorPosX",
        "physPhotonDetectorNegX",
        "physPhotonDetectorPosY",
        "physPhotonDetectorNegY",
        "physPhotonDetectorPosZ",
        "physPhotonDetectorNegZ"};

    const G4double plateHalfThickness = zPlate;
    const G4double paddedX = xWater + detectorPadding;
    const G4double paddedY = yWater + detectorPadding;
    const G4double paddedZ = zWater + detectorPadding;
    const std::array<G4ThreeVector, kPhotonDetectorCount> positions = {
        G4ThreeVector(xWater + detectorDistance + plateHalfThickness, 0, 0),
        G4ThreeVector(-(xWater + detectorDistance + plateHalfThickness), 0, 0),
        G4ThreeVector(0, yWater + detectorDistance + plateHalfThickness, 0),
        G4ThreeVector(0, -(yWater + detectorDistance + plateHalfThickness), 0),
        G4ThreeVector(0, 0, zWater + detectorDistance + plateHalfThickness),
        G4ThreeVector(0, 0, -(zWater + detectorDistance + plateHalfThickness))};

    const std::array<G4ThreeVector, kPhotonDetectorCount> halfSizes = {
        G4ThreeVector(paddedZ, paddedY, plateHalfThickness),
        G4ThreeVector(paddedZ, paddedY, plateHalfThickness),
        G4ThreeVector(paddedX, paddedZ, plateHalfThickness),
        G4ThreeVector(paddedX, paddedZ, plateHalfThickness),
        G4ThreeVector(paddedX, paddedY, plateHalfThickness),
        G4ThreeVector(paddedX, paddedY, plateHalfThickness)};

    auto rotY90 = new G4RotationMatrix();
    rotY90->rotateY(90 * degree);
    auto rotX90 = new G4RotationMatrix();
    rotX90->rotateX(90 * degree);

    const std::array<G4RotationMatrix*, kPhotonDetectorCount> rotations = {
        rotY90, rotY90, rotX90, rotX90, nullptr, nullptr};

    for (G4int i = 0; i < kPhotonDetectorCount; ++i)
    {
        solidPhotonDetectors[i] = new G4Box(
            "solidPhotonDetector", halfSizes[i].x(), halfSizes[i].y(), halfSizes[i].z());
        logicPhotonDetectors[i] = new G4LogicalVolume(
            solidPhotonDetectors[i], photonDetectorMat, "logicPhotonDetector");
        physPhotonDetectors[i] = new G4PVPlacement(
            rotations[i],
            positions[i],
            logicPhotonDetectors[i],
            names[i],
            logicWorld,
            false,
            i,
            true);
    }

    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto waterVis = new G4VisAttributes(G4Colour(0.15, 0.45, 0.95, 0.35));
    waterVis->SetForceSolid(true);
    logicWater->SetVisAttributes(waterVis);

    auto glueVis = new G4VisAttributes(G4Colour(0.95, 0.75, 0.10, 0.65));
    glueVis->SetForceSolid(true);
    logicGlue->SetVisAttributes(glueVis);

    auto plateVis = new G4VisAttributes(G4Colour(0.10, 0.85, 0.40, 0.55));
    plateVis->SetForceSolid(true);
    for (auto* logicPlate : logicPhotonDetectors)
    {
        logicPlate->SetVisAttributes(plateVis);
    }

    return physWorld;
}
