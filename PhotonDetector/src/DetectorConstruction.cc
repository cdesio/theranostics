#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"
#include "XVIReadoutSD.hh"

#include "G4Colour.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4RunManager.hh"
#include "G4SDManager.hh"
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

    xviPanelHalfSize = 204.8 * mm;
    // The Bristol documentation does not specify the CsI thickness. Preserve the
    // previous 1 mm detector thickness until a measured value is available.
    xviScintillatorHalfThickness = 0.5 * mm;
    detectorDistance = 20.0 * cm;

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

void MyDetectorConstruction::SetXVIScintillatorThickness(G4double value)
{
    xviScintillatorHalfThickness = 0.5 * value;
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
    xviScintillatorMat = nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");

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

    const G4double plateHalfThickness = xviScintillatorHalfThickness;
    const std::array<G4ThreeVector, kPhotonDetectorCount> positions = {
        G4ThreeVector(xWater + detectorDistance + plateHalfThickness, 0, 0),
        G4ThreeVector(-(xWater + detectorDistance + plateHalfThickness), 0, 0),
        G4ThreeVector(0, yWater + detectorDistance + plateHalfThickness, 0),
        G4ThreeVector(0, -(yWater + detectorDistance + plateHalfThickness), 0),
        G4ThreeVector(0, 0, zWater + detectorDistance + plateHalfThickness),
        G4ThreeVector(0, 0, -(zWater + detectorDistance + plateHalfThickness))};

    const G4ThreeVector panelHalfSize(
        xviPanelHalfSize, xviPanelHalfSize, plateHalfThickness);

    auto rotYPos90 = new G4RotationMatrix();
    rotYPos90->rotateY(90 * degree);
    auto rotYNeg90 = new G4RotationMatrix();
    rotYNeg90->rotateY(-90 * degree);
    auto rotXPos90 = new G4RotationMatrix();
    rotXPos90->rotateX(90 * degree);
    auto rotXNeg90 = new G4RotationMatrix();
    rotXNeg90->rotateX(-90 * degree);
    auto rotY180 = new G4RotationMatrix();
    rotY180->rotateY(180 * degree);

    const std::array<G4RotationMatrix*, kPhotonDetectorCount> rotations = {
        rotYPos90, rotYNeg90, rotXNeg90, rotXPos90, nullptr, rotY180};

    for (G4int i = 0; i < kPhotonDetectorCount; ++i)
    {
        solidPhotonDetectors[i] = new G4Box(
            "solidXVIPanel", panelHalfSize.x(), panelHalfSize.y(), panelHalfSize.z());
        logicPhotonDetectors[i] = new G4LogicalVolume(
            solidPhotonDetectors[i], xviScintillatorMat, "logicXVIPanel");
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

void MyDetectorConstruction::ConstructSDandField()
{
    auto* sdManager = G4SDManager::GetSDMpointer();
    auto* readout = new XVIReadoutSD("/XVI/readout");
    sdManager->AddNewDetector(readout);

    for (auto* panel : logicPhotonDetectors)
    {
        SetSensitiveDetector(panel, readout);
    }
}
