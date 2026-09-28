#include "XVIReadoutSD.hh"

#include "G4AffineTransform.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4TouchableHandle.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4ios.hh"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <string>

const std::array<const char*, XVIReadoutSD::kPanelCount> XVIReadoutSD::kPanelNames = {
    "pos_x", "neg_x", "pos_y", "neg_y", "pos_z", "neg_z"};

XVIReadoutSD::XVIReadoutSD(const G4String& name) : G4VSensitiveDetector(name)
{
    const std::size_t pixelCount =
        static_cast<std::size_t>(kPixelsPerAxis) * kPixelsPerAxis;
    for (auto& panel : fEnergyDeposit)
    {
        panel.assign(pixelCount, 0.0);
    }
}

std::size_t XVIReadoutSD::PixelIndex(G4int x, G4int y)
{
    return static_cast<std::size_t>(y) * kPixelsPerAxis + x;
}

void XVIReadoutSD::Reset()
{
    for (auto& panel : fEnergyDeposit)
    {
        std::fill(panel.begin(), panel.end(), 0.0);
    }
    fTotalEnergyDeposit.fill(0.0);
    fHitSteps.fill(0);
}

G4bool XVIReadoutSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    const G4double weightedEnergyDeposit =
        step->GetTotalEnergyDeposit() * step->GetPreStepPoint()->GetWeight();
    if (weightedEnergyDeposit <= 0.0)
    {
        return false;
    }

    const auto touchable = step->GetPreStepPoint()->GetTouchableHandle();
    const G4int panel = touchable->GetCopyNumber();
    if (panel < 0 || panel >= kPanelCount)
    {
        return false;
    }

    const G4ThreeVector worldPosition =
        0.5 * (step->GetPreStepPoint()->GetPosition() +
               step->GetPostStepPoint()->GetPosition());
    const G4ThreeVector localPosition =
        touchable->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

    const G4double panelHalfSize = 0.5 * kPixelsPerAxis * kPixelPitchMm * mm;
    const G4int pixelX = static_cast<G4int>(
        std::floor((localPosition.x() + panelHalfSize) / (kPixelPitchMm * mm)));
    const G4int pixelY = static_cast<G4int>(
        std::floor((localPosition.y() + panelHalfSize) / (kPixelPitchMm * mm)));

    if (pixelX < 0 || pixelX >= kPixelsPerAxis ||
        pixelY < 0 || pixelY >= kPixelsPerAxis)
    {
        return false;
    }

    fEnergyDeposit[panel][PixelIndex(pixelX, pixelY)] += weightedEnergyDeposit;
    fTotalEnergyDeposit[panel] += weightedEnergyDeposit;
    ++fHitSteps[panel];
    return true;
}

void XVIReadoutSD::WritePanelImages(const G4String& outputFileName) const
{
    std::string stem = outputFileName.empty() ? "PSfile" : outputFileName;
    const std::size_t dot = stem.find_last_of('.');
    const std::size_t slash = stem.find_last_of("/\\");
    if (dot != std::string::npos &&
        (slash == std::string::npos || dot > slash))
    {
        stem.erase(dot);
    }

    for (G4int panel = 0; panel < kPanelCount; ++panel)
    {
        const std::string panelStem =
            stem + "_xvi_" + kPanelNames[panel] + "_edep";
        std::ofstream image(panelStem + ".raw", std::ios::binary);
        if (!image)
        {
            G4cerr << "Unable to write XVI panel image " << panelStem << ".raw" << G4endl;
            continue;
        }

        for (const G4double value : fEnergyDeposit[panel])
        {
            const float valueMeV = static_cast<float>(value / MeV);
            image.write(reinterpret_cast<const char*>(&valueMeV), sizeof(valueMeV));
        }
        image.close();

        std::ofstream metadata(panelStem + ".json");
        metadata << std::setprecision(12)
                 << "{\n"
                 << "  \"panel_id\": " << panel << ",\n"
                 << "  \"panel_name\": \"" << kPanelNames[panel] << "\",\n"
                 << "  \"rows\": " << kPixelsPerAxis << ",\n"
                 << "  \"columns\": " << kPixelsPerAxis << ",\n"
                 << "  \"pixel_pitch_mm\": " << kPixelPitchMm << ",\n"
                 << "  \"panel_size_mm\": " << kPixelsPerAxis * kPixelPitchMm << ",\n"
                 << "  \"dtype\": \"float32-native-endian\",\n"
                 << "  \"layout\": \"row-major-y-x\",\n"
                 << "  \"quantity\": \"weighted_energy_deposit\",\n"
                 << "  \"unit\": \"MeV\",\n"
                 << "  \"hit_steps\": " << fHitSteps[panel] << ",\n"
                 << "  \"total_energy_deposit_MeV\": "
                 << fTotalEnergyDeposit[panel] / MeV << "\n"
                 << "}\n";
    }
}
