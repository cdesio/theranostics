#include "SteppingAction.hh"

#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"

#include <sstream>

namespace
{
bool IsPhotonDetectorVolume(const G4String& name)
{
    return name.rfind("physPhotonDetector", 0) == 0;
}

bool ContainsExcitedStateMarker(const G4String& name)
{
    return name.find('[') != std::string::npos;
}

G4int LookupOrigin(const std::map<G4String, G4int>& originMap, const G4String& key)
{
    const auto it = originMap.find(key);
    return it != originMap.end() ? it->second : -1;
}
}

MySteppingAction::MySteppingAction(MyEventAction* eventAction,
                                   const G4String& outputFileName,
                                   G4bool debug,
                                   G4int trackGlueCopyNo)
    : fEventAction(eventAction), fDebug(debug), fTrackGlueCopyNo(trackGlueCopyNo)
{
    fDetector = static_cast<const MyDetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    G4String stem = outputFileName.empty() ? "PSfile" : outputFileName;
    const std::size_t dot = stem.rfind('.');
    if (dot != std::string::npos)
    {
        stem.erase(dot);
    }

    fPhotonBoundaryLog.open(stem + "_photon_boundaries.csv");

    if (fPhotonBoundaryLog.is_open())
    {
        fPhotonBoundaryLog
            << "eventID,trackID,parentID,particleName,originCode,originLabel,originIsotope,creatorProcess,boundary,volumeName,"
            << "x_mm,y_mm,z_mm,px,py,pz,kineticEnergy_MeV,time_s,"
            << "gammaEmittedX_mm,gammaEmittedY_mm,gammaEmittedZ_mm,sourceVolumeName\n";
    }
}

MySteppingAction::~MySteppingAction()
{
    if (fPhotonBoundaryLog.is_open())
    {
        fPhotonBoundaryLog.close();
    }
}

void MySteppingAction::EnsureOriginAssigned(const G4Track* track)
{
    const G4int trackID = track->GetTrackID();
    if (fEventAction->parentParticle.find(trackID) != fEventAction->parentParticle.end())
    {
        return;
    }

    const G4String particleName = track->GetParticleDefinition()->GetParticleName();
    const auto* creator = track->GetCreatorProcess();

    if (creator == nullptr)
    {
        fEventAction->parentParticle[trackID] = LookupOrigin(particleOriginMap, particleName);
        return;
    }

    if (creator->GetProcessName() == "RadioactiveDecay")
    {
        G4String lookupKey = particleName;
        if (ContainsExcitedStateMarker(particleName))
        {
            lookupKey = particleName.substr(0, 5);
        }
        else if (particleName == "e-" || particleName == "gamma" || particleName == "alpha" ||
                 particleName == "e+")
        {
            const auto parentIt = fEventAction->parentParticle.find(track->GetParentID());
            G4String parentName = "unknown";
            if (parentIt != fEventAction->parentParticle.end())
            {
                const auto reverseIt = reverseParticleOriginMap.find(parentIt->second);
                if (reverseIt != reverseParticleOriginMap.end())
                {
                    parentName = reverseIt->second;
                }
            }
            lookupKey = particleName + parentName;
        }
        fEventAction->parentParticle[trackID] = LookupOrigin(particleOriginMap, lookupKey);
        return;
    }

    const auto parentIt = fEventAction->parentParticle.find(track->GetParentID());
    fEventAction->parentParticle[trackID] =
        parentIt != fEventAction->parentParticle.end() ? parentIt->second : -1;
}

G4String MySteppingAction::BuildPhotonBoundaryKey(G4int eventID,
                                                  G4int trackID,
                                                  G4int stepNumber,
                                                  const G4String& boundaryTag,
                                                  const G4String& volumeName) const
{
    std::ostringstream key;
    key << eventID << ':' << trackID << ':' << stepNumber << ':' << boundaryTag << ':'
        << volumeName;
    return key.str();
}

G4bool MySteppingAction::IsTrackedPhotonVolume(const G4String& volumeName) const
{
    return volumeName == "physWater" || volumeName == "physGlue" ||
           IsPhotonDetectorVolume(volumeName);
}

G4String MySteppingAction::ResolveOriginLabel(G4int originCode) const
{
    const auto it = originCodeToLabel.find(originCode);
    return it != originCodeToLabel.end() ? it->second : "unknown";
}

G4String MySteppingAction::ResolveOriginIsotope(G4int originCode) const
{
    const auto exact = reverseParticleOriginMap.find(originCode);
    if (exact != reverseParticleOriginMap.end())
    {
        return exact->second;
    }

    G4String label = ResolveOriginLabel(originCode);
    if (label == "unknown" || label == "gamma" || label == "e+" || label == "e-" ||
        label == "alpha")
    {
        return label;
    }

    if (label.rfind("gamma", 0) == 0)
    {
        return label.substr(5);
    }
    if (label.rfind("alpha", 0) == 0)
    {
        return label.substr(5);
    }
    if (label.rfind("e-", 0) == 0)
    {
        return label.substr(2);
    }
    if (label.rfind("e+", 0) == 0)
    {
        return label.substr(2);
    }

    return label;
}

void MySteppingAction::LogPhotonBoundary(const G4Step* step,
                                         const G4String& boundaryTag,
                                         const G4String& volumeName,
                                         const G4StepPoint* point) const
{
    if (!fPhotonBoundaryLog.is_open())
    {
        return;
    }

    const G4Event* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
    const G4int eventID = event ? event->GetEventID() : -1;
    const G4Track* track = step->GetTrack();
    const G4String creatorName =
        track->GetCreatorProcess() ? track->GetCreatorProcess()->GetProcessName() : "primary";
    const G4int originCode =
        fEventAction->parentParticle.count(track->GetTrackID())
            ? fEventAction->parentParticle[track->GetTrackID()]
            : -1;
    const G4String originLabel = ResolveOriginLabel(originCode);
    const G4String originIsotope = ResolveOriginIsotope(originCode);

    const G4ThreeVector position = point->GetPosition();
    const G4ThreeVector momentum = point->GetMomentumDirection();
    const G4ThreeVector gammaEmittedPosition = track->GetVertexPosition();
    const G4String sourceVolumeName =
        track->GetLogicalVolumeAtVertex() ? track->GetLogicalVolumeAtVertex()->GetName()
                                          : "unknown";

    fPhotonBoundaryLog << eventID << ','
                       << track->GetTrackID() << ','
                       << track->GetParentID() << ','
                       << track->GetParticleDefinition()->GetParticleName() << ','
                       << originCode << ','
                       << originLabel << ','
                       << originIsotope << ','
                       << creatorName << ','
                       << boundaryTag << ','
                       << volumeName << ','
                       << position.x() / mm << ','
                       << position.y() / mm << ','
                       << position.z() / mm << ','
                       << momentum.x() << ','
                       << momentum.y() << ','
                       << momentum.z() << ','
                       << point->GetKineticEnergy() / MeV << ','
                       << point->GetGlobalTime() / s << ','
                       << gammaEmittedPosition.x() / mm << ','
                       << gammaEmittedPosition.y() / mm << ','
                       << gammaEmittedPosition.z() / mm << ','
                       << sourceVolumeName
                       << '\n';
}

void MySteppingAction::UserSteppingAction(const G4Step* step)
{
    G4Track* track = step->GetTrack();
    const G4String particleName = track->GetParticleDefinition()->GetParticleName();
    if (particleName == "nu_e" || particleName == "anti_nu_e")
    {
        return;
    }

    EnsureOriginAssigned(track);

    const G4StepPoint* prePoint = step->GetPreStepPoint();
    const G4StepPoint* postPoint = step->GetPostStepPoint();
    const G4String preVolume =
        prePoint->GetPhysicalVolume() ? prePoint->GetPhysicalVolume()->GetName() : "";
    const G4String postVolume =
        postPoint->GetPhysicalVolume() ? postPoint->GetPhysicalVolume()->GetName() : "";

    const G4Event* event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
    const G4int eventID = event ? event->GetEventID() : -1;

    if (particleName == "gamma" && postPoint->GetStepStatus() == fGeomBoundary &&
        IsTrackedPhotonVolume(postVolume))
    {
        const auto key = BuildPhotonBoundaryKey(
            eventID, track->GetTrackID(), track->GetCurrentStepNumber(), "enter", postVolume);
        if (fEventAction->loggedPhotonBoundaries.insert(key).second)
        {
            LogPhotonBoundary(step, "enter", postVolume, postPoint);
        }
    }

    if (particleName == "gamma" && prePoint->GetStepStatus() == fGeomBoundary &&
        IsTrackedPhotonVolume(preVolume))
    {
        const auto key = BuildPhotonBoundaryKey(
            eventID, track->GetTrackID(), track->GetCurrentStepNumber(), "exit", preVolume);
        if (fEventAction->loggedPhotonBoundaries.insert(key).second)
        {
            LogPhotonBoundary(step, "exit", preVolume, prePoint);
        }
    }

    if (fDebug && fTrackGlueCopyNo >= 0 && preVolume == "physGlue" && step->IsFirstStepInVolume())
    {
        G4cout << "[SteppingAction] track entered glue with copy selector "
               << fTrackGlueCopyNo << " trackID=" << track->GetTrackID()
               << G4endl;
    }
}
