#include <stdexcept>
#include <cstring>
#include <memory>

#include "TMultiGraph.h"
#include "TGraphPolar.h"
#include "TSystem.h"
#include "IOSPad.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include "TH1.h"
#include "TF2.h"

#include "FileUtils.h"

namespace ROOT {
namespace iOS {
namespace Browser {

namespace {

//__________________________________________________________________________________________________________________________
void FillVisibleTypes(std::set<TString> &types)
{
   types.insert("TH1C");
   types.insert("TH1D");
   types.insert("TH1F");
   types.insert("TH1I");
   types.insert("TH1K");
   types.insert("TH1S");
   types.insert("TH2C");
   types.insert("TH2D");
   types.insert("TH2F");
   types.insert("TH2I");
   types.insert("TH2Poly");
   types.insert("TH2S");
   types.insert("TH3C");
   types.insert("TH3D");
   types.insert("TH3F");
   types.insert("TH3I");
   types.insert("TH3S");
   types.insert("TF2");
   types.insert("TGraphPolar");
   types.insert("TMultiGraph");
}

const char *errorOptionToString[] = {"", "E", "E1", "E2", "E3", "E4"};

//__________________________________________________________________________________________________________________________
void RemoveErrorDrawOption(TString &options)
{
   const Ssiz_t pos = options.Index("E");
   if (pos != kNPOS) {

      Ssiz_t n = 1;
      if (pos + 1 < options.Length()) {
         const char nextChar = options[pos + 1];
         if (std::isdigit(nextChar) && (nextChar - '0' >= 1 && nextChar - '0' <= 4))
            n = 2;
      }
   
      options.Remove(pos, n);
   }
}

//__________________________________________________________________________________________________________________________
void RemoveMarkerDrawOption(TString &options)
{
   const Ssiz_t pos = options.Index("P");
   if (pos != kNPOS)
      options.Remove(pos, 1);
}

//__________________________________________________________________________________________________________________________
TObject *ReadObjectForKey(TDirectoryFile *inputFile, const TKey *key, TString &option)
{
   option = "";

   TObject *objPtr = inputFile->Get(key->GetName());
   if (!objPtr)
      throw std::runtime_error("bad key in ReadObjectForKey");
   //Objects of some types are onwed by the file. So I have to make
   //them free from such ownership to make
   //their processing later more uniform.
   if (TH1 *hist = dynamic_cast<TH1 *>(objPtr))
      hist->SetDirectory(0);

   //The code below can throw, so I use auto_ptr.
   std::auto_ptr<TObject> obj(objPtr);

   //This is the trick, since ROOT seems not to preserve
   //Draw's option in a file.
   if (dynamic_cast<TF2 *>(obj.get()))
      option = "surf1";
   if (dynamic_cast<TMultiGraph *>(obj.get()))
      option = "acp";
   
   //All this "home-made memory management" is an ugly and broken thing.
   obj->SetBit(kCanDelete, kFALSE);
   obj->SetBit(kMustCleanup, kFALSE);

   return obj.release();
}

}//Unnamed namespace

//__________________________________________________________________________________________________________________________
FileContainer::FileContainer(const std::string &fileName)
                  : fFileName(fileName)
{
}


//__________________________________________________________________________________________________________________________
FileContainer::~FileContainer()
{
   for (auto dir : fDirectories)
      delete dir;
   for (auto fileContainer : fObjects)
      delete fileContainer;
   for (auto pad : fAttachedPads)
      delete pad;
}

//__________________________________________________________________________________________________________________________
auto FileContainer::GetNumberOfObjects()const -> size_type
{
   return fObjects.size();
}

//__________________________________________________________________________________________________________________________
TObject *FileContainer::GetObject(size_type ind)const
{
   return fObjects[ind];
}

//__________________________________________________________________________________________________________________________
const char *FileContainer::GetDrawOption(size_type ind)const
{
   return fOptions[ind].Data();
}

//__________________________________________________________________________________________________________________________
Pad *FileContainer::GetPadAttached(size_type ind)const
{
   return fAttachedPads[ind];
}

//__________________________________________________________________________________________________________________________
void FileContainer::SetErrorDrawOption(size_type ind, EHistogramErrorOption opt)
{
   //Nothing to change.
   if (GetErrorDrawOption(ind) == opt)
      return;

   //1. Remove previous error options (if any).
   RemoveErrorDrawOption(fOptions[ind]);
   //2. Add new option.
   fOptions[ind] += errorOptionToString[opt];
}

//__________________________________________________________________________________________________________________________
EHistogramErrorOption FileContainer::GetErrorDrawOption(size_type ind)const
{
   const TString &options = fOptions[ind];
   const Ssiz_t pos = options.Index("E");
   if (pos == kNPOS)
      return hetNoError;
   
   if (pos + 1 < options.Length()) {
      const char nextChar = options[pos + 1];
      if (nextChar == '1')
         return hetE1;
      if (nextChar == '2')
         return hetE2;
      if (nextChar == '3')
         return hetE3;
      if (nextChar == '4')
         return hetE4;
   }
   
   return hetE;
}

//__________________________________________________________________________________________________________________________
void FileContainer::SetMarkerDrawOption(size_type ind, bool on)
{
   if (GetMarkerDrawOption(ind) == on)
      return;

   RemoveMarkerDrawOption(fOptions[ind]);

   if (on)
      fOptions[ind] += "P";
}

//__________________________________________________________________________________________________________________________
bool FileContainer::GetMarkerDrawOption(size_type ind)const
{
   return fOptions[ind].Index("P") != kNPOS;
}

//__________________________________________________________________________________________________________________________
auto FileContainer::GetNumberOfDirectories()const -> size_type
{
   return fDirectories.size();
}

//__________________________________________________________________________________________________________________________
FileContainer *FileContainer::GetDirectory(size_type ind)const
{
   return fDirectories[ind];
}

//__________________________________________________________________________________________________________________________
const char *FileContainer::GetFileName()const
{
   return fFileName.c_str();
}

//__________________________________________________________________________________________________________________________
void FileContainer::AttachPads()
{
   if (!fObjects.size())
      return;

   fAttachedPads.assign(fObjects.size(), 0);
   for (size_type i = 0; i < fAttachedPads.size(); ++i) {
      std::auto_ptr<Pad> newPad(new Pad(400, 400));//400 - size is NOT important here, it'll be reset later anyway.
      newPad->cd();
      fObjects[i]->Draw(fOptions[i].Data());
      fAttachedPads[i] = newPad.release();
   }
}

//__________________________________________________________________________________________________________________________
auto FileContainer::GetNumberOfDescriptors()const -> size_type
{
   return fContentDescriptors.size();
}

//__________________________________________________________________________________________________________________________
auto FileContainer::GetElementDescriptor(size_type index)const -> const FileContainerElement &
{
   return fContentDescriptors[index];
}

//__________________________________________________________________________________________________________________________
void FileContainer::ScanDirectory(TDirectoryFile *dir, const std::set<TString> &visibleTypes, FileContainer *currentContainer)
{
   const TList *objKeys = dir->GetListOfKeys();
   if (!objKeys)
      return;
   
   TString option;
   std::vector<TObject *> objs;
   std::vector<FileContainer *> dirs;
   std::vector<TString> opts;
   std::vector<FileContainerElement> descriptors;
   
   TObjLink *link = objKeys->FirstLink();
   
   try {
      while (link) {
         const TKey *key = static_cast<TKey *>(link->GetObject());
         const TString className(key->GetClassName());
            
         if (className == "TDirectoryFile") {
            std::auto_ptr<TDirectoryFile> nestedDir(static_cast<TDirectoryFile *>(dir->Get(key->GetName())));
            if (nestedDir.get()) {
               //Recursion - create nested container.
               std::auto_ptr<FileContainer> nestedContainer(new FileContainer(key->GetName()));
               ScanDirectory(nestedDir.get(), visibleTypes, nestedContainer.get());
               nestedContainer->AttachPads();
               
               FileContainerElement newDescriptor(key->GetName(), currentContainer, true, dirs.size());
               descriptors.push_back(newDescriptor);
               descriptors.insert(descriptors.end(), nestedContainer->fContentDescriptors.begin(), nestedContainer->fContentDescriptors.end());

               dirs.push_back(nestedContainer.get());
               nestedContainer.release();
            }
         } else if (visibleTypes.find(className) != visibleTypes.end()) {
            std::auto_ptr<TObject> newObject(ReadObjectForKey(dir, key, option));
            opts.push_back(option);

            FileContainerElement newDescriptor(key->GetName(), currentContainer, false, objs.size());
            descriptors.push_back(newDescriptor);

            objs.push_back(newObject.get());//bad_alloc.
            newObject.release();
         }
      
         link = link->Next();
      }
   } catch (const std::exception &) {
      for (auto obj : objs)
         delete obj;
      for (auto dir : dirs)
         delete dir;
      throw;
   }
   
   currentContainer->fObjects.swap(objs);
   currentContainer->fDirectories.swap(dirs);
   currentContainer->fOptions.swap(opts);
   currentContainer->fContentDescriptors.swap(descriptors);
}

//__________________________________________________________________________________________________________________________
FileContainer *FileContainer::CreateFileContainer(const char *fullPath)
{
   try {
      std::set<TString> visibleTypes;
      FillVisibleTypes(visibleTypes);

      std::auto_ptr<TFile> inputFile(TFile::Open(fullPath, "read"));
      if (!inputFile.get())
         return 0;
      
      const std::string fileName(gSystem->BaseName(fullPath));
      std::auto_ptr<FileContainer> topLevel(new FileContainer(fileName));
      
      ScanDirectory(inputFile.get(), visibleTypes, topLevel.get());
      
      //
      //Attach pads.
      topLevel->AttachPads();
      //
      return topLevel.release();
   } catch (const std::exception &) {
      return 0;
   }
}

//__________________________________________________________________________________________________________________________
void FileContainer::DeleteFileContainer(FileContainer *container)
{
   delete container;
}

}//namespace Browser
}//namespace iOS
}//namespace ROOT
