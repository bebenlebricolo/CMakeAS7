/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include "cmLocalGenerator.h"

class cmGlobalGenerator;
class cmMakefile;

/** \class cmLocalAtmelStudio7Generator
 * \brief Write Atmel Studio 7 project files.
 *
 * cmLocalAtmelStudio7Generator produces an Atmel Studio 7 project
 * file for each target in its directory.
 */
class cmLocalAtmelStudio7Generator : public cmLocalGenerator
{
public:
  //! Set cache only and recurse to false by default.
  cmLocalAtmelStudio7Generator(cmGlobalGenerator* gg, cmMakefile* mf);

  virtual ~cmLocalAtmelStudio7Generator();

  /**
   * Generate the makefile for this directory.
   */
  void Generate() override;
  void GenerateTarget(cmGeneratorTarget* target);
  void ReadAndStoreExternalGUID(const std::string& name,
                                const char* path);

  void WriteStampFiles();

  std::set<cmSourceFile const*>& GetSourcesVisited(
    cmGeneratorTarget const* target)
  {
    return this->SourcesVisited[const_cast<cmGeneratorTarget*>(target)];
  };

protected:
  const char* ReportErrorLabel() const;
  bool CustomCommandUseLocal() const { return true; }

private:
  void GenerateTargetsDepthFirst(cmGeneratorTarget* target,
                                 std::vector<cmGeneratorTarget*>& remaining);

  std::map<cmGeneratorTarget*, std::set<cmSourceFile const*>> SourcesVisited;
};
