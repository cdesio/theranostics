#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#ifdef ALPHAGLUE_USE_VIS
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#endif
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "PhysicsList.hh"
#include "RunAction.hh"
#include "ActionInitialization.hh"
#include "globals.hh"
#include <cstdlib>
#include <filesystem>
#include <string>
int main(int argc, char** argv)
{
  // Parse -o <filename>, optional -gui, --debug, --track-glue <copyNo>, and a macro file (first non-option)
  G4String outputFileName = "PSfile.bin";
  G4String macroFile;
  G4bool forceGui = false;
  G4bool debug = false;
  G4int trackGlueCopyNo = -1;
  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if (arg == "-o" && i + 1 < argc) {
      outputFileName = argv[++i];
    } else if (arg == "-gui") {
      forceGui = true;
    } else if (arg == "--debug") {
      debug = true;
    } else if (arg == "--track-glue" && i + 1 < argc) {
      trackGlueCopyNo = std::stoi(argv[++i]);
    } else if (!arg.empty() && arg[0] != '-') { // treat first non-option as macro file
      macroFile = argv[i];
    }
  }

  std::string rn222ExitCsv = outputFileName;
  const std::size_t slash = rn222ExitCsv.find_last_of("/\\");
  const std::string dir = slash == std::string::npos ? "" : rn222ExitCsv.substr(0, slash + 1);
  std::string stem = slash == std::string::npos ? rn222ExitCsv : rn222ExitCsv.substr(slash + 1);
  const std::size_t dot = stem.rfind('.');
  if (dot != std::string::npos) {
    stem.erase(dot);
  }
  rn222ExitCsv = dir + stem + "_Rn222_exit_glue_events.csv";
  setenv("ALPHAGLUE_RN222_EXIT_CSV", rn222ExitCsv.c_str(), 1);

#ifdef ALPHAGLUE_USE_VIS
  G4UIExecutive* ui = nullptr;
  if (macroFile.empty() || forceGui) { ui = new G4UIExecutive(argc, argv); }
#else
  if (forceGui) {
    G4cerr << "This executable was built without Geant4 visualization support." << G4endl;
    return 1;
  }
#endif

  // Use serial run manager for GUI so visualization markers are drawn on the master thread.
  std::unique_ptr<G4RunManager> runManager(
      G4RunManagerFactory::CreateRunManager(forceGui ? G4RunManagerType::Serial
                                                     : G4RunManagerType::Default));
  runManager->SetNumberOfThreads(1); // by default

  runManager->SetUserInitialization(new MyDetectorConstruction());
  runManager->SetUserInitialization(new MyPhysicsList());
  runManager->SetUserInitialization(new MyActionInitialization(outputFileName, debug, trackGlueCopyNo));

#ifdef ALPHAGLUE_USE_VIS
  G4VisManager* visManager = nullptr;
  if (ui) {
    visManager = new G4VisExecutive;
    visManager->Initialize();
  }
#endif

  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  const std::filesystem::path executableDirectory =
      std::filesystem::absolute(argv[0]).parent_path();
  const auto executeMacro = [UImanager](const std::filesystem::path& path) {
    UImanager->ApplyCommand("/control/execute " + path.string());
  };

  // Process macro or start UI session
#ifdef ALPHAGLUE_USE_VIS
  if ( ! ui ) {
    // batch mode
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command+macroFile);
  }
  else {
    // interactive mode
    if (!macroFile.empty()) {
      G4String command = "/control/execute ";
      UImanager->ApplyCommand(command + macroFile);
      executeMacro(executableDirectory / "vis.mac");
    } else {
      executeMacro(executableDirectory / "setup.mac");
      executeMacro(executableDirectory / "vis.mac");
      UImanager->ApplyCommand("/run/printProgress 10");
      UImanager->ApplyCommand("/run/beamOn 100");
    }
    ui->SessionStart();
    delete ui;
  }
#else
  if (macroFile.empty()) {
    executeMacro(executableDirectory / "pos.mac");
  } else {
    UImanager->ApplyCommand("/control/execute " + macroFile);
  }
#endif

#ifdef ALPHAGLUE_USE_VIS
  delete visManager;
#endif

    return 0;
}
