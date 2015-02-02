// @(#)root/base:$Id: a94734bd8505d0a8f1833346da666b196a797388 $
// Author: Fons Rademakers   17/01/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TUrl                                                                 //
//                                                                      //
// This class represents a WWW compatible URL.                          //
// It provides member functions to return the different parts of        //
// an URL. The supported url format is:                                 //
//  [proto://][user[:passwd]@]host[:port]/file.ext[#anchor][?options]   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "TUrl.h"
#include "THashList.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TEnv.h"
#include "TSystem.h"
#include "TMap.h"
#include "TVirtualMutex.h"

TObjArray *TUrl::fgSpecialProtocols = 0;
THashList *TUrl::fgHostFQDNs = 0;

TVirtualMutex *gURLMutex = 0; // local mutex

#ifdef R__COMPLETE_MEM_TERMINATION
namespace {
   class TUrlCleanup {
      TObjArray **fSpecialProtocols;
      THashList **fHostFQDNs;
   public:
      TUrlCleanup(TObjArray **protocols, THashList **hosts) : fSpecialProtocols(protocols),fHostFQDNs(hosts) {}
      ~TUrlCleanup() {
         if (*fSpecialProtocols) (*fSpecialProtocols)->Delete();
         delete *fSpecialProtocols;
         *fSpecialProtocols = 0;
         if (*fHostFQDNs) (*fHostFQDNs)->Delete();
         delete *fHostFQDNs;
         *fHostFQDNs = 0;
      }
   };
}
#endif

ClassImp(TUrl)

//______________________________________________________________________________
TUrl::TUrl(const char *url, Bool_t defaultIsFile)
{
   // Parse url character string and split in its different subcomponents.
   // Use IsValid() to check if URL is legal.
   //
   // url: [proto://][user[:passwd]@]host[:port]/file.ext[?options][#anchor]
   //
   // Known protocols: http, root, proof, ftp, news and any special protocols
   // defined in the rootrc Url.Special key.
   // The default protocol is "http", unless defaultIsFile is true in which
   // case the url is assumed to be of type "file".
   // If a passwd contains a @ it must be escaped by a \\, e.g.
   // "pip@" becomes "pip\\@".
   //
   // Default ports: http=80, root=1094, proof=1093, ftp=20, news=119.
   // Port #1093 has been assigned by IANA (www.iana.org) to proofd.
   // Port #1094 has been assigned by IANA (www.iana.org) to rootd.

   SetUrl(url, defaultIsFile);

#ifdef R__COMPLETE_MEM_TERMINATION
   static TUrlCleanup cleanup(&fgSpecialProtocols,&fgHostFQDNs);
#endif
}

//______________________________________________________________________________
TUrl::~TUrl()
{
   // Cleanup.

   delete fOptionsMap;
}

//______________________________________________________________________________
void TUrl::SetUrl(const char *url, Bool_t defaultIsFile)
{
   // Parse url character string and split in its different subcomponents.
   // Use IsValid() to check if URL is legal.
   //
   // url: [proto://][user[:passwd]@]host[:port]/file.ext[?options][#anchor]
   //
   // Known protocols: http, root, proof, ftp, news and any special protocols
   // defined in the rootrc Url.Special key.
   // The default protocol is "http", unless defaultIsFile is true in which
   // case the url is assumed to be of type "file".
   // If a passwd contains a @ it must be escaped by a \\, e.g.
   // "pip@" becomes "pip\\@".
   //
   // Default ports: http=80, root=1094, proof=1093, ftp=20, news=119.
   // Port #1093 has been assigned by IANA (www.iana.org) to proofd.
   // Port #1094 has been assigned by IANA (www.iana.org) to rootd.

   fOptionsMap = 0;

   if (!url || !url[0]) {
      fPort = -1;
      return;
   }

   // Set defaults
   fUrl        = "";
   fProtocol   = "http";
   fUser       = "";
   fPasswd     = "";
   fHost       = "";
   fPort       = 80;
   fFile       = "";
   fAnchor     = "";
   fOptions    = "";
   fFileOA     = "";
   fHostFQ     = "";

   // if url starts with a / consider it as a file url
   if (url[0] == '/')
      defaultIsFile = kTRUE;

   // Find protocol
   char *s, sav;

   char *u, *u0 = Strip(url);
tryfile:
   u = u0;

   // Handle special protocol cases: "file:", "rfio:", etc.
   for (int i = 0; i < GetSpecialProtocols()->GetEntriesFast(); i++) {
      TObjString *os = (TObjString*) GetSpecialProtocols()->UncheckedAt(i);
      TString s1 = os->GetString();
      int l = s1.Length();
      Bool_t stripoff = kFALSE;
      if (s1.EndsWith("/-")) {
         stripoff = kTRUE;
         s1 = s1.Strip(TString::kTrailing, '-');
         l--;
      }
      if (!strncmp(u, s1, l)) {
         if (s1(0) == '/' && s1(l-1) == '/') {
            // case whith file namespace like: /alien/user/file.root
            fProtocol = s1(1, l-2);
            if (stripoff)
               l--;    // strip off namespace prefix from file name
            else
               l = 0;  // leave namespace prefix as part of file name
         } else {
            // case with protocol, like: rfio:machine:/data/file.root
            fProtocol = s1(0, l-1);
         }
         if (!strncmp(u+l, "//", 2))
            u += l+2;
         else
            u += l;
         fPort = 0;

         FindFile(u, kFALSE);

         delete [] u0;
         return;
      }
   }

   u = u0;

   char *x, *t, *s2;
   // allow x:/path as Windows filename
   if ((s = strstr(u, ":/")) && u+1 != s) {
      if (*(s+2) != '/') {
         Error("TUrl", "%s malformed, URL must contain \"://\"", u0);
         fPort = -1;
         goto cleanup;
      }
      sav = *s;
      *s = 0;
      SetProtocol(u, kTRUE);
      *s = sav;
      s += 3;
      // allow url of form: "proto://"
   } else {
      if (defaultIsFile) {
         char *newu = new char [strlen("file:") + strlen(u0) + 1];
         sprintf(newu, "file:%s", u0);
         delete [] u0;
         u0 = newu;
         goto tryfile;
      }
      s = u;
   }

   // Find user and passwd
   u = s;
   t = s;
again:
   if ((s = strchr(t, '@')) && (
       ((x = strchr(t, '/')) && s < x) ||
       ((x = strchr(t, '?')) && s < x) ||
       ((x = strchr(t, '#')) && s < x) ||
       (!strchr(t, '/'))
      )) {
      if (*(s-1) == '\\') {
         t = s+1;
         goto again;
      }
      sav = *s;
      *s = 0;
      if ((s2 = strchr(u, ':'))) {
         *s2 = 0;
         fUser = u;
         *s2 = ':';
         s2++;
         if (*s2) {
            fPasswd = s2;
            fPasswd.ReplaceAll("\\@", "@");
         }
      } else
         fUser = u;
      *s = sav;
      s++;
   } else
      s = u;

   // Find host
   u = s;
   if ((s = strchr(u, ':')) || (s = strchr(u, '/')) || (s = strchr(u, '?')) || (s = strchr(u, '#'))) {
      if ((strchr (u, ':') > strchr(u, '/')) && (strchr (u, '/')))
         s = strchr(u, '/');
      sav = *s;
      *s = 0;
      fHost = u;
      *s = sav;
      if (sav == ':') {
         s++;
         // Get port #
         if (!*s) {
            fPort = -1;
            goto cleanup;
         }
         u = s;
         if ((s = strchr(u, '/')) || (s = strchr(u, '?')) || (s = strchr(u, '#'))) {
            sav = *s;
            *s = 0;
            fPort = atoi(u);
            *s = sav;
         } else {
            fPort = atoi(u);
            goto cleanup;
         }
      }
   } else {
      fHost = u;
      goto cleanup;
   }

   if (!*s) goto cleanup;

   // Find file
   u = s;
   if (*u == '/' && fHost.Length())
      u++;

   FindFile(u);

cleanup:
   delete [] u0;
}

//______________________________________________________________________________
void TUrl::FindFile(char *u, Bool_t stripDoubleSlash)
{
   // Find file and optionally anchor and options.

   char *s, sav;

   // Locate anchor and options, if any
   char *opt = strchr(u, '?');
   char *anc = strchr(u, '#');

   // URL invalid if anchor is coming before the options
   if (opt && anc && opt > anc) {
      fPort = -1;
      return;
   }

   if ((s = opt) || (s = anc)) {
      sav = *s;
      *s = 0;
      fFile = u;
      if (stripDoubleSlash)
         fFile.ReplaceAll("//", "/");
      *s = sav;
      s++;
      if (sav == '?') {
         // Get options
         if (!*s) {
            // options string is empty
            return;
         }
         u = s;
         if ((s = strchr(u, '#'))) {
            sav = *s;
            *s = 0;
            fOptions = u;
            *s = sav;
            s++;
         } else {
            fOptions = u;
            return;
         }
      }
      if (!*s) {
         // anchor string is empty
         return;
      }
   } else {
      fFile = u;
      if (stripDoubleSlash)
         fFile.ReplaceAll("//", "/");
      return;
   }

   // Set anchor
   fAnchor = s;
}

//______________________________________________________________________________
TUrl::TUrl(const TUrl &url) : TObject(url)
{
   // TUrl copy ctor.

   fUrl        = url.fUrl;
   fProtocol   = url.fProtocol;
   fUser       = url.fUser;
   fPasswd     = url.fPasswd;
   fHost       = url.fHost;
   fFile       = url.fFile;
   fAnchor     = url.fAnchor;
   fOptions    = url.fOptions;
   fPort       = url.fPort;
   fFileOA     = url.fFileOA;
   fHostFQ     = url.fHostFQ;
   fOptionsMap = 0;
}

//______________________________________________________________________________
TUrl &TUrl::operator=(const TUrl &rhs)
{
   // TUrl assignment operator.

   if (this != &rhs) {
      TObject::operator=(rhs);
      fUrl        = rhs.fUrl;
      fProtocol   = rhs.fProtocol;
      fUser       = rhs.fUser;
      fPasswd     = rhs.fPasswd;
      fHost       = rhs.fHost;
      fFile       = rhs.fFile;
      fAnchor     = rhs.fAnchor;
      fOptions    = rhs.fOptions;
      fPort       = rhs.fPort;
      fFileOA     = rhs.fFileOA;
      fHostFQ     = rhs.fHostFQ;
      fOptionsMap = 0;
   }
   return *this;
}

//______________________________________________________________________________
const char *TUrl::GetUrl(Bool_t withDeflt) const
{
   // Return full URL. If withDflt is kTRUE, explicitly add the port even
   // if it matches the default value for the URL protocol.

   if (((TestBit(kUrlWithDefaultPort) && !withDeflt) ||
       (!TestBit(kUrlWithDefaultPort) && withDeflt)) &&
       TestBit(kUrlHasDefaultPort))
      fUrl = "";

   if (IsValid() && fUrl == "") {
      // Handle special protocol cases: file:, rfio:, etc.
      for (int i = 0; i < GetSpecialProtocols()->GetEntriesFast(); i++) {
         TObjString *os = (TObjString*) GetSpecialProtocols()->UncheckedAt(i);
         TString &s = os->String();
         int l = s.Length();
         if (fProtocol == s(0, l-1)) {
            if (fFile[0] == '/')
               fUrl = fProtocol + "://" + fFile;
            else
               fUrl = fProtocol + ":" + fFile;
            if (fOptions != "") {
               fUrl += "?";
               fUrl += fOptions;
            }
            if (fAnchor != "") {
               fUrl += "#";
               fUrl += fAnchor;
            }
            return fUrl;
         }
      }

      Bool_t deflt = kFALSE;
      if ((!fProtocol.CompareTo("http")  && fPort == 80)   ||
          (fProtocol.BeginsWith("proof") && fPort == 1093) ||
          (fProtocol.BeginsWith("root")  && fPort == 1094) ||
          (!fProtocol.CompareTo("ftp")   && fPort == 20)   ||
          (!fProtocol.CompareTo("news")  && fPort == 119)  ||
          (!fProtocol.CompareTo("https") && fPort == 443)  ||
          fPort == 0) {
         deflt = kTRUE;
         ((TUrl *)this)->SetBit(kUrlHasDefaultPort);
      }

      fUrl = fProtocol + "://";
      if (fUser != "") {
         fUrl += fUser;
         if (fPasswd != "") {
            fUrl += ":";
            TString passwd = fPasswd;
            passwd.ReplaceAll("@", "\\@");
            fUrl += passwd;
         }
         fUrl += "@";
      }
      if (withDeflt)
         ((TUrl*)this)->SetBit(kUrlWithDefaultPort);
      else
         ((TUrl*)this)->ResetBit(kUrlWithDefaultPort);

      if (!deflt || withDeflt) {
         char p[10];
         sprintf(p, "%d", fPort);
         fUrl = fUrl + fHost + ":" + p + "/" + fFile;
      } else
         fUrl = fUrl + fHost + "/" + fFile;
      if (fOptions != "") {
         fUrl += "?";
         fUrl += fOptions;
      }
      if (fAnchor != "") {
         fUrl += "#";
         fUrl += fAnchor;
      }
   }

   fUrl.ReplaceAll("////", "///");
   return fUrl;
}

//______________________________________________________________________________
const char *TUrl::GetHostFQDN() const
{
   // Return fully qualified domain name of url host. If host cannot be
   // resolved or not valid return the host name as originally specified.

   if (fHostFQ == "") {
      // Check if we already resolved it
      TNamed *fqdn = fgHostFQDNs ? (TNamed *) fgHostFQDNs->FindObject(fHost) : 0;
      if (!fqdn) {
         TInetAddress adr(gSystem->GetHostByName(fHost));
         if (adr.IsValid()) {
            fHostFQ = adr.GetHostName();
         } else
            fHostFQ = "-";
         R__LOCKGUARD2(gURLMutex);
         if (!fgHostFQDNs) {
            fgHostFQDNs = new THashList;
            fgHostFQDNs->SetOwner();
         }
         if (fgHostFQDNs && !fgHostFQDNs->FindObject(fHost))
            fgHostFQDNs->Add(new TNamed(fHost,fHostFQ));
      } else {
         fHostFQ = fqdn->GetTitle();
      }
   }
   if (fHostFQ == "-")
      return fHost;
   return fHostFQ;
}

//______________________________________________________________________________
const char *TUrl::GetFileAndOptions() const
{
   // Return the file and its options (the string specified behind the ?).
   // Convenience function useful when the option is used to pass
   // authetication/access information for the specified file.

   if (fFileOA == "") {
      fFileOA = fFile;
      if (fOptions != "") {
         fFileOA += "?";
         fFileOA += fOptions;
      }
      if (fAnchor != "") {
         fFileOA += "#";
         fFileOA += fAnchor;
      }
   }
   return fFileOA;
}

//______________________________________________________________________________
void TUrl::SetProtocol(const char *proto, Bool_t setDefaultPort)
{
   // Set protocol and, optionally, change the port accordingly.

   fProtocol = proto;
   if (setDefaultPort) {
      if (!fProtocol.CompareTo("http"))
         fPort = 80;
      else if (!fProtocol.CompareTo("https"))
         fPort = 443;
      else if (fProtocol.BeginsWith("proof"))  // can also be proofs or proofk
         fPort = 1093;
      else if (fProtocol.BeginsWith("root"))   // can also be roots or rootk
         fPort = 1094;
      else if (!fProtocol.CompareTo("ftp"))
         fPort = 20;
      else if (!fProtocol.CompareTo("news"))
         fPort = 119;
      else {
         // generic protocol (no default port)
         fPort = 0;
      }
   }
   fUrl = "";
}

//______________________________________________________________________________
Int_t TUrl::Compare(const TObject *obj) const
{
   // Compare two urls as strings.

   if (this == obj) return 0;
   if (TUrl::Class() != obj->IsA()) return -1;
   return TString(GetUrl()).CompareTo(((TUrl*)obj)->GetUrl(), TString::kExact);
}

//______________________________________________________________________________
void TUrl::Print(Option_t *) const
{
   // Print URL on stdout.

   if (fPort == -1)
      Printf("Illegal URL");

   Printf("%s", GetUrl());
}

//______________________________________________________________________________
TObjArray *TUrl::GetSpecialProtocols()
{
   // Read the list of special protocols from the rootrc files.
   // These protocols will be parsed in a protocol and a file part,
   // no host or other info will be determined. This is typically
   // used for legacy file descriptions like: rfio:host:/path/file.root.

   static Bool_t usedEnv = kFALSE;

   if (!gEnv) {
      R__LOCKGUARD2(gURLMutex);
      if (!fgSpecialProtocols)
         fgSpecialProtocols = new TObjArray;
      if (fgSpecialProtocols->GetEntriesFast() == 0)
         fgSpecialProtocols->Add(new TObjString("file:"));
      return fgSpecialProtocols;
   }

   if (usedEnv)
      return fgSpecialProtocols;

   R__LOCKGUARD2(gURLMutex);
   if (fgSpecialProtocols)
      fgSpecialProtocols->Delete();

   if (!fgSpecialProtocols)
      fgSpecialProtocols = new TObjArray;

   const char *protos = gEnv->GetValue("Url.Special", "file: rfio: hpss: castor: dcache: dcap:");
   usedEnv = kTRUE;

   if (protos) {
      Int_t cnt = 0;
      char *p = StrDup(protos);
      while (1) {
         TObjString *proto = new TObjString(strtok(!cnt ? p : 0, " "));
         if (proto->String().IsNull()) {
            delete proto;
            break;
         }
         fgSpecialProtocols->Add(proto);
         cnt++;
      }
      delete [] p;
   }
   return fgSpecialProtocols;
}


//______________________________________________________________________________
void TUrl::ParseOptions() const
{
   // Parse URL options into a key/value map.

   if (fOptionsMap) return;

   TString urloptions = GetOptions();
   TObjArray *objOptions = urloptions.Tokenize("&");
   for (Int_t n = 0; n < objOptions->GetEntries(); n++) {
      TString loption = ((TObjString *) objOptions->At(n))->GetName();
      TObjArray *objTags = loption.Tokenize("=");
      if (!fOptionsMap) {
         fOptionsMap = new TMap;
         fOptionsMap->SetOwnerKeyValue();
      }
      if (objTags->GetEntries() == 2) {
         TString key = ((TObjString *) objTags->At(0))->GetName();
         TString value = ((TObjString *) objTags->At(1))->GetName();
         fOptionsMap->Add(new TObjString(key), new TObjString(value));
      } else {
         TString key = ((TObjString *) objTags->At(0))->GetName();
         fOptionsMap->Add(new TObjString(key), 0);
      }
      delete objTags;
   }
   delete objOptions;
}


//______________________________________________________________________________
const char *TUrl::GetValueFromOptions(const char *key) const
{
   // Return a value for a given key from the URL options.
   // Returns 0 in case key is not found.

   if (!key) return 0;
   ParseOptions();
   TObject *option = fOptionsMap ? fOptionsMap->GetValue(key) : 0;
   return (option ? ((TObjString*)fOptionsMap->GetValue(key))->GetName(): 0);
}

//______________________________________________________________________________
Int_t TUrl::GetIntValueFromOptions(const char *key) const
{
   // Return a value for a given key from the URL options as an Int_t,
   // a missing key returns -1.

   if (!key) return -1;
   ParseOptions();
   TObject *option = fOptionsMap ? fOptionsMap->GetValue(key) : 0;
   return (option ? (atoi(((TObjString*)fOptionsMap->GetValue(key))->GetName())) : -1);
}

//______________________________________________________________________________
Bool_t TUrl::HasOption(const char *key) const
{
   // Returns true if the given key appears in the URL options list.
   
   if (!key) return kFALSE;
   ParseOptions();
   
   if (fOptionsMap && fOptionsMap->FindObject(key))
      return kTRUE;
   return kFALSE;
}

//______________________________________________________________________________
void TUrl::CleanRelativePath()
{
   // Recompute the path removing all relative directory jumps via '..'.

   Ssiz_t slash = 0;
   while ( (slash = fFile.Index("/..") ) != kNPOS) {
      // find backwards the next '/'
      Bool_t found = kFALSE;
      for (int l = slash-1; l >=0; l--) {
         if (fFile[l] == '/') {
            // found previous '/'
            fFile.Remove(l, slash+3-l);
            found = kTRUE;
            break;
         }
      }
      if (!found)
        break;
   }
}
