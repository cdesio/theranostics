#ifndef XVIREADOUTSD_HH
#define XVIREADOUTSD_HH

#include "G4VSensitiveDetector.hh"
#include "globals.hh"

#include <array>
#include <cstddef>
#include <vector>

class G4Step;
class G4TouchableHistory;

class XVIReadoutSD : public G4VSensitiveDetector
{
  public:
    static constexpr G4int kPanelCount = 6;
    static constexpr G4int kPixelsPerAxis = 1024;
    static constexpr G4double kPixelPitchMm = 0.4;

    explicit XVIReadoutSD(const G4String& name);
    ~XVIReadoutSD() override = default;

    G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
    void Reset();
    void WritePanelImages(const G4String& outputFileName) const;

  private:
    static const std::array<const char*, kPanelCount> kPanelNames;
    static std::size_t PixelIndex(G4int x, G4int y);

    std::array<std::vector<G4double>, kPanelCount> fEnergyDeposit;
    std::array<G4double, kPanelCount> fTotalEnergyDeposit{};
    std::array<std::size_t, kPanelCount> fHitSteps{};
};

#endif
