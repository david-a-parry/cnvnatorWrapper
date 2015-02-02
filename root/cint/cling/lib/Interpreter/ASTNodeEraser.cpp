//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vvasilev@cern.ch>
//------------------------------------------------------------------------------

#include "ASTNodeEraser.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/DependentDiagnostic.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/FileManager.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/Sema.h"

using namespace clang;

namespace cling {

  ///\brief The class does the actual work of removing a declaration and 
  /// resetting the internal structures of the compiler
  ///
  class DeclReverter : public DeclVisitor<DeclReverter, bool> {
  private:
    Sema* m_Sema;

  public:
    DeclReverter(Sema* S): m_Sema(S) {}

    ///\brief Function that contains common actions, done for every removal of
    /// declaration.
    ///
    /// For example: We must uncache the cached include, which brought that 
    /// declaration in the AST.
    ///\param[in] D - A declaration.
    ///
    void PreVisitDecl(Decl* D);

    ///\brief If it falls back in the base class just remove the declaration
    /// only from the declaration context. 
    /// @param[in] D - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitDecl(Decl* D);

    ///\brief Removes the declaration from the lookup chains and from the
    /// declaration context.
    /// @param[in] ND - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitNamedDecl(NamedDecl* ND);

    ///\brief Removes the declaration from the lookup chains and from the
    /// declaration context and it rebuilds the redeclaration chain.
    /// @param[in] VD - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitVarDecl(VarDecl* VD);

    ///\brief Removes the declaration from the lookup chains and from the
    /// declaration context and it rebuilds the redeclaration chain.
    /// @param[in] FD - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitFunctionDecl(FunctionDecl* FD);

    ///\brief Removes the enumerator and its enumerator constants.
    /// @param[in] ED - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitEnumDecl(EnumDecl* ED);


    ///\brief Removes the namespace.
    /// @param[in] NSD - The declaration to be removed.
    ///
    ///\returns true on success.
    ///
    bool VisitNamespaceDecl(NamespaceDecl* NSD);

    /// @name Helpers
    /// @{

    ///\brief Checks whether the declaration was pushed onto the declaration
    /// chains. Returns 
    /// @param[in] ND - The declaration that is being checked
    ///
    ///\returns true if the ND was found in the lookup chain.
    ///
    bool isOnScopeChains(clang::NamedDecl* ND);

    ///\brief Removes given declaration from the chain of redeclarations.
    /// Rebuilds the chain and sets properly first and last redeclaration.
    /// @param[in] R - The redeclarable, its chain to be rebuilt
    ///
    ///\returns the most recent redeclaration in the new chain.
    ///
    template <typename T>
    T* RemoveFromRedeclChain(clang::Redeclarable<T>* R) {
      llvm::SmallVector<T*, 4> PrevDecls;
      T* PrevDecl = 0;

      // [0]=>C [1]=>B [2]=>A ...
      while ((PrevDecl = R->getPreviousDecl())) {
        PrevDecls.push_back(PrevDecl);
        R = PrevDecl;
      }

      if (!PrevDecls.empty()) {
        // Put 0 in the end of the array so that the loop will reset the 
        // pointer to latest redeclaration in the chain to itself.
        //
        PrevDecls.push_back(0);

        // 0 <- A <- B <- C 
        for(unsigned i = PrevDecls.size() - 1; i > 0; --i) {
          PrevDecls[i-1]->setPreviousDeclaration(PrevDecls[i]);
        }
      }

      return PrevDecls.empty() ? 0 : PrevDecls[0]->getMostRecentDecl();
    }

    /// @}
  };

  void DeclReverter::PreVisitDecl(Decl *D) {
    //SourceLocation Loc = D->getLocStart();
    //SourceManager& SM = m_Sema->getSourceManager();
    //FileManager& FM = SM.getFileManager();
    //const FileEntry* OldEntry = SM.getFileEntryForID(SM.getFileID(Loc));
    //const FileEntry* NewEntry 
    //  = FM.getFile(OldEntry->getName(), /*openFile*/ true);
    //std::string errStr = "";
    // SM.overrideFileContents(OldEntry, FM.getBufferForFile(NewEntry, &errStr));
  }

  // Gives us access to the protected members that we  need.
  class DeclContextExt : public DeclContext {
  public:
    static bool removeIfLast(DeclContext* DC, Decl* D) {
      if (!D->getNextDeclInContext()) {
        // Either last (remove!), or invalid (nothing to remove)
        if (((DeclContextExt*)DC)->LastDecl == D) {
          // Valid. Thus remove.
          DC->removeDecl(D);
          return true;
        }
      } 
      else {
        DC->removeDecl(D);
        return true;
      }

      return false;
    }
  };

  bool DeclReverter::VisitDecl(Decl* D) {
    assert(D && "The Decl is null"); 
    PreVisitDecl(D);

    DeclContext* DC = D->getDeclContext();

    bool ExistsInDC = false;

    for (DeclContext::decl_iterator I = DC->decls_begin(), E = DC->decls_end();
         E !=I; ++I) {
      if (*I == D) {
        ExistsInDC = true;
        break;
      }
    }

    bool Successful = DeclContextExt::removeIfLast(DC, D); 

    // ExistsInDC && Successful 
    // true          false      -> false // In the context but cannot delete
    // false         false      -> true  // Not in the context cannot delete
    // true          true       -> true  // In the context and can delete
    // false         true       -> assert // Not in the context but can delete ?
    assert(!(!ExistsInDC && Successful) && "Not in the context but can delete?!");
    if (ExistsInDC && !Successful)
      return false;
    else // in release we'd want the assert to fall into true
      return true;
  }

  bool DeclReverter::VisitNamedDecl(NamedDecl* ND) {
    bool Successful = VisitDecl(ND);

    DeclContext* DC = ND->getDeclContext();
    
    // If the decl was removed make sure that we fix the lookup
    if (Successful) {
      Scope* S = m_Sema->getScopeForContext(DC);
      if (S)
        S->RemoveDecl(ND);
      
      if (isOnScopeChains(ND))
        m_Sema->IdResolver.RemoveDecl(ND);

      return true;
    }

    return false;
  }

  bool DeclReverter::VisitVarDecl(VarDecl* VD) {
    bool Successful = VisitNamedDecl(VD);

    DeclContext* DC = VD->getDeclContext();
    Scope* S = m_Sema->getScopeForContext(DC);

    // Find other decls that the old one has replaced
    StoredDeclsMap *Map = DC->getPrimaryContext()->getLookupPtr();
    if (!Map) 
      return false;
    StoredDeclsMap::iterator Pos = Map->find(VD->getDeclName());
    assert(Pos != Map->end() && "no lookup entry for decl");
    
    if (Pos->second.isNull())
      // We need to rewire the list of the redeclarations in order to exclude
      // the reverted one, because it gets found for example by 
      // Sema::MergeVarDecl and ends up in the lookup
      //
      if (VarDecl* MostRecentVD = RemoveFromRedeclChain(VD)) {
        
        Pos->second.setOnlyValue(MostRecentVD);
        if (S)
          S->AddDecl(MostRecentVD);
        m_Sema->IdResolver.AddDecl(MostRecentVD);
      }

    return Successful;
  }

  bool DeclReverter::VisitFunctionDecl(FunctionDecl* FD) {
    bool Successful = true;

    DeclContext* DC = FD->getDeclContext();
    Scope* S = m_Sema->getScopeForContext(DC);

    // Template instantiation of templated function first creates a canonical
    // declaration and after the actual template specialization. For example:
    // template<typename T> T TemplatedF(T t);
    // template<> int TemplatedF(int i) { return i + 1; } creates:
    // 1. Canonical decl: int TemplatedF(int i);
    // 2. int TemplatedF(int i){ return i + 1; }
    //
    // The template specialization is attached to the list of specialization of
    // the templated function.
    // When TemplatedF is looked up it finds the templated function and the 
    // lookup is extended by the templated function with its specializations.
    // In the end we don't need to remove the canonical decl because, it
    // doesn't end up in the lookup table.
    //
#if 0
    getSpecializations() now returns a FoldingSetVector which
    does not have an interface for removing nodes...
    class FunctionTemplateDeclExt : public FunctionTemplateDecl {
    public:
      static llvm::FoldingSet<FunctionTemplateSpecializationInfo>& 
      getSpecializationsExt(FunctionTemplateDecl* FTD) {
        assert(FTD && "Cannot be null!");
        return ((FunctionTemplateDeclExt*) FTD)->getSpecializations();
      }
    };
#endif

    if (FD->isFunctionTemplateSpecialization()) {
#if 0
      // 1. Remove the canonical decl.
      // TODO: Can the canonical have another DeclContext and Scope, different
      // from the specialization's implementation?
      FunctionDecl* CanFD = FD->getCanonicalDecl();
      FunctionTemplateDecl* FTD 
        = FD->getTemplateSpecializationInfo()->getTemplate();
      llvm::FoldingSet<FunctionTemplateSpecializationInfo> &FS 
        = FunctionTemplateDeclExt::getSpecializationsExt(FTD);
      FS.RemoveNode(CanFD->getTemplateSpecializationInfo());
#endif
      assert("FunctionTemplateSpecialization not handled yet" && 0);
    }

    // Find other decls that the old one has replaced
    StoredDeclsMap *Map = DC->getPrimaryContext()->getLookupPtr();
    if (!Map) 
      return false;      
    StoredDeclsMap::iterator Pos = Map->find(FD->getDeclName());
    assert(Pos != Map->end() && "no lookup entry for decl");

    if (Pos->second.getAsDecl()) {
      Successful = VisitNamedDecl(FD) && Successful;

      Pos = Map->find(FD->getDeclName());
      assert(Pos != Map->end() && "no lookup entry for decl");

      if (Pos->second.isNull()) {
        // When we have template specialization we have to clean up
        if (FD->isFunctionTemplateSpecialization()) {
          while ((FD = FD->getPreviousDecl())) {
            Successful = VisitNamedDecl(FD) && Successful;
          }
          return true;
        }

        // We need to rewire the list of the redeclarations in order to exclude
        // the reverted one, because it gets found for example by 
        // Sema::MergeVarDecl and ends up in the lookup
        //
        if (FunctionDecl* MostRecentFD = RemoveFromRedeclChain(FD)) {
          Pos->second.setOnlyValue(MostRecentFD);
          if (S)
            S->AddDecl(MostRecentFD);
          m_Sema->IdResolver.AddDecl(MostRecentFD);
        }
      }
    }
    else if (llvm::SmallVector<NamedDecl*, 4>* Decls 
             = Pos->second.getAsVector()) {
      for(llvm::SmallVector<NamedDecl*, 4>::iterator I = Decls->begin();
          I != Decls->end(); ++I) {
        if ((*I) == FD) {
          if (FunctionDecl* MostRecentFD = RemoveFromRedeclChain(FD)) {
            Successful = VisitNamedDecl(*I) && Successful;
            Decls->insert(I, MostRecentFD);
          }
          else
            Decls->erase(I);
        }
      }
    }

    return Successful;
  }

  bool DeclReverter::VisitEnumDecl(EnumDecl* ED) {
    bool Successful = true;

    for (EnumDecl::enumerator_iterator I = ED->enumerator_begin(),
           E = ED->enumerator_end(); I != E; ++I) {
      assert(I->getDeclName() && "EnumConstantDecl with no name?");
      Successful = VisitNamedDecl(&(*I)) && Successful;
    }

    Successful = VisitNamedDecl(ED) && Successful; 

    return Successful;
  }

  bool DeclReverter::VisitNamespaceDecl(NamespaceDecl* NSD) {
    bool Successful = VisitNamedDecl(NSD);

    //DeclContext* DC = NSD->getPrimaryContext();
    DeclContext* DC = NSD->getDeclContext();
    Scope* S = m_Sema->getScopeForContext(DC);

    // Find other decls that the old one has replaced
    StoredDeclsMap *Map = DC->getPrimaryContext()->getLookupPtr();
    if (!Map) 
      return false;      
    StoredDeclsMap::iterator Pos = Map->find(NSD->getDeclName());
    assert(Pos != Map->end() && "no lookup entry for decl");
    
    if (Pos->second.isNull())
      if (NSD != NSD->getOriginalNamespace()) {
        NamespaceDecl* NewNSD = NSD->getOriginalNamespace();
        Pos->second.setOnlyValue(NewNSD);
        if (S)
          S->AddDecl(NewNSD);
        m_Sema->IdResolver.AddDecl(NewNSD);
      }

    return Successful;
  }

  // See Sema::PushOnScopeChains
  bool DeclReverter::isOnScopeChains(NamedDecl* ND) {
    
    // Named decls without name shouldn't be in. Eg: struct {int a};
    if (!ND->getDeclName())
      return false;

    // Out-of-line definitions shouldn't be pushed into scope in C++.
    // Out-of-line variable and function definitions shouldn't even in C.
    if ((isa<VarDecl>(ND) || isa<FunctionDecl>(ND)) && ND->isOutOfLine() && 
        !ND->getDeclContext()->getRedeclContext()->Equals(
                        ND->getLexicalDeclContext()->getRedeclContext()))
      return false;

    // Template instantiations should also not be pushed into scope.
    if (isa<FunctionDecl>(ND) &&
        cast<FunctionDecl>(ND)->isFunctionTemplateSpecialization())
      return false;

    IdentifierResolver::iterator 
      IDRi = m_Sema->IdResolver.begin(ND->getDeclName()),
      IDRiEnd = m_Sema->IdResolver.end();
    
    for (; IDRi != IDRiEnd; ++IDRi) {
      if (ND == *IDRi) 
        return true;
    }


    // Check if the declaration is template instantiation, which is not in
    // any DeclContext yet, because it came from 
    // Sema::PerformPendingInstantiations
    // if (isa<FunctionDecl>(D) && 
    //     cast<FunctionDecl>(D)->getTemplateInstantiationPattern())
    //   return false;ye


    return false;
  }

  ASTNodeEraser::ASTNodeEraser(Sema* S) : m_Sema(S) {
    m_DeclReverter = new DeclReverter(S);
  }

  ASTNodeEraser::~ASTNodeEraser() {
    delete m_DeclReverter;
    m_DeclReverter = 0;
  }

  bool ASTNodeEraser::RevertDecl(Decl* D) {
    return m_DeclReverter->Visit(D);
  }

} // end namespace cling
