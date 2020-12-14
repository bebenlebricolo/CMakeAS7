#pragma once

#include <string>

#include "cmConfigure.h" // IWYU pragma: keep
#include "cmLocalGenerator.h"

class cmMakefile;
class cmGlobalGenerator;

/**
 * @brief Write Atmel Studio 7 project files.
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
  * @brief Generate the makefile for this directory.
   */
  void Generate() override;

  /**
   * @brief generates AS7 project files for the given target representation.
   *
   * @param target	: pointer to the target to be generated
   */
  void GenerateTarget(cmGeneratorTarget* target);

  /**
   * @brief writes a timestamp file used to determine if the current project generation
   * is out of date and needs to be regenerated.
   */
  void WriteStampFiles();

  std::set<cmSourceFile const*>& GetSourcesVisited(cmGeneratorTarget const* target);

protected:
  /**
   * @brief get the standardized Visual Studio encoded error label used to report error.
   * @return the error label
   */
  const char* ReportErrorLabel() const;

private:
  std::map<cmGeneratorTarget*, std::set<cmSourceFile const*>> SourcesVisited;
};
