/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
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

void cmLocalAtmelStudio7Generator::GenerateTarget(cmGeneratorTarget* target)
{
 // cmAtmelStudio7TargetGenerator targetGenerator(
 //   target,
 //   static_cast<cmGlobalAtmelStudio7Generator*>(this->GetGlobalGenerator()));
 // targetGenerator.Generate();
}

void cmLocalAtmelStudio7Generator::ReadAndStoreExternalGUID(
  const std::string& name, const char* path)
{
  // Parse previously written file and extract GUID from it if found
  //  cmVS10XMLParser parser;
  // parser.ParseFile(path);

  // if we can not find a GUID then we will generate one later
  //if (parser.GUID.empty()) {
  //  return;
  //}

  //std::string guidStoreName = cmStrCat(name, "_GUID_CMAKE");
  //// save the GUID in the cache
  //this->GlobalGenerator->GetCMakeInstance()->AddCacheEntry(
  //  guidStoreName, parser.GUID.c_str(), "Stored GUID", cmStateEnums::INTERNAL);
}

const char* cmLocalAtmelStudio7Generator::ReportErrorLabel() const
{
  return ":VCEnd";
}
