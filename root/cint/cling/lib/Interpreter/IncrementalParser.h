//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Axel Naumann <axel@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INCREMENTAL_PARSER_H
#define CLING_INCREMENTAL_PARSER_H

#include "ChainedConsumer.h"
#include "cling/Interpreter/CompilationOptions.h"

#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/SourceLocation.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringRef.h"

#include <vector>

namespace llvm {
  struct GenericValue;
  class MemoryBuffer;
}

namespace clang {
  class ASTConsumer;
  class CodeGenerator;
  class CompilerInstance;
  class Decl;
  class FileID;
  class FunctionDecl;
  class Parser;
  class Sema;
  class SourceLocation;
}


namespace cling {
  class ChainedConsumer;
  class CIFactory;
  class ExecutionContext;
  class Interpreter;
  
  class IncrementalParser {
  public:
    ///\brief Contains information about the last input
    struct Transaction {      
    private:
      clang::Decl* m_BeforeFirst;
      clang::Decl* m_LastDecl;
      void setBeforeFirstDecl(clang::DeclContext* DC) {
        class DeclContextExt : public clang::DeclContext {            
        public:           
          static clang::Decl* getLastDecl(DeclContext* DC) {
            return ((DeclContextExt*)DC)->LastDecl;
          }
        };
        m_BeforeFirst = DeclContextExt::getLastDecl(DC);
      }
    public:
      clang::Decl* getFirstDecl() const {
        return m_BeforeFirst->getNextDeclInContext();
      }
      clang::Decl* getLastDeclSlow() const {
        clang::Decl* Result = getFirstDecl();
        while (Result->getNextDeclInContext())
          Result = Result->getNextDeclInContext();

        return Result;
      }
      friend class IncrementalParser;
    };
    enum EParseResult {
      kSuccess,
      kSuccessWithWarnings,
      kFailed
    };
    IncrementalParser(Interpreter* interp, int argc, const char* const *argv,
                      const char* llvmdir);
    ~IncrementalParser();
    void Initialize();
    clang::CompilerInstance* getCI() const { return m_CI.get(); }
    clang::Parser* getParser() const { return m_Parser.get(); }

    ///\brief Compiles the given input with the given compilation options.
    ///
    EParseResult Compile(llvm::StringRef input, const CompilationOptions& Opts);

    void Parse(llvm::StringRef input, 
               llvm::SmallVector<clang::DeclGroupRef, 4>& DGRs);

    llvm::MemoryBuffer* getCurBuffer() const {
      return m_MemoryBuffer.back();
    }
    void enablePrintAST(bool print /*=true*/) {
      m_Consumer->getCompilationOpts().Debug = print;
    }
    void enableDynamicLookup(bool value = true);
    bool isDynamicLookupEnabled() const { return m_DynamicLookupEnabled; }
    bool isSyntaxOnly() const { return m_SyntaxOnly; }
    clang::Decl* getFirstTopLevelDecl() const { return m_FirstTopLevelDecl; }
    clang::Decl* getLastTopLevelDecl() const { return m_LastTopLevelDecl; }
    Transaction& getLastTransaction() { return m_LastTransaction; }
    
    clang::CodeGenerator* GetCodeGenerator() const;

  private:
    void CreateSLocOffsetGenerator();
    EParseResult Compile(llvm::StringRef input);
    EParseResult Parse(llvm::StringRef input);

    Interpreter* m_Interpreter; // our interpreter context
    llvm::OwningPtr<clang::CompilerInstance> m_CI; // compiler instance.
    llvm::OwningPtr<clang::Parser> m_Parser; // parser (incremental)
    bool m_DynamicLookupEnabled; // enable/disable dynamic scope
    std::vector<llvm::MemoryBuffer*> m_MemoryBuffer; // One buffer for each command line, owner by the source file manager
    clang::FileID m_VirtualFileID; // file ID of the memory buffer
    ChainedConsumer* m_Consumer; // CI owns it
    clang::Decl* m_FirstTopLevelDecl; // first top level decl
    clang::Decl* m_LastTopLevelDecl; // last top level decl after most recent call to parse()
    Transaction m_LastTransaction; // Holds information for the last transaction
    bool m_SyntaxOnly; // whether to run codegen; cannot be flipped during lifetime of *this
  };
}
#endif // CLING_INCREMENTAL_PARSER_H
