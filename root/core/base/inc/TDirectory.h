// @(#)root/base:$Id$
// Author: Rene Brun   28/11/94

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDirectory
#define ROOT_TDirectory


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDirectory                                                           //
//                                                                      //
// Describe directory structure in memory.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif
#ifndef ROOT_TList
#include "TList.h"
#endif
#ifndef ROOT_TDatime
#include "TDatime.h"
#endif
#ifndef ROOT_TUUID
#include "TUUID.h"
#endif

class TBrowser;
class TKey;
class TFile;

class TDirectory : public TNamed {
public:
   /** @class Context
     *
     *  Small helper to keep current directory context.
     *  Automatically reverts to "old" directory
     */
   class TContext  {
   private:
      TDirectory *fDirectory;   //! Pointer to the previous current directory.
      TContext   *fPrevious;    //! Pointer to the next TContext in the implied list of context pointing to fPrevious.
      TContext   *fNext;        //! Pointer to the next TContext in the implied list of context pointing to fPrevious.
      TContext(TContext&);
      TContext& operator=(TContext&);
      void CdNull();
      friend class TDirectory;
   public:
      TContext(TDirectory* previous, TDirectory* newCurrent)
         : fDirectory(previous),fPrevious(0),fNext(0)
      {
         // Store the current directory so we can restore it
         // later and cd to the new directory.
         if ( fDirectory ) fDirectory->RegisterContext(this);
         if ( newCurrent ) newCurrent->cd();
      }
      TContext(TDirectory* newCurrent) : fDirectory(TDirectory::CurrentDirectory()),fPrevious(0),fNext(0)
      {
         // Store the current directory so we can restore it
         // later and cd to the new directory.
         if ( fDirectory ) fDirectory->RegisterContext(this);
         if ( newCurrent ) newCurrent->cd();
      }
      ~TContext()
      {
         // Destructor.   Reset the current directory to its
         // previous state.
         if ( fDirectory ) {
            fDirectory->UnregisterContext(this);
            fDirectory->cd();
         }
         else CdNull();
      }
   };

protected:

   TObject      *fMother;          //pointer to mother of the directory
   TList        *fList;            //List of objects in memory
   TUUID         fUUID;            //Unique identifier
   TString       fPathBuffer;      //!Buffer for GetPath() function
   TContext     *fContext;         //!Pointer to a list of TContext object pointing to this TDirectory
   static Bool_t fgAddDirectory;   //!flag to add histograms, graphs,etc to the directory

          Bool_t  cd1(const char *path);
   static Bool_t  Cd1(const char *path);

   virtual void   CleanTargets();
           void   FillFullPath(TString& buf) const;
           void   RegisterContext(TContext *ctxt);
           void   UnregisterContext(TContext *ctxt);
   friend class TContext;

protected:
   TDirectory(const TDirectory &directory);  //Directories cannot be copied
   void operator=(const TDirectory &); //Directorise cannot be copied

public:

   TDirectory();
   TDirectory(const char *name, const char *title, Option_t *option="", TDirectory* motherDir = 0);
   virtual ~TDirectory();
   static  void        AddDirectory(Bool_t add=kTRUE);
   static  Bool_t      AddDirectoryStatus();
   virtual void        Append(TObject *obj, Bool_t replace = kFALSE);
   virtual void        Add(TObject *obj, Bool_t replace = kFALSE) { Append(obj,replace); }
   virtual Int_t       AppendKey(TKey *) {return 0;}
   virtual void        Browse(TBrowser *b);
   virtual void        Build(TFile* motherFile = 0, TDirectory* motherDir = 0);
   virtual void        Clear(Option_t *option="");
   virtual TObject    *CloneObject(const TObject *obj, Bool_t autoadd = kTRUE);
   virtual void        Close(Option_t *option="");
   static TDirectory *&CurrentDirectory();  // Return the current directory for this thread.   
   virtual void        Copy(TObject &) const { MayNotUse("Copy(TObject &)"); }
   virtual Bool_t      cd(const char *path = 0);
   virtual void        DeleteAll(Option_t *option="");
   virtual void        Delete(const char *namecycle="");
   virtual void        Draw(Option_t *option="");
   virtual TKey       *FindKey(const char * /*keyname*/) const {return 0;}
   virtual TKey       *FindKeyAny(const char * /*keyname*/) const {return 0;}
   virtual TObject    *FindObject(const char *name) const;
   virtual TObject    *FindObject(const TObject *obj) const;
   virtual TObject    *FindObjectAny(const char *name) const;
   virtual TObject    *FindObjectAnyFile(const char * /*name*/) const {return 0;}
   virtual TObject    *Get(const char *namecycle);
   virtual TDirectory *GetDirectory(const char *namecycle, Bool_t printError = false, const char *funcname = "GetDirectory");
   template <class T> inline void GetObject(const char* namecycle, T*& ptr) // See TDirectory::Get for information
      {
         ptr = (T*)GetObjectChecked(namecycle,TBuffer::GetClass(typeid(T)));
      }
   virtual void       *GetObjectChecked(const char *namecycle, const char* classname);
   virtual void       *GetObjectChecked(const char *namecycle, const TClass* cl);
   virtual void       *GetObjectUnchecked(const char *namecycle);
   virtual Int_t       GetBufferSize() const {return 0;}
   virtual TFile      *GetFile() const { return 0; }
   virtual TKey       *GetKey(const char * /*name */, Short_t /* cycle */=9999) const {return 0;}
   virtual TList      *GetList() const { return fList; }
   virtual TList      *GetListOfKeys() const { return 0; }
   virtual TObject    *GetMother() const { return fMother; }
   virtual TDirectory *GetMotherDir() const { return fMother==0 ? 0 : dynamic_cast<TDirectory*>(fMother); }
   virtual Int_t       GetNbytesKeys() const { return 0; }
   virtual Int_t       GetNkeys() const { return 0; }
   virtual Long64_t    GetSeekDir() const { return 0; }
   virtual Long64_t    GetSeekParent() const { return 0; }
   virtual Long64_t    GetSeekKeys() const { return 0; }
   virtual const char *GetPathStatic() const;
   virtual const char *GetPath() const;
   TUUID               GetUUID() const {return fUUID;}
   virtual Bool_t      IsFolder() const { return kTRUE; }
   virtual Bool_t      IsModified() const { return kFALSE; }
   virtual Bool_t      IsWritable() const { return kFALSE; }
   virtual void        ls(Option_t *option="") const;
   virtual TDirectory *mkdir(const char *name, const char *title="");
   virtual TFile      *OpenFile(const char * /*name*/, Option_t * /*option*/ = "",
                            const char * /*ftitle*/ = "", Int_t /*compress*/ = 1,
                            Int_t /*netopt*/ = 0) {return 0;}
   virtual void        Paint(Option_t *option="");
   virtual void        Print(Option_t *option="") const;
   virtual void        Purge(Short_t /*nkeep*/=1) {}
   virtual void        pwd() const;
   virtual void        ReadAll(Option_t * /*option*/="") {}
   virtual Int_t       ReadKeys(Bool_t /*forceRead*/=kTRUE) {return 0;}
   virtual Int_t       ReadTObject(TObject * /*obj*/, const char * /*keyname*/) {return 0;}
   virtual TObject    *Remove(TObject*);
   virtual void        RecursiveRemove(TObject *obj);
   virtual void        rmdir(const char *name);
   virtual void        Save() {}
   virtual Int_t       SaveObjectAs(const TObject * /*obj*/, const char * /*filename*/="", Option_t * /*option*/="") const;
   virtual void        SaveSelf(Bool_t /*force*/ = kFALSE) {}
   virtual void        SetBufferSize(Int_t /* bufsize */) {}
   virtual void        SetModified() {}
   virtual void        SetMother(TObject *mother) {fMother = (TObject*)mother;}
   virtual void        SetName(const char* newname);
   virtual void        SetTRefAction(TObject * /*ref*/, TObject * /*parent*/) {}
   virtual void        SetSeekDir(Long64_t) {}
   virtual void        SetWritable(Bool_t) {}
   virtual Int_t       Sizeof() const {return 0;}
   virtual Int_t       Write(const char * /*name*/=0, Int_t /*opt*/=0, Int_t /*bufsize*/=0){return 0;}
   virtual Int_t       Write(const char * /*name*/=0, Int_t /*opt*/=0, Int_t /*bufsize*/=0) const {return 0;}
   virtual Int_t       WriteTObject(const TObject *obj, const char *name =0, Option_t * /*option*/="", Int_t /*bufsize*/ =0);
   template <class T> inline Int_t WriteObject(const T* obj, const char* name, Option_t *option="", Int_t bufsize=0) // see TDirectory::WriteTObject or TDirectoryWriteObjectAny for explanation
      {
         return WriteObjectAny(obj,TBuffer::GetClass(typeid(T)),name,option,bufsize);
      }
   virtual Int_t       WriteObjectAny(const void *, const char * /*classname*/, const char * /*name*/, Option_t * /*option*/="", Int_t /*bufsize*/ =0) {return 0;}
   virtual Int_t       WriteObjectAny(const void *, const TClass * /*cl*/, const char * /*name*/, Option_t * /*option*/="", Int_t /*bufsize*/ =0) {return 0;}
   virtual void        WriteDirHeader() {}
   virtual void        WriteKeys() {}

   static Bool_t       Cd(const char *path);
   static void         DecodeNameCycle(const char *namecycle, char *name, Short_t &cycle, const size_t namesize = 0);
   static void         EncodeNameCycle(char *buffer, const char *name, Short_t cycle);

   ClassDef(TDirectory,5)  //Describe directory structure in memory
};

#ifndef __CINT__
#define gDirectory (TDirectory::CurrentDirectory())

#elif defined(__MAKECINT__)
// To properly handle the use of gDirectory in header files (in static declarations)
R__EXTERN TDirectory *gDirectory;
#endif

#endif
