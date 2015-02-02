//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#include "cling/Interpreter/InvocationOptions.h"
#include "cling/Interpreter/ClingOptions.h"

#include "clang/Driver/Arg.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/Option.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/OptTable.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::driver;
using namespace cling::driver::clingoptions;

namespace {

  static const OptTable::Info ClingInfoTable[] = {
#define OPTION(NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM,              \
               HELPTEXT, METAVAR)                                       \
    { NAME, HELPTEXT, METAVAR, Option::KIND##Class, FLAGS, PARAM,       \
      OPT_##GROUP, OPT_##ALIAS },
#include "cling/Interpreter/ClingOptions.inc"
#undef OPTION
  };

  class ClingOptTable : public OptTable {
  public:
    ClingOptTable()
      : OptTable(ClingInfoTable, sizeof(ClingInfoTable) / sizeof(ClingInfoTable[0])) {}
  };

  static OptTable* CreateClingOptTable() {
    return new ClingOptTable();
  }

  static void ParseStartupOpts(cling::InvocationOptions& Opts,
                               InputArgList& Args /* , Diags */) {
    Opts.NoLogo = Args.hasArg(OPT_nologo);
    Opts.ShowVersion = Args.hasArg(OPT_version);
    Opts.Verbose = Args.hasArg(OPT_v);
    Opts.Help = Args.hasArg(OPT_help);
  }

  static void ParseLinkerOpts(cling::InvocationOptions& Opts,
                              InputArgList& Args /* , Diags */) {
    Opts.LibsToLoad = Args.getAllArgValues(OPT_l);
    std::vector<std::string> LibPaths = Args.getAllArgValues(OPT_L);
    for (size_t i = 0; i < LibPaths.size(); ++i)
      Opts.LibSearchPath.push_back(llvm::sys::Path(LibPaths[i]));
  }

}

cling::InvocationOptions
cling::InvocationOptions::CreateFromArgs(int argc, const char* const argv[],
                                         std::vector<unsigned>& leftoverArgs
                                         /* , Diagnostic &Diags */) {
  InvocationOptions ClingOpts;
  llvm::OwningPtr<OptTable> Opts(CreateClingOptTable());
  unsigned MissingArgIndex, MissingArgCount;
  llvm::OwningPtr<InputArgList> Args(
    Opts->ParseArgs(argv, argv + argc, MissingArgIndex, MissingArgCount));

  //if (MissingArgCount)
  //  Diags.Report(diag::err_drv_missing_argument)
  //    << Args->getArgString(MissingArgIndex) << MissingArgCount;

  // Forward unknown arguments.
  for (ArgList::const_iterator it = Args->begin(),
         ie = Args->end(); it != ie; ++it) {
    if ((*it)->getOption().getKind() == Option::UnknownClass
        ||(*it)->getOption().getKind() == Option::InputClass) {
      leftoverArgs.push_back((*it)->getIndex());
    }
  }
  ParseStartupOpts(ClingOpts, *Args /* , Diags */);
  ParseLinkerOpts(ClingOpts, *Args /* , Diags */);
  return ClingOpts;
}

void cling::InvocationOptions::PrintHelp() {
  llvm::OwningPtr<OptTable> Opts(CreateClingOptTable());
  Opts->PrintHelp(llvm::outs(), "cling",
                  "cling: LLVM/clang C++ Interpreter: http://root.cern.ch/drupal/content/cling");

  llvm::OwningPtr<OptTable> OptsC1(createDriverOptTable());
  OptsC1->PrintHelp(llvm::outs(), "clang -cc1",
                    "LLVM 'Clang' Compiler: http://clang.llvm.org");
  
}
