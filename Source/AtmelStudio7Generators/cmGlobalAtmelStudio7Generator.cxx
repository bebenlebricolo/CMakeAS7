/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalAtmelStudio7Generator.h"

#include <utility>

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmEncoding.h"
#include "cmStringUtils.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmLocalAtmelStudio7Generator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmUuid.h"
#include "cmake.h"

class cmGlobalAtmelStudio7Generator::Factory : public cmGlobalGeneratorFactory
{
public:
  std::unique_ptr<cmGlobalGenerator> CreateGlobalGenerator(
    const std::string& name, cmake* cm) const override
  {
    if (name == cmGlobalAtmelStudio7Generator::TruncatedGeneratorName ||
        name == cmGlobalAtmelStudio7Generator::GeneratorName) {
        return std::unique_ptr<cmGlobalAtmelStudio7Generator>(
          new cmGlobalAtmelStudio7Generator(cm, name));
    }
    return std::unique_ptr<cmGlobalGenerator>();
  }

  void GetDocumentation(cmDocumentationEntry& entry) const override
  {
    entry.Name = std::string(cmGlobalAtmelStudio7Generator::GeneratorName);
    entry.Brief = "Generates Atmel Studio 7.0 project files.";
  }

  std::vector<std::string> GetGeneratorNames() const override
  {
    std::vector<std::string> names;
    names.push_back(cmGlobalAtmelStudio7Generator::GeneratorName);
    return names;
  }

  std::vector<std::string> GetGeneratorNamesWithPlatform() const override
  {
    std::vector<std::string> names;
    names.push_back(cmGlobalAtmelStudio7Generator::GeneratorName +
                    std::string(" ARM"));
    names.push_back(cmGlobalAtmelStudio7Generator::GeneratorName +
                    std::string(" AVR8"));
    names.push_back(cmGlobalAtmelStudio7Generator::GeneratorName +
                    std::string(" AVR32"));

    return names;
  }

  bool SupportsToolset() const override { return false; }
  bool SupportsPlatform() const override { return true; }

  std::vector<std::string> GetKnownPlatforms() const override
  {
    std::vector<std::string> platforms;
    platforms.emplace_back("ARM");
    platforms.emplace_back("AVR8");
    platforms.emplace_back("AVR32");

    return platforms;
  }

  std::string GetDefaultPlatformName() const override { return "AVR8"; }
};

std::string cmGlobalAtmelStudio7Generator::GetPlatform(
  AvailablePlatforms platform)
{
  auto foundItem = std::find_if(
    PlatformMap.begin(), PlatformMap.end(),
    [platform](const std::pair<AvailablePlatforms, std::string>& target) {
      return (target.first == platform);
    });
  if (foundItem != PlatformMap.end()) {
    return foundItem->second;
  }
  return "";
}

cmGlobalAtmelStudio7Generator::AvailablePlatforms
cmGlobalAtmelStudio7Generator::GetPlatform(const std::string& name)
{
  auto foundItem = std::find_if(
    PlatformMap.begin(), PlatformMap.end(),
    [name](const std::pair<AvailablePlatforms, std::string>& target) {
      return (target.second == name);
    });
  if (foundItem != PlatformMap.end()) {
    return (foundItem->first);
  }
  return AvailablePlatforms::Unsupported;
}

cmGlobalAtmelStudio7Generator::AvailablePlatforms
cmGlobalAtmelStudio7Generator::GetCurrentPlatform() const
{
  return CurrentPlatform;
}

bool cmGlobalAtmelStudio7Generator::SetGeneratorPlatform(std::string const& p, cmMakefile* mf)
{
  if (p.empty()) {
    // Defaults to AVR8
    CurrentPlatform = AvailablePlatforms::AVR8;
    return true;
  }

  AvailablePlatforms platform = GetPlatform(p);
  if (AvailablePlatforms::Unsupported == platform)
  {
      std::ostringstream e;
      /* clang-format off */
      e <<
        "Generator\n"
        "  " << this->GetName() << "\n"
        "does not support platform specification, but platform\n"
        "  " << p << "\n"
        "was specified.";
      /* clang-format on */
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return false;
  }
  else
  {
    CurrentPlatform = platform;
  }

  return true;
}

std::string cmGlobalAtmelStudio7Generator::GetName() const
{
  return GeneratorName;
}

std::vector<cmGlobalAtmelStudio7Generator::LangProp> cmGlobalAtmelStudio7Generator::SupportedLanguagesList = {
  { cmGlobalAtmelStudio7Generator::SupportedLanguages::C, { "C" }, { "H" } },
  { cmGlobalAtmelStudio7Generator::SupportedLanguages::CPP, { "CPP", "CXX" }, { "HPP", "HXX", "H" }},
  { cmGlobalAtmelStudio7Generator::SupportedLanguages::ASM, { "ASM" }, {} }
};

bool cmGlobalAtmelStudio7Generator::SupportsLanguage(const cmGlobalAtmelStudio7Generator::SupportedLanguages lang)
{
  auto found_item = std::find_if(SupportedLanguagesList.begin(), SupportedLanguagesList.end(), [lang](const LangProp& prop) {
    return prop.lang == lang;
  });

  return found_item != SupportedLanguagesList.end();
}

static bool langmatch(const std::string& lang, const std::vector<std::string>& reference)
{
  std::string lower_lang = cmutils::strings::to_lowercase(lang);
  auto found_item = std::find_if(reference.begin(), reference.end(), [lower_lang](const std::string& str) {
    return cmutils::strings::to_lowercase(str) == lower_lang;
  });
  return found_item != reference.end();
}

bool cmGlobalAtmelStudio7Generator::SupportsLanguage(const std::string& lang)
{
  auto found_item = std::find_if(SupportedLanguagesList.begin(), SupportedLanguagesList.end(), [lang](const LangProp& prop) {
      return langmatch(lang, prop.lang_src_str);
  });

  return found_item != SupportedLanguagesList.end();
}


bool cmGlobalAtmelStudio7Generator::CheckLanguages(std::vector<std::string> const& languages, cmMakefile* mf) const
{
  (void)mf;
  bool supports_all_languages = true;
  for (const std::string& lang : languages) {
    supports_all_languages &= SupportsLanguage(lang);
  }

  return supports_all_languages;
}

// Static map definition
std::map<cmGlobalAtmelStudio7Generator::AvailablePlatforms, std::string>
  cmGlobalAtmelStudio7Generator::PlatformMap = {
    { cmGlobalAtmelStudio7Generator::AvailablePlatforms::ARM, "ARM" },
    { cmGlobalAtmelStudio7Generator::AvailablePlatforms::AVR32, "AVR32" },
    { cmGlobalAtmelStudio7Generator::AvailablePlatforms::AVR8, "AVR8" },
    { cmGlobalAtmelStudio7Generator::AvailablePlatforms::Unsupported,
      "Unsupported" }
  };

std::unique_ptr<cmGlobalGeneratorFactory>
cmGlobalAtmelStudio7Generator::NewFactory()
{
  return std::unique_ptr<cmGlobalGeneratorFactory>(new Factory);
}

cmGlobalAtmelStudio7Generator::cmGlobalAtmelStudio7Generator(
  cmake* cm, const std::string& platformInGeneratorName)
  : cmGlobalGenerator(cm)
{
  // this->ExpressEdition = cmSystemTools::ReadRegistryValue(
  //  "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VCExpress\\11.0\\Setup\\VC;"
  //  "ProductDir",
  //  vc11Express, cmSystemTools::KeyWOW64_32);
  // this->DefaultPlatformToolset = "v110";
  // this->DefaultCLFlagTableName = "v11";
  // this->DefaultCSharpFlagTableName = "v11";
  // this->DefaultLibFlagTableName = "v11";
  // this->DefaultLinkFlagTableName = "v11";
  // this->DefaultMasmFlagTableName = "v11";
  // this->DefaultRCFlagTableName = "v11";
  // this->Version = VS11;
}

void cmGlobalAtmelStudio7Generator::WriteATSLNHeader(std::ostream& fout)
{
  // Write magic numbers first
  const cmutils::EncodingProperties& encoding_prop =
    cmutils::EncodingHandler::get_encoding_properties(cmutils::Encoding::UTF8);
  std::string utf8bom = encoding_prop.signature.magic;
  fout << utf8bom << std::endl;

  // Uses Microsoft Visual Studio VS11 format version
  fout << "Microsoft Visual Studio Solution File, Format Version 12.00\n"
       << "# Atmel Studio Solution File, Format Version 11.00\n";
}

std::string cmGlobalAtmelStudio7Generator::GetATSLNFile(
  cmLocalGenerator* const root) const
{
  return cmStrCat(root->GetCurrentBinaryDirectory(), '/',
                  root->GetProjectName(), SolutionFileExtension);
}

void cmGlobalAtmelStudio7Generator::OutputATSLNFile()
{
  for (auto& it : this->ProjectMap) {
    this->OutputATSLNFile(it.second[0], it.second);
  }
}

std::unique_ptr<cmLocalGenerator> cmGlobalAtmelStudio7Generator::CreateLocalGenerator(
  cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalAtmelStudio7Generator>(this, mf));
}

void cmGlobalAtmelStudio7Generator::OutputATSLNFile(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  if (generators.empty()) {
    return;
  }
  this->CurrentProject = root->GetProjectName();

  // Recompose targeted solution file path
  std::string fname = GetATSLNFile(root);
  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }
  this->WriteATSLNFile(fout, root, generators);
  if (fout.Close()) {
    this->FileReplacedDuringGenerate(fname);
  }
}

std::string cmGlobalAtmelStudio7Generator::GetStartupProjectName(
  cmLocalGenerator const* root) const
{
  cmProp n = root->GetMakefile()->GetProperty("VS_STARTUP_PROJECT");
  if (cmNonempty(n)) {
    std::string startup = *n;
    if (this->FindTarget(startup)) {
      return startup;
    } else {
      root->GetMakefile()->IssueMessage(
        MessageType::AUTHOR_WARNING,
        "Directory property VS_STARTUP_PROJECT specifies target "
        "'" +
          startup + "' that does not exist.  Ignoring.");
    }
  }

  // default, if not specified
  return this->GetAllTargetName();
}

bool cmGlobalAtmelStudio7Generator::FindMakeProgram(cmMakefile*)
{
  // Visual Studio generators know how to lookup their build tool
  // directly instead of needing a helper module to do it, so we
  // do not actually need to put CMAKE_MAKE_PROGRAM into the cache.

  //if (cmIsOff(mf->GetDefinition("CMAKE_MAKE_PROGRAM"))) {
  //  mf->AddDefinition("CMAKE_MAKE_PROGRAM", this->GetVSMakeProgram());
  //}
  return true;
}

std::string cmGlobalAtmelStudio7Generator::GetGUID(std::string const& name)
{
  std::string const& guidStoreName = name + "_GUID_CMAKE";
  if (const char* storedGUID =
        this->CMakeInstance->GetCacheDefinition(guidStoreName)) {
    return std::string(storedGUID);
  }
  // Compute a GUID that is deterministic but unique to the build tree.
  std::string input =
    cmStrCat(this->CMakeInstance->GetState()->GetBinaryDirectory(), '|', name);

  cmUuid uuidGenerator;

  std::vector<unsigned char> uuidNamespace;
  uuidGenerator.StringToBinary("ee30c4be-5192-4fb0-b335-722a2dffe760",
                               uuidNamespace);

  std::string guid = uuidGenerator.FromMd5(uuidNamespace, input);

  return cmSystemTools::UpperCase(guid);
}

void cmGlobalAtmelStudio7Generator::WriteATSLNFile(
  std::ostream& fout, cmLocalGenerator* root,
  std::vector<cmLocalGenerator*>& generators)
{
  std::vector<std::string> configs =
    root->GetMakefile()->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);

  // Write out the header for a SLN file
  this->WriteATSLNHeader(fout);

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet orderedProjectTargets(
    projectTargets, this->GetStartupProjectName(root));

  // Generate the targets specification to a string.  We will put this in
  // the actual .atsln file later.  As a side effect, this method also
  // populates the set of folders.
  std::ostringstream targetsAtSlnString;
  this->WriteTargetsToSolution(targetsAtSlnString, root,
                               orderedProjectTargets);

  // Generate folder specification.
  bool useFolderProperty = this->UseFolderProperty();
  if (useFolderProperty) {
    this->WriteFolders(fout);
  }

  // Now write the actual target specification content.
  fout << targetsAtSlnString.str();

  // Write out the configurations information for the solution
  fout << "Global\n";
  // Write out the configurations for the solution
  this->WriteSolutionConfigurations(fout, configs);
  fout << "\tGlobalSection(" << this->ProjectConfigurationSectionName
       << ") = postSolution\n";
  // Write out the configurations for all the targets in the project
  this->WriteTargetConfigurations(fout, configs, orderedProjectTargets);
  fout << "\tEndGlobalSection\n";

  if (useFolderProperty) {
    // Write out project folders
    fout << "\tGlobalSection(NestedProjects) = preSolution\n";
    this->WriteFoldersContent(fout);
    fout << "\tEndGlobalSection\n";
  }

  // Write out global sections
  this->WriteATSLNGlobalSections(fout, root);

  // Write the footer for the SLN file
  this->WriteATSLNFooter(fout);
}

bool cmGlobalAtmelStudio7Generator::MatchesGeneratorName(
  const std::string& name) const
{
  return (name == std::string(this->GeneratorName));
}

bool cmGlobalAtmelStudio7Generator::IsAtmelStudioInstalled() const
{
  return !GetAtmelStudio7InstallationFolder().empty();
}

std::string cmGlobalAtmelStudio7Generator::GetAtmelStudio7InstallationFolder() const
{
  const char atmelStudio7InstallDir[] =
    R"(HKEY_CURRENT_USER\Software\Atmel\AtmelStudio\7.0_Config;InstallDir)";

  std::string path;
  bool hasKey = cmSystemTools::ReadRegistryValue(atmelStudio7InstallDir, path,
                                                 cmSystemTools::KeyWOW64_32);
  if (hasKey) {
    return path;
  }

  return "";
}

cmGlobalAtmelStudio7Generator::~cmGlobalAtmelStudio7Generator()
{
}

void cmGlobalAtmelStudio7Generator::EnableLanguage(
  std::vector<std::string> const& languages, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("CMAKE_GENERATOR_RC", "rc");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  if (!mf->GetDefinition("CMAKE_CONFIGURATION_TYPES")) {
    mf->AddCacheDefinition(
      "CMAKE_CONFIGURATION_TYPES", "Debug;Release;MinSizeRel;RelWithDebInfo",
      "Semicolon separated list of supported configuration types, "
      "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
      "anything else will be ignored.",
      cmStateEnums::STRING);
  }

  mf->AddDefinition("CMAKE_AS_PLATFORM_NAME_DEFAULT",
                    GetPlatform(DefaultPlatform));
  this->cmGlobalGenerator::EnableLanguage(languages, mf, optional);

  // if this environment variable is set, then copy it to
  // a static cache entry.  It will be used by
  // cmLocalGenerator::ConstructScript, to add an extra PATH
  // to all custom commands.   This is because the VS IDE
  // does not use the environment it is running in, and this allows
  // for running commands and using dll's that the IDE environment
  // does not know about.
  std::string extraPath;
  if (cmSystemTools::GetEnv("CMAKE_MSVCIDE_RUN_PATH", extraPath)) {
    mf->AddCacheDefinition("CMAKE_MSVCIDE_RUN_PATH", extraPath,
                           "Saved environment variable CMAKE_MSVCIDE_RUN_PATH",
                           cmStateEnums::STATIC);
  }
}

void cmGlobalAtmelStudio7Generator::Generate()
{
  // Handles generic stuff about generation process
  cmGlobalGenerator::Generate();

  // Write the global solution file for this build tree
  this->OutputATSLNFile();
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalAtmelStudio7Generator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& config, bool fast, int jobs, bool verbose,
  std::vector<std::string> const& makeOptions)
{
  std::vector<GeneratedMakeCommand> makeCommands;
  // Select the caller- or user-preferred make program, else MSBuild.
  cmProp AtmelStudioPath = this->CMakeInstance->GetState()->GetCacheEntryValue("CMAKE_ATMEL_STUDIO_EXECUTABLE");
  if (!AtmelStudioPath) {
    GeneratedMakeCommand makeCommand;
    makeCommand.Add("Could not find CMAKE_ATMEL_STUDIO_EXECUTABLE in cache !");
    makeCommands.push_back(makeCommand);
    return makeCommands;
  }

  // "C:\Program Files (x86)\Atmel\Studio\7.0\AtmelStudio.exe" GETTING - STARTED14.atsln / build debug / out log.txt "
  GeneratedMakeCommand makeCommand;
  makeCommand.Add(*AtmelStudioPath);
  std::string atslnFile = projectDir + "/" + projectName + ".atsln";
  makeCommand.Add(atslnFile);
  makeCommand.Add("/build");
  makeCommand.Add(config);

  makeCommands.push_back(makeCommand);

  return makeCommands;
}

// Write a dsp file into the SLN file, Note, that dependencies from
// executables to the libraries it uses are also done here
void cmGlobalAtmelStudio7Generator::WriteExternalProject(
  std::ostream& fout, const std::string& name, const std::string& location,
  const char* typeGuid,
  const std::set<BT<std::pair<std::string, bool>>>& depends)
{
  fout << "Project(\"{"
       << (typeGuid ? typeGuid : this->ExternalProjectType(location))
       << "}\") = \"" << name << "\", \""
       << this->ConvertToSolutionPath(location) << "\", \"{"
       << this->GetGUID(name) << "}\"\n";

  // write out the dependencies here VS 7.1 includes dependencies with the
  // project instead of in the global section
  if (!depends.empty()) {
    fout << "\tProjectSection(ProjectDependencies) = postProject\n";
    for (BT<std::pair<std::string, bool>> const& it : depends) {
      std::string const& dep = it.Value.first;
      if (!dep.empty()) {
        fout << "\t\t{" << this->GetGUID(dep) << "} = {" << this->GetGUID(dep)
             << "}\n";
      }
    }
    fout << "\tEndProjectSection\n";
  }

  fout << "EndProject\n";
}

const char* cmGlobalAtmelStudio7Generator::ExternalProjectType(
  const std::string& location)
{
  std::string extension = cmSystemTools::GetFilenameLastExtension(location);
  return "8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942";
}

void cmGlobalAtmelStudio7Generator::WriteTargetsToSolution(
  std::ostream& fout, cmLocalGenerator* root,
  OrderedTargetDependSet const& projectTargets)
{
  AtmelStudioFolders.clear();

  std::string rootBinaryDir = root->GetCurrentBinaryDirectory();
  for (cmGeneratorTarget const* target : projectTargets) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    bool written = false;

    // handle external vc project files
    cmProp expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if (expath) {
      std::string project = target->GetName();
      std::string location = *expath;

      cmProp p = target->GetProperty("VS_PROJECT_TYPE");
      this->WriteExternalProject(fout, project, location, cmToCStr(p),
                                 target->GetUtilities());
      written = true;
    } else {
      cmProp cprojName = target->GetProperty("GENERATOR_FILE_NAME");
      if (cprojName) {
        cmLocalGenerator* lg = target->GetLocalGenerator();
        std::string dir = lg->GetCurrentBinaryDirectory();
        dir = root->MaybeConvertToRelativePath(rootBinaryDir, dir);
        if (dir == ".") {
          dir.clear(); // msbuild cannot handle ".\" prefix
        }
        this->WriteProject(fout, *cprojName, dir, target);
        written = true;
      }
    }

    // Create "solution folder" information from FOLDER target property
    //
    if (written && this->UseFolderProperty()) {
      const std::string targetFolder = target->GetEffectiveFolderName();
      if (!targetFolder.empty()) {
        std::vector<std::string> tokens =
          cmSystemTools::SplitString(targetFolder, '/', false);

        std::string cumulativePath;

        for (std::string const& iter : tokens) {
          if (!iter.size()) {
            continue;
          }

          if (cumulativePath.empty()) {
            cumulativePath = "CMAKE_FOLDER_GUID_" + iter;
          } else {
            AtmelStudioFolders[cumulativePath].insert(cumulativePath + "/" +
                                                      iter);

            cumulativePath = cumulativePath + "/" + iter;
          }
        }

        if (!cumulativePath.empty()) {
          AtmelStudioFolders[cumulativePath].insert(target->GetName());
        }
      }
    }
  }
}

std::string cmGlobalAtmelStudio7Generator::ConvertToSolutionPath(
  const std::string& path)
{
  // Convert to backslashes.  Do not use ConvertToOutputPath because
  // we will add quoting ourselves, and we know these projects always
  // use windows slashes.
  std::string d = path;
  std::string::size_type pos = 0;
  while ((pos = d.find('/', pos)) != d.npos) {
    d[pos++] = '\\';
  }
  return d;
}

void cmGlobalAtmelStudio7Generator::WriteProjectDepends(
  std::ostream& fout, const std::string&, const std::string&,
  cmGeneratorTarget const* gt)
{
  TargetDependSet const& unordered = this->GetTargetDirectDepends(gt);
  OrderedTargetDependSet depends(unordered, std::string());
  for (cmTargetDepend const& i : depends) {
    if (!i->IsInBuildSystem()) {
      continue;
    }
    std::string guid = this->GetGUID(i->GetName());
    fout << "\t\t{" << guid << "} = {" << guid << "}\n";
  }
}

// Write a dsp file into the SLN file,
// Note, that dependencies from executables to
// the libraries it uses are also done here
void cmGlobalAtmelStudio7Generator::WriteProject(std::ostream& fout,
                                                 const std::string& dspname,
                                                 const std::string& dir,
                                                 cmGeneratorTarget const* t)
{
  // check to see if this is a fortran build
  std::string ext = ".vcproj";
  const char* project =
    "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"";

  cmProp targetExt = t->GetProperty("GENERATOR_FILE_NAME_EXT");
  if (targetExt) {
    ext = *targetExt;
  }

  std::string guid = this->GetGUID(dspname);
  fout << project << dspname << "\", \"" << this->ConvertToSolutionPath(dir)
       << (!dir.empty() ? "\\" : "") << dspname << ext << "\", \"{" << guid
       << "}\"\n";
  fout << "\tProjectSection(ProjectDependencies) = postProject\n";
  this->WriteProjectDepends(fout, dspname, dir, t);
  fout << "\tEndProjectSection\n";

  fout << "EndProject\n";

  UtilityDependsMap::iterator ui = this->UtilityDepends.find(t);
  if (ui != this->UtilityDepends.end()) {
    const char* uname = ui->second.c_str();
    /* clang-format off */
    fout << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \""
         << uname << "\", \""
         << this->ConvertToSolutionPath(dir) << (dir[0]? "\\":"")
         << uname << ".vcproj" << "\", \"{"
         << this->GetGUID(uname) << "}\"\n"
         << "\tProjectSection(ProjectDependencies) = postProject\n"
         << "\t\t{" << guid << "} = {" << guid << "}\n"
         << "\tEndProjectSection\n"
         << "EndProject\n";
    /* clang-format on */
  }
}

void cmGlobalAtmelStudio7Generator::WriteTargetConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs,
  OrderedTargetDependSet const& projectTargets)
{
  // loop over again and write out configurations for each target
  // in the solution
  for (cmGeneratorTarget const* target : projectTargets) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    cmProp expath = target->GetProperty("EXTERNAL_MSPROJECT");
    if (expath) {
      std::set<std::string> allConfigurations(configs.begin(), configs.end());
      cmProp mapping = target->GetProperty("VS_PLATFORM_MAPPING");
      this->WriteProjectConfigurations(fout, target->GetName(), *target,
                                       configs, allConfigurations,
                                       mapping ? *mapping : "");
    } else {
      const std::set<std::string>& configsPartOfDefaultBuild =
        this->IsPartOfDefaultBuild(configs, projectTargets, target);
      cmProp vcprojName = target->GetProperty("GENERATOR_FILE_NAME");
      if (vcprojName) {
        this->WriteProjectConfigurations(fout, *vcprojName, *target, configs,
                                         configsPartOfDefaultBuild);
      }
    }
  }
}

void cmGlobalAtmelStudio7Generator::WriteFolders(std::ostream& fout)
{
  cm::string_view const prefix = "CMAKE_FOLDER_GUID_";
  std::string guidProjectTypeFolder = "2150E333-8FDC-42A3-9474-1A3956D46DE8";
  for (auto const& iter : AtmelStudioFolders) {
    std::string fullName = iter.first;
    std::string guid = this->GetGUID(fullName);

    std::replace(fullName.begin(), fullName.end(), '/', '\\');
    if (cmHasPrefix(fullName, prefix)) {
      fullName = fullName.substr(prefix.size());
    }

    std::string nameOnly = cmSystemTools::GetFilenameName(fullName);

    fout << "Project(\"{" << guidProjectTypeFolder << "}\") = \"" << nameOnly
         << "\", \"" << fullName << "\", \"{" << guid << "}\"\nEndProject\n";
  }
}

void cmGlobalAtmelStudio7Generator::WriteFoldersContent(std::ostream& fout)
{
  for (auto const& iter : AtmelStudioFolders) {
    std::string key(iter.first);
    std::string guidParent(this->GetGUID(key));

    for (std::string const& it : iter.second) {
      std::string value(it);
      std::string guid(this->GetGUID(value));

      fout << "\t\t{" << guid << "} = {" << guidParent << "}\n";
    }
  }
}

void cmGlobalAtmelStudio7Generator::WriteProjectConfigurations(
  std::ostream& fout, const std::string& name, cmGeneratorTarget const& target,
  std::vector<std::string> const& configs,
  const std::set<std::string>& configsPartOfDefaultBuild,
  const std::string& platformMapping)
{
  std::string guid = this->GetGUID(name);
  for (std::string const& i : configs) {
    std::vector<std::string> mapConfig;
    const char* dstConfig = i.c_str();
    if (target.GetProperty("EXTERNAL_MSPROJECT")) {
      if (cmProp m = target.GetProperty("MAP_IMPORTED_CONFIG_" +
                                        cmSystemTools::UpperCase(i))) {
        cmExpandList(*m, mapConfig);
        if (!mapConfig.empty()) {
          dstConfig = mapConfig[0].c_str();
        }
      }
    }

    std::string platformName = GetPlatform(CurrentPlatform);
    fout << "\t\t{" << guid << "}." << i << "|" << platformName
         << ".ActiveCfg = " << dstConfig << "|"
         << (!platformMapping.empty() ? platformMapping : platformName)
         << "\n";
    std::set<std::string>::const_iterator ci =
      configsPartOfDefaultBuild.find(i);

    if (!(ci == configsPartOfDefaultBuild.end())) {
      fout << "\t\t{" << guid << "}." << i << "|" << platformName
           << ".Build.0 = " << dstConfig << "|"
           << (!platformMapping.empty() ? platformMapping : platformName)
           << "\n";
    }
  }
}

void cmGlobalAtmelStudio7Generator::WriteSolutionConfigurations(
  std::ostream& fout, std::vector<std::string> const& configs)
{
  fout << "\tGlobalSection(SolutionConfiguration) = preSolution\n";
  for (std::string const& i : configs) {
    fout << "\t\t" << i << " = " << i << "\n";
  }
  fout << "\tEndGlobalSection\n";
}

void cmGlobalAtmelStudio7Generator::WriteATSLNGlobalSections(
  std::ostream& fout, cmLocalGenerator* root)
{
  std::string const guid =
    this->GetGUID(root->GetProjectName() + SolutionFileExtension);
  bool extensibilityGlobalsOverridden = false;
  bool extensibilityAddInsOverridden = false;
  const std::vector<std::string> propKeys =
    root->GetMakefile()->GetPropertyKeys();

  for (std::string const& it : propKeys) {
    if (cmHasLiteralPrefix(it, "VS_GLOBAL_SECTION_")) {
      std::string sectionType;
      std::string name = it.substr(18);
      if (cmHasLiteralPrefix(name, "PRE_")) {
        name = name.substr(4);
        sectionType = "preSolution";
      } else if (cmHasLiteralPrefix(name, "POST_")) {
        name = name.substr(5);
        sectionType = "postSolution";
      } else
        continue;
      if (!name.empty()) {
        bool addGuid = false;
        if (name == "ExtensibilityGlobals" && sectionType == "postSolution") {
          addGuid = true;
          extensibilityGlobalsOverridden = true;
        } else if (name == "ExtensibilityAddIns" &&
                   sectionType == "postSolution") {
          extensibilityAddInsOverridden = true;
        }
        fout << "\tGlobalSection(" << name << ") = " << sectionType << "\n";
        cmProp p = root->GetMakefile()->GetProperty(it);
        std::vector<std::string> keyValuePairs = cmExpandedList(p ? *p : "");
        for (std::string const& itPair : keyValuePairs) {
          const std::string::size_type posEqual = itPair.find('=');
          if (posEqual != std::string::npos) {
            const std::string key =
              cmTrimWhitespace(itPair.substr(0, posEqual));
            const std::string value =
              cmTrimWhitespace(itPair.substr(posEqual + 1));
            fout << "\t\t" << key << " = " << value << "\n";
            if (key == "SolutionGuid") {
              addGuid = false;
            }
          }
        }
        if (addGuid) {
          fout << "\t\tSolutionGuid = {" << guid << "}\n";
        }
        fout << "\tEndGlobalSection\n";
      }
    }
  }
  if (!extensibilityGlobalsOverridden) {
    fout << "\tGlobalSection(ExtensibilityGlobals) = postSolution\n"
         << "\t\tSolutionGuid = {" << guid << "}\n"
         << "\tEndGlobalSection\n";
  }
  if (!extensibilityAddInsOverridden)
    fout << "\tGlobalSection(ExtensibilityAddIns) = postSolution\n"
         << "\tEndGlobalSection\n";
}

void cmGlobalAtmelStudio7Generator::WriteATSLNFooter(std::ostream& fout)
{
  fout << "EndGlobal\n";
}

std::set<std::string> cmGlobalAtmelStudio7Generator::IsPartOfDefaultBuild(
  std::vector<std::string> const& configs,
  OrderedTargetDependSet const& projectTargets,
  cmGeneratorTarget const* target)
{
  std::set<std::string> activeConfigs;
  // if it is a utilitiy target then only make it part of the
  // default build if another target depends on it
  int type = target->GetType();
  if (type == cmStateEnums::GLOBAL_TARGET) {
    std::vector<std::string> targetNames;
    targetNames.push_back("INSTALL");
    targetNames.push_back("PACKAGE");
    for (std::string const& t : targetNames) {
      // check if target <t> is part of default build
      if (target->GetName() == t) {
        const std::string propertyName =
          "CMAKE_VS_INCLUDE_" + t + "_TO_DEFAULT_BUILD";
        // inspect CMAKE_VS_INCLUDE_<t>_TO_DEFAULT_BUILD properties
        for (std::string const& i : configs) {
          cmProp propertyValue =
            target->Target->GetMakefile()->GetDefinition(propertyName);
          if (propertyValue &&
              cmIsOn(cmGeneratorExpression::Evaluate(
                *propertyValue, target->GetLocalGenerator(), i))) {
            activeConfigs.insert(i);
          }
        }
      }
    }
    return activeConfigs;
  }
  if (type == cmStateEnums::UTILITY &&
      !this->IsDependedOn(projectTargets, target)) {
    return activeConfigs;
  }
  // inspect EXCLUDE_FROM_DEFAULT_BUILD[_<CONFIG>] properties
  for (std::string const& i : configs) {
    if (cmIsOff(target->GetFeature("EXCLUDE_FROM_DEFAULT_BUILD", i))) {
      activeConfigs.insert(i);
    }
  }
  return activeConfigs;
}

bool cmGlobalAtmelStudio7Generator::IsDependedOn(
  OrderedTargetDependSet const& projectTargets, cmGeneratorTarget const* gtIn)
{
  for (cmTargetDepend const& l : projectTargets) {
    TargetDependSet const& tgtdeps = this->GetTargetDirectDepends(l);
    if (tgtdeps.count(gtIn)) {
      return true;
    }
  }
  return false;
}

cmGlobalAtmelStudio7Generator::OrderedTargetDependSet::OrderedTargetDependSet(
  TargetDependSet const& targets, std::string const& first)
  : derived(TargetCompare(first))
{
  this->insert(targets.begin(), targets.end());
}

cmGlobalAtmelStudio7Generator::OrderedTargetDependSet::OrderedTargetDependSet(
  TargetSet const& targets, std::string const& first)
  : derived(TargetCompare(first))
{
  for (cmGeneratorTarget const* it : targets) {
    this->insert(it);
  }
}

bool cmGlobalAtmelStudio7Generator::TargetCompare::operator()(
  cmGeneratorTarget const* l, cmGeneratorTarget const* r) const
{
  // Make sure a given named target is ordered first,
  // e.g. to set ALL_BUILD as the default active project.
  // When the empty string is named this is a no-op.
  if (r->GetName() == this->First) {
    return false;
  }
  if (l->GetName() == this->First) {
    return true;
  }
  return l->GetName() < r->GetName();
}