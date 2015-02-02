// @(#)root/io:$Id$
// Author: Andreas Peters + Fons Rademakers   26/5/2005

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TFileMerger
#define ROOT_TFileMerger

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TFileMerger                                                          //
//                                                                      //
// This class provides file copy and merging services.                  //
//                                                                      //
// It can be used to copy files (not only ROOT files), using TFile or   //
// any of its remote file access plugins. It is therefore usefull in    //
// a Grid environment where the files might be accessable via Castor,   //
// rfio, dcap, etc.                                                     //
// The merging interface allows files containing histograms and trees   //
// to be merged, like the standalone hadd program.                      //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif
#ifndef ROOT_TString
#include "TString.h"
#endif
#ifndef ROOT_TStopwatch
#include "TStopwatch.h"
#endif

class TList;
class TFile;
class TDirectory;


class TFileMerger : public TObject {
private:
   TFileMerger(const TFileMerger&); // Not implemented
   TFileMerger& operator=(const TFileMerger&); // Not implemented

protected:
   TStopwatch     fWatch;            // stop watch to measure file copy speed
   TList         *fFileList;         // a list the file (TFile*) which shall be merged
   TFile         *fOutputFile;       // the outputfile for merging
   TString        fOutputFilename;   // the name of the outputfile for merging
   Bool_t         fFastMethod;       // True if using Fast merging algorithm (default)
   Bool_t         fNoTrees;          // True if Trees should not be merged (default is kFALSE)
   Bool_t         fExplicitCompLevel;// True if the user explicitly requested a compressio level change (default kFALSE)
   Bool_t         fCompressionChange;// True if the output and input have different compression level (default kFALSE)
   Int_t          fPrintLevel;       // How much information to print out at run time.
   TString        fMsgPrefix;        // Prefix to be used when printing informational message (default TFileMerger)

   Int_t          fMaxOpenedFiles;  // Maximum number of files opened at the same time by the TFileMerger.
   Bool_t         fLocal;           // Makes local copies of merging files if True (default is kTRUE)
   Bool_t         fHistoOneGo;      // Merger histos in one go (default is kTRUE)
   TString        fObjectNames;     // List of object names to be either merged exclusively or skipped
   TList         *fMergeList;       // list of TObjString containing the name of the files need to be merged
   TList         *fExcessFiles;     //! List of TObjString containing the name of the files not yet added to fFileList due to user or system limitiation on the max number of files opened.

   Bool_t         OpenExcessFiles();
   virtual Bool_t AddFile(TFile *source, Bool_t own, Bool_t cpProgress);
   virtual Bool_t MergeRecursive(TDirectory *target, TList *sourcelist, Int_t type = kRegular | kAll);

public:
   enum EPartialMergeType {
      kRegular      = 0,             // Normal merge, overwritting the output file.
      kIncremental  = BIT(1),        // Merge the input file with the content of the output file (if already exising).
      kResetable    = BIT(2),        // Only the objects with a MergeAfterReset member function.
      kNonResetable = BIT(3),        // Only the objects without a MergeAfterReset member function.

      kAll            = BIT(2)|BIT(3),       // Merge all type of objects (default)
      kAllIncremental = kIncremental | kAll, // Merge incrementally all type of objects.
      
      kOnlyListed   = BIT(4),        // Merge only the objects specified in fObjectNames list
      kSkipListed   = BIT(5)         // Skip objects specified in fObjectNames list
   };
   TFileMerger(Bool_t isLocal = kTRUE, Bool_t histoOneGo = kTRUE);
   virtual ~TFileMerger();

   Int_t       GetPrintLevel() const { return fPrintLevel; }
   void        SetPrintLevel(Int_t level) { fPrintLevel = level; }
   Bool_t      HasCompressionChange() const { return fCompressionChange; }
   const char *GetOutputFileName() const { return fOutputFilename; }
   TList      *GetMergeList() const { return fMergeList;  }
   TFile      *GetOutputFile() const { return fOutputFile; }
   Int_t       GetMaxOpenedFies() const { return fMaxOpenedFiles; }
   void        SetMaxOpenedFiles(Int_t newmax);
   const char *GetMsgPrefix() const { return fMsgPrefix; }
   void        SetMsgPrefix(const char *prefix);
   void        AddObjectNames(const char *name) {fObjectNames += name; fObjectNames += " ";}
   const char *GetObjectNames() const {return fObjectNames.Data();}
   void        ClearObjectNames() {fObjectNames.Clear();}

    //--- file management interface
   virtual Bool_t SetCWD(const char * /*path*/) { MayNotUse("SetCWD"); return kFALSE; }
   virtual const char *GetCWD() { MayNotUse("GetCWD"); return 0; }

   //--- file merging interface
   virtual void   Reset();
   virtual Bool_t AddFile(const char *url, Bool_t cpProgress = kTRUE);
   virtual Bool_t AddFile(TFile *source, Bool_t cpProgress = kTRUE);
   virtual Bool_t AddAdoptFile(TFile *source, Bool_t cpProgress = kTRUE);
   virtual Bool_t OutputFile(const char *url, Bool_t force);
   virtual Bool_t OutputFile(const char *url, Bool_t force, Int_t compressionLevel);
   virtual Bool_t OutputFile(const char *url, const char *mode = "RECREATE");
   virtual Bool_t OutputFile(const char *url, const char *mode, Int_t compressionLevel);
   virtual void   PrintFiles(Option_t *options);
   virtual Bool_t Merge(Bool_t = kTRUE);
   virtual Bool_t PartialMerge(Int_t type = kAll | kIncremental);
   virtual void   SetFastMethod(Bool_t fast=kTRUE)  {fFastMethod = fast;}
   virtual void   SetNotrees(Bool_t notrees=kFALSE) {fNoTrees = notrees;}
   virtual void        RecursiveRemove(TObject *obj);

   ClassDef(TFileMerger,5)  // File copying and merging services
};

#endif

