#include "cmLocalAtmelStudio7Generator.h"

#include <cmext/algorithm>

#include <cm3p/expat.h>

#include "cmAlgorithms.h"
#include "cmAtmelStudio7TargetGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalAtmelStudio7Generator.h"
#include "cmMakefile.h"
#include "cmXMLParser.h"
#include "cmake.h"

cmLocalAtmelStudio7Generator::cmLocalAtmelStudio7Generator(
  cmGlobalGenerator* gg, cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
}

cmLocalAtmelStudio7Generator::~cmLocalAtmelStudio7Generator()
{
}

void cmLocalAtmelStudio7Generator::WriteStampFiles()
{
  // Touch a timestamp file used to determine when the project file is
  // out of date.
  std::string stampName = cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles");
  cmSystemTools::MakeDirectory(stampName);

  stampName += "/generate.stamp";
  cmsys::ofstream stamp(stampName.c_str());
  stamp << "# CMake generation timestamp file for this directory.\n";

  // Create a helper file so CMake can determine when it is run
  // through the rule created by CreateVCProjBuildRule whether it
  // really needs to regenerate the project.  This file lists its own
  // dependencies.  If any file listed in it is newer than itself then
  // CMake must rerun.  Otherwise the project files are up to date and
  // the stamp file can just be touched.
  std::string depName = cmStrCat(stampName, ".depend");
  cmsys::ofstream depFile(depName.c_str());
  depFile << "# CMake generation dependency list for this directory.\n";

  std::vector<std::string> listFiles(this->Makefile->GetListFiles());
  cmake* cm = this->GlobalGenerator->GetCMakeInstance();
  if (cm->DoWriteGlobVerifyTarget()) {
    listFiles.push_back(cm->GetGlobVerifyStamp());
  }

  // Sort the list of input files and remove duplicates.
  std::sort(listFiles.begin(), listFiles.end(), std::less<std::string>());
  std::vector<std::string>::iterator new_end =
    std::unique(listFiles.begin(), listFiles.end());
  listFiles.erase(new_end, listFiles.end());

  for (const std::string& lf : listFiles) {
    depFile << lf << "\n";
  }
}

std::set<cmSourceFile const*>& cmLocalAtmelStudio7Generator::GetSourcesVisited(cmGeneratorTarget const* target)
{
  return this->SourcesVisited[const_cast<cmGeneratorTarget*>(target)];
}

void cmLocalAtmelStudio7Generator::Generate()
{
  auto target_list = this->GlobalGenerator->GetLocalGeneratorTargetsInOrder(this);
  // Create the project file for each target.
  for (cmGeneratorTarget* gt : target_list) {
    if (!gt->IsInBuildSystem() || gt->GetProperty("EXTERNAL_MSPROJECT")) {
      continue;
    }

    auto& gtVisited = this->GetSourcesVisited(gt);
    auto& deps = this->GlobalGenerator->GetTargetDirectDepends(gt);
    for (auto& d : deps) {
      // Take the union of visited source files of custom commands
      auto depVisited = this->GetSourcesVisited(d);
      gtVisited.insert(depVisited.begin(), depVisited.end());
    }

    // Generate each target
    this->GenerateTarget(gt);
  }

  this->WriteStampFiles();
}

void cmLocalAtmelStudio7Generator::GenerateTarget(cmGeneratorTarget* target)
{
  cmAtmelStudio7TargetGenerator targetGenerator( target,
                                                 static_cast<cmGlobalAtmelStudio7Generator*>(this->GetGlobalGenerator()));
  targetGenerator.Generate();
}

const char* cmLocalAtmelStudio7Generator::ReportErrorLabel() const
{
  return ":VCEnd";
}
