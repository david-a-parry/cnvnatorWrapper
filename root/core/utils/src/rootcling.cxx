// @(#)root/utils:$Id$
// Author: Fons Rademakers   13/07/96

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/rootcint.            *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// CREDITS                                                              //
//                                                                      //
// This program generates the CINT dictionaries needed in order to      //
// get access to your classes via the interpreter.                      //
// In addition rootcint can generate the Streamer(),                    //
// TBuffer &operator>>() and ShowMembers() methods for ROOT classes,    //
// i.e. classes using the ClassDef and ClassImp macros.                 //
//                                                                      //
// Rootcint can be used like:                                           //
//                                                                      //
//  rootcint TAttAxis.h[{+,-}][!] ... [LinkDef.h] > AxisDict.cxx        //
//                                                                      //
// or                                                                   //
//                                                                      //
//  rootcint [-v[0-4]][-l][-f] dict.C [-c] [-p]                         //
//           file.h[{+,-}][!] ... [LinkDef.h]                           //
//                                                                      //
// The difference between the two is that in the first case only the    //
// Streamer() and ShowMembers() methods are generated while in the      //
// latter case a complete compileable file is generated (including      //
// the include statements). The first method also allows the            //
// output to be appended to an already existing file (using >>).        //
// The optional - behind the header file name tells rootcint to not     //
// generate the Streamer() method. A custom method must be provided     //
// by the user in that case. For the + and ! options see below.         //
// When using option -c also the interpreter method interface stubs     //
// will be written to the output file (AxisDict.cxx in the above case). //
// By default the output file will not be overwritten if it exists.     //
// Use the -f (force) option to overwite the output file. The output    //
// file must have one of the following extensions: .cxx, .C, .cpp,      //
// .cc, .cp.                                                            //
// Use the -p option to request the use of the compiler's preprocessor  //
// instead of CINT's preprocessor.  This is useful to handle header\n"  //
// files with macro construct not handled by CINT.\n\n"                 //
// Use the -l (long) option to prepend the pathname of the              //
// dictionary source file to the include of the dictionary header.      //
// This might be needed in case the dictionary file needs to be         //
// compiled with the -I- option that inhibits the use of the directory  //
// of the source file as the first search directory for                 //
// "#include "file"".                                                   //
// The flag --lib-list-prefix=xxx can be used to produce a list of      //
// libraries needed by the header files being parsed. Rootcint will     //
// read the content of xxx.in for a list of rootmap files (see          //
// rlibmap). Rootcint will read these files and use them to deduce a    //
// list of libraries that are needed to properly link and load this     //
// dictionary. This list of libraries is saved in the first line of the //
// file xxx.out; the remaining lines contains the list of classes for   //
// which this run of rootcint produced a dictionary.                    //
// This feature is used by ACliC (the automatic library generator).     //
// The verbose flags have the following meaning:                        //
//      -v   Display all messages                                       //
//      -v0  Display no messages at all.                                //
//      -v1  Display only error messages (default).                     //
//      -v2  Display error and warning messages.                        //
//      -v3  Display error, warning and note messages.                  //
//      -v4  Display all messages                                       //
// rootcint also support the other CINT options (see 'cint -h)          //
//                                                                      //
// Before specifying the first header file one can also add include     //
// file directories to be searched and preprocessor defines, like:      //
//   -I$MYPROJECT/include -DDebug=1                                     //
//                                                                      //
// The (optional) file LinkDef.h looks like:                            //
//                                                                      //
// #ifdef __CINT__                                                      //
//                                                                      //
// #pragma link off all globals;                                        //
// #pragma link off all classes;                                        //
// #pragma link off all functions;                                      //
//                                                                      //
// #pragma link C++ class TAxis;                                        //
// #pragma link C++ class TAttAxis-;                                    //
// #pragma link C++ class TArrayC-!;                                    //
// #pragma link C++ class AliEvent+;                                    //
//                                                                      //
// #pragma link C++ function StrDup;                                    //
// #pragma link C++ function operator+(const TString&,const TString&);  //
//                                                                      //
// #pragma link C++ global gROOT;                                       //
// #pragma link C++ global gEnv;                                        //
//                                                                      //
// #pragma link C++ enum EMessageTypes;                                 //
//                                                                      //
// #endif                                                               //
//                                                                      //
// This file tells rootcint for which classes the method interface      //
// stubs should be generated. A trailing - in the class name tells      //
// rootcint to not generate the Streamer() method. This is necessary    //
// for those classes that need a customized Streamer() method.          //
// A trailing ! in the class name tells rootcint to not generate the    //
// operator>>(TBuffer &b, MyClass *&obj) function. This is necessary to //
// be able to write pointers to objects of classes not inheriting from  //
// TObject. See for an example the source of the TArrayF class.         //
// If the class contains a ClassDef macro, a trailing + in the class    //
// name tells rootcint to generate an automatic Streamer(), i.e. a      //
// streamer that let ROOT do automatic schema evolution. Otherwise, a   //
// trailing + in the class name tells rootcint to generate a ShowMember //
// function and a Shadow Class. The + option is mutually exclusive with //
// the - option. For new classes the + option is the                    //
// preferred option. For legacy reasons it is not yet the default.      //
// When the linkdef file is not specified a default version exporting   //
// the classes with the names equal to the include files minus the .h   //
// is generated.                                                        //
//                                                                      //
// *** IMPORTANT ***                                                    //
// 1) LinkDef.h must be the last argument on the rootcint command line. //
// 2) Note that the LinkDef file name MUST contain the string:          //
//    LinkDef.h, Linkdef.h or linkdef.h, i.e. NA49_LinkDef.h is fine    //
//    just like, linkdef1.h. Linkdef.h is case sensitive.               //
//                                                                      //
// The default constructor used by the ROOT I/O can be customized by    //
// using the rootcint pragma:                                           //
//    #pragma link C++ ioctortype UserClass;                            //
// For example, with this pragma and a class named MyClass,             //
// this method will called the first of the following 3                 //
// constructors which exists and is public:                             //
//    MyClass(UserClass*);                                              //
//    MyClass(TRootIOCtor*);                                            //
//    MyClass(); // Or a constructor with all its arguments defaulted.  //
//                                                                      //
// When more than one pragma ioctortype is used, the first seen has     //
// priority.  For example with:                                         //
//    #pragma link C++ ioctortype UserClass1;                           //
//    #pragma link C++ ioctortype UserClass2;                           //
// We look in the following order:                                      //
//    MyClass(UserClass1*);                                             //
//    MyClass(UserClass2*);                                             //
//    MyClass(TRootIOCtor*);                                            //
//    MyClass(); // Or a constructor with all its arguments defaulted.  //
//                                                                      //
// ----------- historical ---------                                     //
//                                                                      //
// Note that the file rootcint.C is constructed in such a way that it   //
// can also be interpreted by CINT. The above two statements become in  //
// that case:                                                           //
//                                                                      //
// cint -I$ROOTSYS/include +V TAttAxis.h TAxis.h LinkDef.h rootcint.C \ //
//                            TAttAxis.h TAxis.h > AxisGen.C            //
//                                                                      //
// or                                                                   //
//                                                                      //
// cint -I$ROOTSYS/include +V TAttAxis.h TAxis.h LinkDef.h rootcint.C \ //
//                            AxisGen.C TAttAxis.h TAxis.h              //
//                                                                      //
// The +V and -I$ROOTSYS/include options are added to the list of       //
// arguments in the compiled version of rootcint.                       //
//////////////////////////////////////////////////////////////////////////

#ifndef __CINT__

#include "RConfigure.h"
#include "RConfig.h"
#include "Rtypes.h"
#include <iostream>
#include <memory>
#include "Shadow.h"
#include "cintdictversion.h"
#include "FastAllocString.h"
#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/CIFactory.h"
#include "cling/Interpreter/Value.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Pragma.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/CXXInheritance.h"


#ifdef __APPLE__
#include <libgen.h> // Needed for basename
#include <mach-o/dyld.h>
#endif

#if defined(R__WIN32)
#include "cygpath.h"
#endif

#ifdef fgets // in G__ci.h
#  undef fgets
#  undef printf
#  undef fprintf
#  undef fputc
#  undef putc
#  undef putchar
#  undef fputs
#  undef puts
#  undef fgets
#  undef gets
#  undef system
#endif

extern "C" {
   void  G__setothermain(int othermain);
   int  G__setglobalcomp(int globalcomp);
   int   G__main(int argc, char **argv);
   void  G__exit(int rtn);
   struct G__includepath *G__getipathentry();
}
const char *ShortTypeName(const char *typeDesc);

const char *help =
"\n"
"This program generates the CINT dictionaries needed in order to\n"
"get access to your classes via the interpreter.\n"
"In addition rootcint can generate the Streamer(), TBuffer &operator>>()\n"
"and ShowMembers() methods for ROOT classes, i.e. classes using the\n"
"ClassDef and ClassImp macros.\n"
"\n"
"Rootcint can be used like:\n"
"\n"
"  rootcint TAttAxis.h[{+,-}][!] ... [LinkDef.h] > AxisDict.cxx\n"
"\n"
"or\n"
"\n"
"  rootcint [-v[0-4]] [-l] [-f] dict.C [-c] [-p] TAxis.h[{+,-}][!] ... [LinkDef.h] \n"
"\n"
"The difference between the two is that in the first case only the\n"
"Streamer() and ShowMembers() methods are generated while in the\n"
"latter case a complete compileable file is generated (including\n"
"the include statements). The first method also allows the\n"
"output to be appended to an already existing file (using >>).\n"
"The optional - behind the header file name tells rootcint\n"
"to not generate the Streamer() method. A custom method must be\n"
"provided by the user in that case. For the + and ! options see below.\n\n"
"When using option -c also the interpreter method interface stubs\n"
"will be written to the output file (AxisDict.cxx in the above case).\n"
"By default the output file will not be overwritten if it exists.\n"
"Use the -f (force) option to overwite the output file. The output\n"
"file must have one of the following extensions: .cxx, .C, .cpp,\n"
".cc, .cp.\n\n"
"Use the -p option to request the use of the compiler's preprocessor\n"
"instead of CINT's preprocessor.  This is useful to handle header\n"
"files with macro construct not handled by CINT.\n\n"
"Use the -l (long) option to prepend the pathname of the\n"
"dictionary source file to the include of the dictionary header.\n"
"This might be needed in case the dictionary file needs to be\n"
"compiled with the -I- option that inhibits the use of the directory\n"
"of the source file as the first search directory for\n"
"\"#include \"file\"\".\n"
"The flag --lib-list-prefix=xxx can be used to produce a list of\n"
"libraries needed by the header files being parsed. Rootcint will\n"
"read the content of xxx.in for a list of rootmap files (see\n"
"rlibmap). Rootcint will read these files and use them to deduce a\n"
"list of libraries that are needed to properly link and load this\n"
"dictionary. This list of libraries is saved in the file xxx.out.\n"
"This feature is used by ACliC (the automatic library generator).\n"
"The verbose flags have the following meaning:\n"
"      -v   Display all messages\n"
"      -v0  Display no messages at all.\n"
"      -v1  Display only error messages (default).\n"
"      -v2  Display error and warning messages.\n"
"      -v3  Display error, warning and note messages.\n"
"      -v4  Display all messages\n"
"rootcint also support the other CINT options (see 'cint -h)\n"
"\n"
"Before specifying the first header file one can also add include\n"
"file directories to be searched and preprocessor defines, like:\n"
"   -I../include -DDebug\n"
"\n"
"The (optional) file LinkDef.h looks like:\n"
"\n"
"#ifdef __CINT__\n"
"\n"
"#pragma link off all globals;\n"
"#pragma link off all classes;\n"
"#pragma link off all functions;\n"
"\n"
"#pragma link C++ class TAxis;\n"
"#pragma link C++ class TAttAxis-;\n"
"#pragma link C++ class TArrayC-!;\n"
"#pragma link C++ class AliEvent+;\n"
"\n"
"#pragma link C++ function StrDup;\n"
"#pragma link C++ function operator+(const TString&,const TString&);\n"
"\n"
"#pragma link C++ global gROOT;\n"
"#pragma link C++ global gEnv;\n"
"\n"
"#pragma link C++ enum EMessageTypes;\n"
"\n"
"#endif\n"
"\n"
"This file tells rootcint for which classes the method interface\n"
"stubs should be generated. A trailing - in the class name tells\n"
"rootcint to not generate the Streamer() method. This is necessary\n"
"for those classes that need a customized Streamer() method.\n"
"A trailing ! in the class name tells rootcint to not generate the\n"
"operator>>(TBuffer &b, MyClass *&obj) method. This is necessary to\n"
"be able to write pointers to objects of classes not inheriting from\n"
"TObject. See for an example the source of the TArrayF class.\n"
"If the class contains a ClassDef macro, a trailing + in the class\n"
"name tells rootcint to generate an automatic Streamer(), i.e. a\n"
"streamer that let ROOT do automatic schema evolution. Otherwise, a\n"
"trailing + in the class name tells rootcint to generate a ShowMember\n"
"function and a Shadow Class. The + option is mutually exclusive with\n"
"the - option. For new classes the + option is the\n"
"preferred option. For legacy reasons it is not yet the default.\n"
"When this linkdef file is not specified a default version exporting\n"
"the classes with the names equal to the include files minus the .h\n"
"is generated.\n"
"\n"
"*** IMPORTANT ***\n"
"1) LinkDef.h must be the last argument on the rootcint command line.\n"
"2) Note that the LinkDef file name MUST contain the string:\n"
"   LinkDef.h, Linkdef.h or linkdef.h, i.e. NA49_LinkDef.h is fine,\n"
"   just like linkdef1.h. Linkdef.h is case sensitive.\n";

#else
#include <ertti.h>
#endif

#ifdef _WIN32
#ifdef system
#undef system
#endif
#include <windows.h>
#include <Tlhelp32.h> // for MAX_MODULE_NAME32
#include <process.h>
#include <errno.h>
#endif

#include <time.h>
#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include <sys/stat.h>

namespace std {}
using namespace std;

//#include <fstream>
//#include <strstream>

#include "TClassEdit.h"
using namespace TClassEdit;
#include "TMetaUtils.h"
using namespace ROOT;

#include "RClStl.h"
#include "RConversionRuleParser.h"
#include "XMLReader.h"
#include "LinkdefReader.h"
#include "SelectionRules.h"
#include "Scanner.h"

using namespace ROOT;

const char *autoldtmpl = "G__auto%dLinkDef.h";
char autold[64];

std::ostream* dictSrcOut=&std::cout;
G__ShadowMaker *shadowMaker=0;

bool gNeedCollectionProxy = false;

enum EDictType {
   kDictTypeCint,
   kDictTypeReflex,
   kDictTypeGCCXML
} dict_type = kDictTypeCint;

char *StrDup(const char *str);

class RConstructorType {
   std::string           fArgTypeName;
   clang::CXXRecordDecl *fArgType;

public:
   RConstructorType(const char *type_of_arg) : fArgTypeName(type_of_arg),fArgType(0) {}

   const char *GetName() { return fArgTypeName.c_str(); }
};

vector<RConstructorType> gIoConstructorTypes;
void AddConstructorType(const char *arg)
{
   if (arg) gIoConstructorTypes.push_back(RConstructorType(arg));
}

bool ClassInfo__HasMethod(const clang::RecordDecl *cl, const char* name) 
{
   const clang::CXXRecordDecl* CRD = llvm::dyn_cast<clang::CXXRecordDecl>(cl);
   if (!CRD) {
      return false;
   }
   std::string given_name(name);
   for (
        clang::CXXRecordDecl::method_iterator M = CRD->method_begin(),
        MEnd = CRD->method_end();
        M != MEnd;
        ++M
        ) 
   {
      if (M->getNameAsString() == given_name) {
         return true;
      }
   }
   return false;
}

bool Namespace__HasMethod(const clang::NamespaceDecl *cl, const char* name) 
{
   std::string given_name(name);
   for (
        clang::DeclContext::decl_iterator M = cl->decls_begin(),
        MEnd = cl->decls_begin();
        M != MEnd;
        ++M
        ) {
      if (M->isFunctionOrFunctionTemplate()) {
         clang::NamedDecl *named = llvm::dyn_cast<clang::NamedDecl>(*M);
         if (named && named->getNameAsString() == given_name) {
            return true;
         }
      }
   }
   return false;
}

llvm::StringRef R__GetFileName(const clang::Decl *decl)
{
   const clang::CXXRecordDecl* clxx = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
   if (clxx) {
      switch(clxx->getTemplateSpecializationKind()) {
         case clang::TSK_Undeclared:
            // We want the default behavior
            break;
         case clang::TSK_ExplicitInstantiationDeclaration:
         case clang::TSK_ExplicitInstantiationDefinition:
         case clang::TSK_ImplicitInstantiation: {
            // We want the location of the template declaration:
            const clang::ClassTemplateSpecializationDecl *tmplt_specialization = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl> (clxx);
            if (tmplt_specialization) {
               return R__GetFileName(const_cast< clang::ClassTemplateSpecializationDecl *>(tmplt_specialization)->getSpecializedTemplate());
            }
            break;
         }
         case clang::TSK_ExplicitSpecialization:
            // We want the default behavior
            break;
         default:
            break;
      } 
   }      
   clang::SourceLocation sourceLocation = decl->getLocation();
   clang::SourceManager& sourceManager = decl->getASTContext().getSourceManager();

   if (sourceLocation.isValid() && sourceLocation.isFileID()) {
      clang::PresumedLoc PLoc = sourceManager.getPresumedLoc(sourceLocation);
      return PLoc.getFilename();
   }
   else {
      return "invalid";
   }   
}

long R__GetLineNumber(const clang::Decl *decl)
{
   const clang::CXXRecordDecl* clxx = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
   if (clxx) {
      switch(clxx->getTemplateSpecializationKind()) {
         case clang::TSK_Undeclared:
            // We want the default behavior
            break;
         case clang::TSK_ExplicitInstantiationDeclaration:
         case clang::TSK_ExplicitInstantiationDefinition:
         case clang::TSK_ImplicitInstantiation: {
            // We want the location of the template declaration:
            const clang::ClassTemplateSpecializationDecl *tmplt_specialization = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl> (clxx);
            if (tmplt_specialization) {
               return R__GetLineNumber(const_cast< clang::ClassTemplateSpecializationDecl *>(tmplt_specialization)->getSpecializedTemplate());
            }
            break;
         }
         case clang::TSK_ExplicitSpecialization:
            // We want the default behavior
            break;
         default:
            break;
      } 
   }      
   clang::SourceLocation sourceLocation = decl->getLocation();
   clang::SourceManager& sourceManager = decl->getASTContext().getSourceManager();

   if (sourceLocation.isValid() && sourceLocation.isFileID()) {
      return sourceManager.getLineNumber(sourceManager.getFileID(sourceLocation),sourceManager.getFileOffset(sourceLocation));
   }
   else {
      return -1;
   }   
}

const char *R__GetComment(const clang::Decl &decl)
{
   clang::SourceManager& sourceManager = decl.getASTContext().getSourceManager();
   clang::SourceLocation sourceLocation = decl.getLocation();

   // Guess where the comment start.   
   const char *comment = sourceManager.getCharacterData(sourceLocation);
   // Find the end of declaration:
   while (comment[0]!=';' && comment[0]!='\n' && comment[0]!='\r') {
      ++comment;
   }
   ++comment;
   // Now skip the spaces and beginning of comments.
   while ( (isspace(comment[0]) || comment[0]=='/') && comment[0]!='\n' && comment[0]!='\r') {
      ++comment;
   }
   
//   while (comment[0]!='\n' && comment[0]!='\r') {
//      ++comment;
//   }

   return comment;
}

void R__GetQualifiedName(std::string &qual_name, const clang::QualType &type, const clang::NamedDecl &forcontext)
{
   type.getAsStringInternal(qual_name,forcontext.getASTContext().getPrintingPolicy());
}

void R__GetQualifiedName(std::string &qual_name, const clang::NamedDecl &cl)
{
   cl.getNameForDiagnostic(qual_name,cl.getASTContext().getPrintingPolicy(),true); // qual_name = N->getQualifiedNameAsString();
}

void R__GetQualifiedName(std::string &qual_name, const RScanner::AnnotatedRecordDecl &annotated)
{
   R__GetQualifiedName(qual_name,*annotated.GetRecordDecl());
}

std::string  R__GetQualifiedName(const clang::QualType &type, const clang::NamedDecl &forcontext)
{
   std::string result;
   R__GetQualifiedName(result,type,forcontext);
   return result;
}

std::string  R__GetQualifiedName(const clang::Type &type, const clang::NamedDecl &forcontext)
{
   std::string result;
   R__GetQualifiedName(result,clang::QualType(&type,0),forcontext);
   return result;
}

std::string R__GetQualifiedName(const clang::NamedDecl &cl)
{
   std::string result;
   cl.getNameForDiagnostic(result,cl.getASTContext().getPrintingPolicy(),true); // qual_name = N->getQualifiedNameAsString();
   return result;
}

std::string R__GetQualifiedName(const clang::CXXBaseSpecifier &base)
{
   std::string result;
   R__GetQualifiedName(result,*base.getType()->getAsCXXRecordDecl());
   return result;
}

std::string R__GetQualifiedName(const RScanner::AnnotatedRecordDecl &annotated)
{
   return R__GetQualifiedName(*annotated.GetRecordDecl());
}

std::string R__TrueName(const clang::FieldDecl &m)
{
   // TrueName strips the typedefs and array dimensions.
   
   const clang::Type *rawtype = m.getType()->getCanonicalTypeInternal().getTypePtr();   
   if (rawtype->isArrayType()) {
      rawtype = rawtype->getBaseElementTypeUnsafe ();
   }
   
   std::string result;
   R__GetQualifiedName(result, clang::QualType(rawtype,0), m);
   return result;
}

bool R__IsInt(const clang::Type *type) 
{
   const clang::BuiltinType * builtin = llvm::dyn_cast<clang::BuiltinType>(type->getCanonicalTypeInternal().getTypePtr());
   if (builtin) {
      return builtin->isInteger(); // builtin->getKind() == clang::BuiltinType::Int;
   } else {
      return false;
   }
}

bool R__IsInt(const clang::FieldDecl *field) 
{
   return R__IsInt(field->getType().getTypePtr());
}

cling::Interpreter *gInterp = 0;

const clang::NamedDecl *R__SlowSearchImpl(const char *name, const clang::DeclContext *ctxt = 0) 
{
   // First find the left most part, search it and proceed with the search inside it.

   int paran = 0;
   int nest = 0;
   unsigned int len = strlen(name);
   for(unsigned int i = 0; i < len; ++i) {
      switch (name[i]) {
         case '(' : ++paran; break;
         case ')' : --paran; break;
         case '<' : if (paran==0) ++nest; break;
         case '>' : if (paran==0) --nest; break;
         case ':' : if (paran==0 && nest==0) {
            // We got the end of the leftpart.
            std::string leftpart(name);
            leftpart[i] = '\0';
            const clang::NamedDecl *scope = gInterp->LookupDecl(leftpart.c_str(), ctxt).getSingleDecl();
            if (!scope) {
               return 0;
            } else {
               const clang::DeclContext *scope_ctxt = llvm::dyn_cast<clang::DeclContext>(scope);
               if (scope_ctxt) {
                  return R__SlowSearchImpl(name+i+2,scope_ctxt);
               } else {
                  return 0;
               }
            }
         }
      }   
      if (paran < 0 || nest < 0) {
         // malformed, we can't find it.
         return 0;
      }
   }
   // If we get here, we have a decomposed named (unqualified),
   // if the lookup fails, we may have a request for a template instance
   // and which case, let's use the 'function template' based lookup
   const clang::NamedDecl *result = gInterp->LookupDecl(name, ctxt).getSingleDecl();
   if (!result && strchr(name,'<') != 0)
   {
      string fullname = name;
      fullname[ strchr(fullname.c_str(),'<')-fullname.c_str() ] = '\0';
      const clang::NamedDecl *templateResult = gInterp->LookupDecl(fullname.c_str(), ctxt);
      if (templateResult) {
         // The template exist, let's look for an instance.
         fullname = "";
         if (ctxt) {
            R__GetQualifiedName(fullname,*llvm::dyn_cast<clang::NamedDecl>(ctxt));
            fullname += "::";
         }
         fullname += name;
         clang::QualType type = TMetaUtils::LookupTypeDecl(*gInterp,fullname.c_str());
         if (!type.isNull()) {
            const clang::Type *typeptr = type.getTypePtr();
            if (typeptr) result = type->getAsCXXRecordDecl();
         }
      }
   }
   return result;
}

const clang::NamedDecl *R__SlowSearch(const char *name, const clang::DeclContext *ctxt = 0) 
{
   const clang::NamedDecl *result = R__SlowSearchImpl(name,0);

   if (!result) {
      // For compatiblity with CINT, we also search the 'std' namespace.
      static const clang::NamespaceDecl *std_decl = llvm::dyn_cast_or_null<clang::NamespaceDecl>(R__SlowSearchImpl("std"));
      if (std_decl) {
         result = R__SlowSearchImpl(name,std_decl);
      }
   }
   return result;
}

const clang::CXXRecordDecl *R__SlowClassSearch(const char *name, const clang::DeclContext *ctxt = 0) 
{
   const clang::NamedDecl *result = R__SlowSearch(name,0);

   // Now, let's check with cling's lookupClass 
   const clang::CXXRecordDecl *result_cxx = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(result);
   const clang::CXXRecordDecl *result_new = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(gInterp->lookupClass(name));
   if (!result_new) {
      std::string std_name("std::");
      std_name += name;
      result_new = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(gInterp->lookupClass(std_name));
   }
   if (result_cxx != result_new) {
      // Humm ok, we are not the same
      fprintf(stderr,"Error: lookupClass disagree with R__SlowSearchImpl on %s: lookupClass: %s slowClassSearch: %s\n",
              name, result_new ? R__GetQualifiedName(*result_new).c_str() : " nothing ", result ? R__GetQualifiedName(*result_cxx).c_str() : " nothing ");
   }

   if (result) {
      return llvm::dyn_cast<clang::CXXRecordDecl>(result);
   } else {
      return 0;
   }
}

const clang::Type *R__GetUnderlyingType(clang::QualType type)
{
   // Return the base/underlying type of a chain of array or pointers type.
   // Does not yet support the array and pointer part being intermixed.
   
   const clang::Type *rawtype = type.getTypePtr();

   if (rawtype->isArrayType()) {
      rawtype = type.getTypePtr()->getBaseElementTypeUnsafe ();
   }   
   if (rawtype->isPointerType() || rawtype->isReferenceType() ) {
      //Get to the 'raw' type.
      clang::QualType pointee;
      while ( (pointee = rawtype->getPointeeType()) , pointee.getTypePtrOrNull() && pointee.getTypePtr() != rawtype)
      {
         rawtype = pointee.getTypePtr();
      }
   }
   if (rawtype->isArrayType()) {
      rawtype = type.getTypePtr()->getBaseElementTypeUnsafe ();
   }
   return rawtype;
}


clang::RecordDecl *R__GetUnderlyingRecordDecl(clang::QualType type)
{
   const clang::Type *rawtype = R__GetUnderlyingType(type);

   if (rawtype->isFundamentalType() || rawtype->isEnumeralType()) {
      // not an ojbect.
      return 0;
   }
   return rawtype->getAsCXXRecordDecl();
}

size_t R__GetFullArrayLength(const clang::ConstantArrayType *arrayType)
{
   llvm::APInt len = arrayType->getSize();
   while(const clang::ConstantArrayType *subArrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual()) )
   {
      len *= subArrayType->getSize();
      arrayType = subArrayType;
   }
   return len.getLimitedValue();
}

const clang::CXXRecordDecl *R__SlowRawTypeSearch(const char *input_name, const clang::DeclContext *ctxt = 0) 
{
   // Strip leading 'const' and '*' and '&'

   if (strncmp("const ",input_name,6)==0) {
      input_name += 6;
   }
   bool done = false;
   std::string name(input_name);
   while (!done) {
      done = true;
      if (6<name.length() && strncmp(" const",&(name.c_str()[name.length()-6]),6)==0 ) {
         name.erase(name.length()-6);
         done = false;
      }
      while(name.length() && name[name.length()-1] == '*' ) {
         name.erase(name.length()-1);
         done = false;
      }
      while(name.length() && name[name.length()-1] == '&' ) {
         name.erase(name.length()-1);
         done = false;
      }
      while (6<name.length() && strncmp("*const",&(name.c_str()[name.length()-6]),6)==0 ) {
         name.erase(name.length()-6);
         done = false;
      }
      while (6<name.length() && strncmp("&const",&(name.c_str()[name.length()-6]),6)==0 ) {
         name.erase(name.length()-6);
         done = false;
      }
   }
   const clang::NamedDecl *result = R__SlowSearch(name.c_str(),0);

   if (!result) return 0;

   const clang::TypedefDecl *typedef_result = llvm::dyn_cast<clang::TypedefDecl>(result);
   if (typedef_result) {
      result = R__GetUnderlyingRecordDecl(typedef_result->getUnderlyingType());
   }
   if (result) {
      // Try to find a complete definition if any.
      const clang::TagDecl *tagdecl = llvm::dyn_cast<clang::TagDecl>(result);
      while(tagdecl && !tagdecl->isCompleteDefinition()) {
         tagdecl = tagdecl->getPreviousDecl();
      }
      if (tagdecl) {
         result = tagdecl;
      }
      return llvm::dyn_cast<clang::CXXRecordDecl>(result);
   } else {
      return 0;
   }
}

class FDVisitor : public clang::RecursiveASTVisitor<FDVisitor> {
private:
   clang::FunctionDecl* fFD;
public:
   clang::FunctionDecl* getFD() const { return fFD; }
   bool VisitDeclRefExpr(clang::DeclRefExpr* DRE) {
      fFD = llvm::dyn_cast<clang::FunctionDecl>(DRE->getDecl());
      return true;
   }
   FDVisitor() : fFD(0) {}
};


//______________________________________________________________________________
const clang::CXXMethodDecl *R__GetFuncWithProto(const clang::CXXRecordDecl* cinfo, 
                                                const char *method, const char *proto)
{
#define USE_CLING_LOOKUP

   // First check it exist under some name.
   const clang::FunctionDecl *decl = gInterp->LookupDecl(method,cinfo).getAs<clang::FunctionDecl>();
   bool found_at_least_one = false;
   if (decl) {
      found_at_least_one = true;
   } else {
      bool is_operator_new = (strcmp(method,"operator new")==0);
      // Do a crawling search :( ...
      for(clang::CXXRecordDecl::method_iterator iter = cinfo->method_begin(),
          end = cinfo->method_end();
          iter != end;
          ++iter)
      {
         std::string methodname;
         iter->getNameForDiagnostic( methodname, cinfo->getASTContext().getPrintingPolicy(), false );
         if ( methodname == method ) {
            found_at_least_one = true;
            if (is_operator_new) return &*iter;
            break;
         }
      }
   }
   if (!found_at_least_one) {
      for(clang::CXXRecordDecl::base_class_const_iterator iter = cinfo->bases_begin(), end = cinfo->bases_end();
          iter != end;
          ++iter)
      {
         if (const clang::CXXMethodDecl *res = R__GetFuncWithProto((iter->getType()->getAsCXXRecordDecl ()),method,proto)) {
            return res;
         }
      }
   }
#ifndef USE_CLING_LOOKUP
   if (decl) {
      // NOTE this is *wrong* we do not check that the routine has the right arguments!!!
      return llvm::dyn_cast<clang::CXXMethodDecl>(decl);
   }
#else
   if (found_at_least_one) {
      // Build int (ClassInfo::*f)(arglist) = &ClassInfo::method;
      // Then we will have the resolved overload of clang::FunctionDecl on the lhs.
      std::string Str("");
      gInterp->createUniqueName(Str);
      Str += "void " + Str + "() {\n";
      Str += "void (" + cinfo->getNameAsString() + "::*fptr)(" + proto + ") = &" + cinfo->getNameAsString()  + "::" + method + ";;";
      Str += "\n};;";

      std::string uniquename;
      gInterp->createUniqueName(uniquename);

      Str = "template<typename CLASS, typename RET> void " + uniquename + "lookup_arg(RET (CLASS::* const arg)(";
      Str += proto;
      Str += ") ) { };\n";

      Str = "template<typename CLASS, typename RET> void " + uniquename + "lookup_arg(RET (CLASS::*)(";
      Str += proto;
      Str += ") ) { };\n";
      Str += "void " + uniquename + "_wrapper() {\n";
      Str += uniquename + "lookup_arg(& " + R__GetQualifiedName(*cinfo)  + "::" + method + ");\n";
      Str += ";;};;";

      const clang::Decl* D = 0;
      const clang::FunctionDecl* ResultFD = 0;
      cling::Value value;
      if (gInterp->declare(Str, &D)
          == cling::Interpreter::kSuccess) {
         if (D) {
            FDVisitor FDV = FDVisitor();
            FDV.TraverseDecl(D->getASTContext().getTranslationUnitDecl());
            ResultFD = FDV.getFD();
         }
         if (ResultFD) {
            const clang::CXXMethodDecl *method = llvm::dyn_cast<clang::CXXMethodDecl>(ResultFD);
            return method;
         }
      }
   }
#endif
   return 0;
}

bool R__CheckPublicFuncWithProto(const clang::CXXRecordDecl *cl, const char *methodname, const char *proto)
{
   // Return true, if the function (defined by the name and prototype) exists and is public

   const clang::CXXMethodDecl *method = R__GetFuncWithProto(cl,methodname,proto);
   if (method && method->getAccess() == clang::AS_public) {
      return true;
   }
   return false;
}

bool ClassInfo__IsBase(const clang::RecordDecl *cl, const char* basename)
{
   const clang::CXXRecordDecl* CRD = llvm::dyn_cast<clang::CXXRecordDecl>(cl);
   if (!CRD) {
      return false;
   }
   // This would be better but is to verbose for now.
   // clang::QualType basetype = TMetaUtils::LookupTypeDecl(*gInterp, basename);
   const clang::NamedDecl *base = R__SlowClassSearch(basename);
   if (base) {
      const clang::CXXRecordDecl* baseCRD = llvm::dyn_cast<clang::CXXRecordDecl>( base ); 
      if (baseCRD) return CRD->isDerivedFrom(baseCRD);
   }
   return false;
}

bool R__IsBase(const clang::CXXRecordDecl *cl, const clang::CXXRecordDecl *base)
{
   if (!cl || !base) {
      return false;
   }
   return cl->isDerivedFrom(base);
}

bool R__IsBase(const clang::FieldDecl &m, const char* basename)
{
   const clang::CXXRecordDecl* CRD = llvm::dyn_cast<clang::CXXRecordDecl>(R__GetUnderlyingRecordDecl(m.getType()));
   if (!CRD) {
      return false;
   }
   
   // This would be better but is to verbose for now.
   // clang::QualType basetype = TMetaUtils::LookupTypeDecl(*gInterp, basename);
   const clang::NamedDecl *base = R__SlowClassSearch(basename);
   
   if (base) {
      const clang::CXXRecordDecl* baseCRD = llvm::dyn_cast<clang::CXXRecordDecl>( base ); 
      if (baseCRD) return CRD->isDerivedFrom(baseCRD);
   }
   return false;
}


bool InheritsFromTObject(const clang::RecordDecl *cl)
{
   static const clang::CXXRecordDecl *TObject_decl = R__SlowClassSearch("TObject");

   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl);
   return R__IsBase(clxx, TObject_decl);
}

bool InheritsFromTSelector(const clang::RecordDecl *cl)
{
   static const clang::CXXRecordDecl *TObject_decl = R__SlowClassSearch("TSelector");

   return R__IsBase(llvm::dyn_cast<clang::CXXRecordDecl>(cl), TObject_decl);
}

bool IsStdClass(const clang::CXXRecordDecl &cl)
{
   // Return true, if the decl is part of the std namespace.

   const clang::DeclContext *ctx = cl.getDeclContext();

   if (ctx->isNamespace())
   {
      const clang::NamedDecl *parent = llvm::dyn_cast<clang::NamedDecl> (ctx);
      if (parent) {
         if (parent->getQualifiedNameAsString()=="std") {
            return true;
         }
      }
   }
   return false;
}

void R__GetName(std::string &qual_name, const clang::NamedDecl *cl)
{
   cl->getNameForDiagnostic(qual_name,cl->getASTContext().getPrintingPolicy(),false); // qual_name = N->getQualifiedNameAsString();
}

inline bool R__IsTemplate(const clang::CXXRecordDecl *cl)
{
   return cl->getTemplateSpecializationKind() != clang::TSK_Undeclared;
}

bool R__IsSelectionXml(const char *filename)
{
   size_t len = strlen(filename);
   size_t xmllen = 4; /* strlen(".xml"); */
   if (strlen(filename) >= xmllen ) {
      return (0 == strcasecmp( filename + (len - xmllen), ".xml"));
   } else {
      return false;
   }
}

bool R__IsLinkdefFile(const char *filename)
{
   if ((strstr(filename,"LinkDef") || strstr(filename,"Linkdef") ||
        strstr(filename,"linkdef")) && strstr(filename,".h")) {
      return true;
   }
   size_t len = strlen(filename);
   size_t linkdeflen = 9; /* strlen("linkdef.h") */
   if (len >= 9) {
      if (0 == strncasecmp( filename + (len - linkdeflen), "linkdef", linkdeflen-2)
          && 0 == strcmp(filename + (len - 2),".h")
          ) {
         return true;
      } else {
         return false;
      }
   } else {
      return false;
   }
}

bool R__IsSelectionFile(const char *filename)
{
   return R__IsLinkdefFile(filename) || R__IsSelectionXml(filename);
}

//const char* root_style()  {
//  static const char* s = ::getenv("MY_ROOT");
//  return s;
//}

//______________________________________________________________________________

#ifndef ROOT_Varargs
#include "Varargs.h"
#endif

const int kInfo     =      0;
const int kNote     =    500;
const int kWarning  =   1000;
const int kError    =   2000;
const int kSysError =   3000;
const int kFatal    =   4000;
const int kMaxLen   =   1024;

static int gErrorIgnoreLevel = kError;

//______________________________________________________________________________
void LevelPrint(bool prefix, int level, const char *location,
                const char *fmt, va_list ap)
{
   if (level < gErrorIgnoreLevel)
      return;

   const char *type = 0;

   if (level >= kInfo)
      type = "Info";
   if (level >= kNote)
      type = "Note";
   if (level >= kWarning)
      type = "Warning";
   if (level >= kError)
      type = "Error";
   if (level >= kSysError)
      type = "SysError";
   if (level >= kFatal)
      type = "Fatal";

   if (!location || strlen(location) == 0) {
      if (prefix) fprintf(stderr, "%s: ", type);
      vfprintf(stderr, (char*)va_(fmt), ap);
   } else {
      if (prefix) fprintf(stderr, "%s in <%s>: ", type, location);
      else fprintf(stderr, "In <%s>: ", location);
      vfprintf(stderr, (char*)va_(fmt), ap);
   }

   fflush(stderr);
}

//______________________________________________________________________________
void Error(const char *location, const char *va_(fmt), ...)
{
   // Use this function in case an error occured.

   va_list ap;
   va_start(ap,va_(fmt));
   LevelPrint(true, kError, location, va_(fmt), ap);
   va_end(ap);
}

//______________________________________________________________________________
void SysError(const char *location, const char *va_(fmt), ...)
{
   // Use this function in case a system (OS or GUI) related error occured.

   va_list ap;
   va_start(ap, va_(fmt));
   LevelPrint(true, kSysError, location, va_(fmt), ap);
   va_end(ap);
}

//______________________________________________________________________________
void Info(const char *location, const char *va_(fmt), ...)
{
   // Use this function for informational messages.

   va_list ap;
   va_start(ap,va_(fmt));
   LevelPrint(true, kInfo, location, va_(fmt), ap);
   va_end(ap);
}

//______________________________________________________________________________
void Warning(const char *location, const char *va_(fmt), ...)
{
   // Use this function in warning situations.

   va_list ap;
   va_start(ap,va_(fmt));
   LevelPrint(true, kWarning, location, va_(fmt), ap);
   va_end(ap);
}

//______________________________________________________________________________
void Fatal(const char *location, const char *va_(fmt), ...)
{
   // Use this function in case of a fatal error. It will abort the program.

   va_list ap;
   va_start(ap,va_(fmt));
   LevelPrint(true, kFatal, location, va_(fmt), ap);
   va_end(ap);
}

//______________________________________________________________________________
const char *GetExePath()
{
   // Returns the executable path name, used by SetRootSys().

   static std::string exepath;
   if (exepath == "") {
#ifdef __APPLE__
      exepath = _dyld_get_image_name(0);
#endif
#ifdef __linux
      char linkname[PATH_MAX];  // /proc/<pid>/exe
      char buf[PATH_MAX];     // exe path name
      pid_t pid;

      // get our pid and build the name of the link in /proc
      pid = getpid();
      snprintf(linkname,PATH_MAX, "/proc/%i/exe", pid);
      int ret = readlink(linkname, buf, 1024);
      if (ret > 0 && ret < 1024) {
         buf[ret] = 0;
         exepath = buf;
      }
#endif
#ifdef _WIN32
   char *buf = new char[MAX_MODULE_NAME32 + 1];
   ::GetModuleFileName(NULL, buf, MAX_MODULE_NAME32 + 1);
   char* p = buf;
   while ((p = strchr(p, '\\')))
      *(p++) = '/';
   exepath = buf;
   delete[] buf;
#endif
   }
   return exepath.c_str();
}

//______________________________________________________________________________
void SetRootSys()
{
   // Set the ROOTSYS env var based on the executable location.

   const char *exepath = GetExePath();
   if (exepath && *exepath) {
#if !defined(_WIN32)
      char *ep = new char[PATH_MAX];
      if (!realpath(exepath, ep)) {
         if (getenv("ROOTSYS")) {
            delete [] ep;
            return;
         } else {
            fprintf(stderr, "rootcint: error getting realpath of rootcint, please set ROOTSYS in the shell");
            strlcpy(ep, exepath,PATH_MAX);
         }
      }
#else
      int nche = strlen(exepath)+1;
      char *ep = new char[nche];
      strlcpy(ep, exepath,nche);
#endif
      char *s;
      if ((s = strrchr(ep, '/'))) {
         // $ROOTSYS/bin/rootcint
         int removesubdirs = 2;
         if (!strncmp(s, "rootcint_tmp", 12))
            // $ROOTSYS/core/utils/src/rootcint_tmp
            removesubdirs = 4;
         for (int i = 1; s && i < removesubdirs; ++i) {
            *s = 0;
            s = strrchr(ep, '/');
         }
         if (s) *s = 0;
      } else {
         // There was no slashes at all let now change ROOTSYS
         return;
      }
      int ncha = strlen(ep) + 10;
      char *env = new char[ncha];
      snprintf(env, ncha, "ROOTSYS=%s", ep);
      putenv(env);
      delete [] ep;
   }
}

//______________________________________________________________________________
bool ParsePragmaLine(const std::string& line, const char* expectedTokens[],
                     size_t* end = 0) {
   // Check whether the #pragma line contains expectedTokens (0-terminated array).
   if (end) *end = 0;
   if (line[0] != '#') return false;
   size_t pos = 1;
   for (const char** iToken = expectedTokens; *iToken; ++iToken) {
      while (isspace(line[pos])) ++pos;
      size_t lenToken = strlen(*iToken);
      if (line.compare(pos, lenToken, *iToken)) {
         if (end) *end = pos;
         return false;
      }
      pos += lenToken;
   }
   if (end) *end = pos;
   return true;
}

namespace {
   class R__tmpnamElement {
   public:
      R__tmpnamElement() : fTmpnam() {}
      R__tmpnamElement(const std::string& tmpnam): fTmpnam(tmpnam) {}
      ~R__tmpnamElement() { unlink(fTmpnam.c_str()); }
   private:
      string fTmpnam;
   };
}

#ifndef R__USE_MKSTEMP
# if defined(R__GLIBC) || defined(__FreeBSD__) || \
    (defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_5))
#  define R__USE_MKSTEMP 1
# endif
#endif

//______________________________________________________________________________
string R__tmpnam()
{
   // return a unique temporary file name as defined by tmpnam

   static char filename[L_tmpnam+2];
   static string tmpdir;
   static bool initialized = false;
   static list<R__tmpnamElement> tmpnamList;


   if (!initialized) {
#if R__USE_MKSTEMP
      // Unlike tmpnam mkstemp does not prepend anything
      // to its result but must get the pattern as a
      // full pathname.
      tmpdir = std::string(P_tmpdir) + "/";
#endif

      if (strlen(P_tmpdir) <= 2) {
         // tmpnam (see man page) prepends the value of the
         // P_tmpdir (defined in stdio.h) to its result.
         // If P_tmpdir is less that 2 character it is likely to
         // just be '/' or '\\' and we do not want to write in
         // the root directory, so let's add the temp directory.
         char *tmp;
         if ((tmp = getenv("CINTTMPDIR"))) tmpdir = tmp;
         else if ((tmp=getenv("TEMP")))    tmpdir = tmp;
         else if ((tmp=getenv("TMP")))     tmpdir = tmp;
         else tmpdir = ".";
         tmpdir += '/';
      }
      initialized = true;
   }

#if R__USE_MKSTEMP
   static const char *radix = "XXXXXX";
   static const char *prefix = "rootcint_";
   if (tmpdir.length() + strlen(radix) + strlen(prefix) + 2 > L_tmpnam + 2) {
      // too long
      std::cerr << "Temporary file name too long! Trying with /tmp..." << std::endl;
      tmpdir = "/tmp/";
   }
   strlcpy(filename, tmpdir.c_str(),L_tmpnam+2);
   strlcat(filename, prefix,L_tmpnam+2);
   strlcat(filename, radix,L_tmpnam+2);
   mode_t old_umask = umask(077); // be restrictive for mkstemp()
   int temp_fileno = mkstemp(filename);/*mkstemp not only generate file name but also opens the file*/
   umask(old_umask);
   if (temp_fileno >= 0) {
      close(temp_fileno);
   }
   remove(filename);
   tmpnamList.push_back(R__tmpnamElement(filename));
   return filename;

#else
   tmpnam(filename);

   string result(tmpdir);
   result += filename;
   result += "_rootcint";
   tmpnamList.push_back(R__tmpnamElement(result));
   return result;
#endif
}

#ifdef _WIN32
//______________________________________________________________________________
// defined in newlink.c
extern "C" FILE *FOpenAndSleep(const char *filename, const char *mode);

# ifdef fopen
#  undef fopen
# endif
# define fopen(A,B) FOpenAndSleep((A),(B))
#endif


//______________________________________________________________________________
typedef map<string,string> Recmap_t;
Recmap_t gAutoloads;
string gLiblistPrefix;
string gLibsNeeded;

int AutoLoadCallbackImpl(char *c, char *)
{
   string need( gAutoloads[c] );
   if (need.length() && gLibsNeeded.find(need)==string::npos) {
      gLibsNeeded += " " + need;
   }
   return -1; // We did not actually 'succeed' in loading the definition.
}

extern "C" int AutoLoadCallback(char *c, char *l)
{
   return AutoLoadCallbackImpl(c,l);
}

void LoadLibraryMap()
{
   string filelistname = gLiblistPrefix + ".in";
   ifstream filelist(filelistname.c_str());

   string filename;
   static char *buffer = 0;
   static unsigned int sbuffer = 0;

   while ( filelist >> filename ) {
#ifdef WIN32
      struct _stati64 finfo;

      if (_stati64(filename.c_str(), &finfo) < 0 ||
          finfo.st_mode & S_IFDIR) {
         continue;
      }
#else
      struct stat finfo;
      if (stat(filename.c_str(), &finfo) < 0 ||
          S_ISDIR(finfo.st_mode)) {
         continue;
      }

#endif

      ifstream file(filename.c_str());

      string line;
      string classname;

      while ( file >> line ) {

         if (line.substr(0,8)=="Library.") {

            int pos = line.find(":",8);
            classname = line.substr(8,pos-8);

            pos = 0;
            while ( (pos=classname.find("@@",pos)) >= 0 ) {
               classname.replace(pos,2,"::");
            }
            pos = 0;
            while ( (pos=classname.find("-",pos)) >= 0 ) {
               classname.replace(pos,1," ");
            }

            getline(file,line,'\n');

            while( line[0]==' ' ) line.replace(0,1,"");

            if ( strchr(classname.c_str(),':')!=0 ) {
               // We have a namespace and we have to check it first

               int slen = classname.size();
               for(int k=0;k<slen;++k) {
                  if (classname[k]==':') {
                     if (k+1>=slen || classname[k+1]!=':') {
                        // we expected another ':'
                        break;
                     }
                     if (k) {
                        string base = classname.substr(0,k);
                        if (base=="std") {
                           // std is not declared but is also ignored by CINT!
                           break;
                        } else {
                           gAutoloads[base] = ""; // We never load namespaces on their own.
                           if (sbuffer < base.size()+20) {
                              delete [] buffer;
                              sbuffer = base.size()+20;
                              buffer = new char[sbuffer];
                           }
                           strlcpy(buffer,base.c_str(),sbuffer);
                           G__set_class_autoloading_table(buffer, (char*)""); // We never load namespaces on their own.
                        }
                        ++k;
                     }
                  } else if (classname[k] == '<') {
                     // We do not want to look at the namespace inside the template parameters!
                     break;
                  }
               }
            }

            if (strncmp("ROOT::TImpProxy",classname.c_str(),strlen("ROOT::TImpProxy"))==0) {
               // Do not register the ROOT::TImpProxy so that they can be instantiated.
               continue;
            }
            gAutoloads[classname] = line;
            if (sbuffer < classname.size()+20) {
               delete [] buffer;
               sbuffer = classname.size()+20;
               buffer = new char[sbuffer];
            }
            strlcpy(buffer,classname.c_str(),sbuffer);
            G__set_class_autoloading_table(buffer,(char*)line.c_str());
         }
      }
      file.close();
   }
}

extern "C" {
   typedef void G__parse_hook_t ();
   G__parse_hook_t* G__set_beforeparse_hook (G__parse_hook_t* hook);
}

//______________________________________________________________________________
void BeforeParseInit()
{
   // If needed initialize the autoloading hook
   if (gLiblistPrefix.length()) {
      G__set_class_autoloading_table((char*)"ROOT", (char*)"libCore.so");
      LoadLibraryMap();
      G__set_class_autoloading_callback(&AutoLoadCallback);
   }

   //---------------------------------------------------------------------------
   // Add the conversion rule processors
   //---------------------------------------------------------------------------
   G__addpragma( (char*)"read", ProcessReadPragma );
   G__addpragma( (char*)"readraw", ProcessReadRawPragma );

}


//______________________________________________________________________________
bool CheckInputOperator(G__ClassInfo &cl, int dicttype)
{
   // Check if the operator>> has been properly declared if the user has
   // resquested a custom version.

   bool has_input_error = false;

   // Need to find out if the operator>> is actually defined for
   // this class.
   G__ClassInfo gcl;
   long offset;

   int ncha = strlen(cl.Fullname())+13;
   char *proto = new char[ncha];
   snprintf(proto,ncha,"TBuffer&,%s*&",cl.Fullname());

   G__MethodInfo methodinfo = gcl.GetMethod("operator>>",proto,&offset);

   Info(0, "Class %s: Do not generate operator>>()\n",
        cl.Fullname());
   G__MethodArgInfo args( methodinfo );
   args.Next(); args.Next();
   if (!methodinfo.IsValid() ||
       !args.IsValid() ||
       args.Type()==0 ||
       args.Type()->Tagnum() != cl.Tagnum() ||
       strstr(methodinfo.FileName(),"TBuffer.h")!=0 ||
       strstr(methodinfo.FileName(),"Rtypes.h" )!=0) {

      if (dicttype==0||dicttype==1){
         // We don't want to generate duplicated error messages in several dictionaries (when generating temporaries)
         Error(0,
               "in this version of ROOT, the option '!' used in a linkdef file\n"
               "       implies the actual existence of customized operators.\n"
               "       The following declaration is now required:\n"
               "   TBuffer &operator>>(TBuffer &,%s *&);\n",cl.Fullname());

      }

      has_input_error = true;
   } else {
      // Warning(0, "TBuffer &operator>>(TBuffer &,%s *&); defined at line %s %d \n",cl.Fullname(),methodinfo.FileName(),methodinfo.LineNumber());
   }
   // fprintf(stderr, "DEBUG: %s %d\n",methodinfo.FileName(),methodinfo.LineNumber());

   methodinfo = gcl.GetMethod("operator<<",proto,&offset);
   args.Init(methodinfo);
   args.Next(); args.Next();
   if (!methodinfo.IsValid() ||
       !args.IsValid() ||
       args.Type()==0 ||
       args.Type()->Tagnum() != cl.Tagnum() ||
       strstr(methodinfo.FileName(),"TBuffer.h")!=0 ||
       strstr(methodinfo.FileName(),"Rtypes.h" )!=0) {

      if (dicttype==0||dicttype==1){
         Error(0,
               "in this version of ROOT, the option '!' used in a linkdef file\n"
               "       implies the actual existence of customized operator.\n"
               "       The following declaration is now required:\n"
               "   TBuffer &operator<<(TBuffer &,const %s *);\n",cl.Fullname());
      }

      has_input_error = true;
   } else {
      //fprintf(stderr, "DEBUG: %s %d\n",methodinfo.FileName(),methodinfo.LineNumber());
   }

   delete [] proto;
   return has_input_error;
}

//______________________________________________________________________________
bool CheckInputOperator(const clang::RecordDecl *cl, int dicttype)
{
   // Check if the operator>> has been properly declared if the user has
   // resquested a custom version.

   bool has_input_error = false;

   G__ClassInfo clinfo(R__GetQualifiedName(*cl).c_str());
   return CheckInputOperator(clinfo,dicttype);
#if 0
   // Need to find out if the operator>> is actually defined for
   // this class.
   G__ClassInfo gcl;
   long offset;

   int ncha = strlen(cl.Fullname())+13;
   char *proto = new char[ncha];
   snprintf(proto,ncha,"TBuffer&,%s*&",cl.Fullname());

   G__MethodInfo methodinfo = gcl.GetMethod("operator>>",proto,&offset);

   Info(0, "Class %s: Do not generate operator>>()\n",
        cl.Fullname());
   G__MethodArgInfo args( methodinfo );
   args.Next(); args.Next();
   if (!methodinfo.IsValid() ||
       !args.IsValid() ||
       args.Type()==0 ||
       args.Type()->Tagnum() != cl.Tagnum() ||
       strstr(methodinfo.FileName(),"TBuffer.h")!=0 ||
       strstr(methodinfo.FileName(),"Rtypes.h" )!=0) {

      if (dicttype==0||dicttype==1){
         // We don't want to generate duplicated error messages in several dictionaries (when generating temporaries)
         Error(0,
               "in this version of ROOT, the option '!' used in a linkdef file\n"
               "       implies the actual existence of customized operators.\n"
               "       The following declaration is now required:\n"
               "   TBuffer &operator>>(TBuffer &,%s *&);\n",cl.Fullname());

      }

      has_input_error = true;
   } else {
      // Warning(0, "TBuffer &operator>>(TBuffer &,%s *&); defined at line %s %d \n",cl.Fullname(),methodinfo.FileName(),methodinfo.LineNumber());
   }
   // fprintf(stderr, "DEBUG: %s %d\n",methodinfo.FileName(),methodinfo.LineNumber());

   methodinfo = gcl.GetMethod("operator<<",proto,&offset);
   args.Init(methodinfo);
   args.Next(); args.Next();
   if (!methodinfo.IsValid() ||
       !args.IsValid() ||
       args.Type()==0 ||
       args.Type()->Tagnum() != cl.Tagnum() ||
       strstr(methodinfo.FileName(),"TBuffer.h")!=0 ||
       strstr(methodinfo.FileName(),"Rtypes.h" )!=0) {

      if (dicttype==0||dicttype==1){
         Error(0,
               "in this version of ROOT, the option '!' used in a linkdef file\n"
               "       implies the actual existence of customized operator.\n"
               "       The following declaration is now required:\n"
               "   TBuffer &operator<<(TBuffer &,const %s *);\n",cl.Fullname());
      }

      has_input_error = true;
   } else {
      //fprintf(stderr, "DEBUG: %s %d\n",methodinfo.FileName(),methodinfo.LineNumber());
   }

   delete [] proto;
#endif
   return has_input_error;
}

string FixSTLName(const string& cintName) {

   const char *s = cintName.c_str();
   char type[kMaxLen];
   strlcpy(type, s,kMaxLen);

#if 0 // (G__GNUC<3) && !defined (G__KCC)
   if (!strncmp(type, "vector",6)   ||
       !strncmp(type, "list",4)     ||
       !strncmp(type, "deque",5)    ||
       !strncmp(type, "map",3)      ||
       !strncmp(type, "multimap",8) ||
       !strncmp(type, "set",3)      ||
       !strncmp(type, "multiset",8) ) {

      // we need to remove this type of construct ",__malloc_alloc_template<0>"
      // in case of older gcc
      string result;
      unsigned int i;
      unsigned int start;
      unsigned int next = 0;
      unsigned int nesting = 0;
      unsigned int end;
      const string toReplace = "__malloc_alloc_template<0>";
      const string replacement = "alloc";
      for(i=0; i<cintName.length(); i++) {
         switch (cintName[i]) {
         case '<':
            if (nesting==0) {
               start = next;
               next = i+1;
               end = i;
               result += cintName.substr(start,end-start);
               result += "< ";
            }
            nesting++;
            break;
         case '>':
            nesting--;
            if (nesting==0) {
               start = next;
               next = i+1;
               end = i;
               string param = cintName.substr(start,end-start-1); // the -1 removes the space we know is there
               if (param==toReplace) {
                  result += replacement;
               } else {
                  result += FixSTLName(param);
               }
               result += " >";
            }
            break;
         case ',':
            if (nesting==1) {
               start = next;
               next = i+1;
               end = i;
               string param = cintName.substr(start,end-start);
               if (param==toReplace) {
                  result += replacement;
               } else {
                  result += FixSTLName(param);
               }
               result += ',';
            }
         }
      }
      return result;
   }
#endif
   return cintName;
}

//______________________________________________________________________________
bool CheckClassDef(const clang::RecordDecl *cl)
{
   // Return false if the class does not have ClassDef even-though it should.


   // Detect if the class has a ClassDef
   bool hasClassDef = ClassInfo__HasMethod(cl,"Class_Version");

   /*
    The following could be use to detect whether one of the
    class' parent class has a ClassDef

    long offset;
    const char *proto = "";
    const char *name = "IsA";

    G__MethodInfo methodinfo = cl.GetMethod(name,proto,&offset);
    bool parentHasClassDef = methodinfo.IsValid() && (methodinfo.Property() & G__BIT_ISPUBLIC);
    */

   // Avoid unadvertently introducing a dependency on libTree.so (when running with
   // the --lib-list-prefix option.
   //int autoloadEnable = G__set_class_autoloading(0);
   const clang::CXXRecordDecl* clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl);
   if (!clxx) {
      return false;
   }
   bool isAbstract = clxx->isAbstract();
   //G__set_class_autoloading(autoloadEnable);

   bool result = true;
   if (!isAbstract && InheritsFromTObject(clxx) && !InheritsFromTSelector(clxx)
       && !hasClassDef) {
      Error(R__GetQualifiedName(*cl).c_str(),"CLING: %s inherits from TObject but does not have its own ClassDef\n",R__GetQualifiedName(*cl).c_str());
      // We do want to always output the message (hence the Error level)
      // but still want rootcint to succeed.
      result = true;
   }

   // This check is disabled for now.
   return result;
}

//______________________________________________________________________________
bool HasDirectoryAutoAdd(const clang::CXXRecordDecl *cl)
{
   // Return true if the class has a method DirectoryAutoAdd(TDirectory *)

   // Detect if the class has a DirectoryAutoAdd

   // Detect if the class or one of its parent has a DirectoryAutoAdd
   const char *proto = "TDirectory*";
   const char *name = "DirectoryAutoAdd";

   return R__CheckPublicFuncWithProto(cl,name,proto);
}


//______________________________________________________________________________
bool HasNewMerge(const clang::CXXRecordDecl *cl)
{
   // Return true if the class has a method Merge(TCollection*,TFileMergeInfo*)

   // Detect if the class has a 'new' Merge function.

   // Detect if the class or one of its parent has a DirectoryAutoAdd
   const char *proto = "TCollection*,TFileMergeInfo*";
   const char *name = "Merge";

   return R__CheckPublicFuncWithProto(cl,name,proto);
}

//______________________________________________________________________________
bool HasOldMerge(const clang::CXXRecordDecl *cl)
{
   // Return true if the class has a method Merge(TCollection*)

   // Detect if the class has an old fashion Merge function.

   // Detect if the class or one of its parent has a DirectoryAutoAdd
   const char *proto = "TCollection*";
   const char *name = "Merge";

   return R__CheckPublicFuncWithProto(cl,name,proto);
}


//______________________________________________________________________________
bool HasResetAfterMerge(const clang::CXXRecordDecl *cl)
{
   // Return true if the class has a method ResetAfterMerge(TFileMergeInfo *)

   // Detect if the class has a 'new' Merge function.
   // bool hasMethod = cl.HasMethod("DirectoryAutoAdd");

   // Detect if the class or one of its parent has a DirectoryAutoAdd
   const char *proto = "TFileMergeInfo*";
   const char *name = "ResetAfterMerge";

   return R__CheckPublicFuncWithProto(cl,name,proto);
}

//______________________________________________________________________________
int GetClassVersion(G__ClassInfo &cl)
{
   // Return the version number of the class or -1
   // if the function Class_Version does not exist.

   if (!cl.HasMethod("Class_Version")) return -1;

   const char *function = "::Class_Version()";
   string funcname = GetLong64_Name( cl.Fullname() ) + function;
   int version = (int)G__int(G__calc(funcname.c_str()));
   return version;
}

//______________________________________________________________________________
int GetClassVersion(const clang::RecordDecl *cl)
{
   // Return the version number of the class or -1
   // if the function Class_Version does not exist.

   if (!ClassInfo__HasMethod(cl,"Class_Version")) return -1;

   const clang::CXXRecordDecl* CRD = llvm::dyn_cast<clang::CXXRecordDecl>(cl);
   if (!CRD) {
      // Must be an enum or namespace.
      // FIXME: Make it work for a namespace!
      return false;
   }
   std::string given_name("Class_Version");
   for (
        clang::CXXRecordDecl::method_iterator M = CRD->method_begin(),
        MEnd = CRD->method_end();
        M != MEnd;
        ++M
        ) {
      if (M->getNameAsString() == given_name) {
         clang::CompoundStmt *func;
         if (M->getBody()) {
            func = llvm::dyn_cast<clang::CompoundStmt>(M->getBody());
         } else {
            const clang::FunctionDecl *inst = M->getInstantiatedFromMemberFunction();
            if (inst->getBody()) {
               func = llvm::dyn_cast<clang::CompoundStmt>(inst->getBody());
            } else {
               Error("GetClassVersion","Could not find the body for %s::ClassVersion!",R__GetQualifiedName(*cl).c_str());
            }
         }
         if (func && !func->body_empty()) {
            clang::ReturnStmt *ret = llvm::dyn_cast<clang::ReturnStmt>(*func->body_begin());
            if (ret) {
               clang::IntegerLiteral *val;
               clang::ImplicitCastExpr *cast = llvm::dyn_cast<clang::ImplicitCastExpr>( ret->getRetValue() );
               if (cast) {
                  val = llvm::dyn_cast<clang::IntegerLiteral>( cast->getSubExprAsWritten() );
               } else {
                  val = llvm::dyn_cast<clang::IntegerLiteral>( ret->getRetValue() );
               }
               if (val) {
                  return (int)val->getValue().getLimitedValue(~0);
               }
            }
         }
         return 0;
      }
   }
   return 0;   
}

string GetNonConstMemberName(G__DataMemberInfo &m, const string &prefix = "")
{
   // Return the name of the data member so that it can be used
   // by non-const operation (so it includes a const_cast if necessary).
   
   if (m.Property() & (G__BIT_ISCONSTANT|G__BIT_ISPCONSTANT)) {
      string ret = "const_cast< ";
      ret += G__ShadowMaker::GetNonConstTypeName(m);
      ret += " &>( ";
      ret += prefix;
      ret += m.Name();
      ret += " )";
      return ret;
   } else {
      return prefix+m.Name();
   }
}

//______________________________________________________________________________
string GetNonConstMemberName(const clang::FieldDecl &m, const string &prefix = "")
{
   // Return the name of the data member so that it can be used
   // by non-const operation (so it includes a const_cast if necessary).

   if (m.getType().isConstQualified()) {      
      string ret = "const_cast< ";
      string type_name;
      R__GetQualifiedName(type_name, m.getType(), m);
      ret += type_name;
      ret += " &>( ";
      ret += prefix;
      ret += m.getName().data();
      ret += " )";
      return ret;
   } else {
      return prefix+m.getName().data();
   }
}

//______________________________________________________________________________
bool NeedShadowClass(G__ClassInfo& cl)
{
   if (G__ShadowMaker::IsStdPair(cl)) return true;
   if (G__ShadowMaker::IsSTLCont(cl.Name())) return false;
   if (strcmp(cl.Name(),"string") == 0 ) return false;

   if (strcmp(cl.Name(),"complex<float>") == 0 || strcmp(cl.Name(),"complex<double>") == 0) return true;

   if (cl.FileName() && !strncmp(cl.FileName(),"prec_stl",8)) {
      // Allow I/O for auto_ptr ...
      if (strncmp(cl.Name(),"auto_ptr<",strlen("auto_ptr<"))==0) return true;
      return false;
   }
   // This means templated classes hiding members won't have
   // a proper shadow class, and the use has no change of
   // vetoring a shadow, as we need it for ShowMembers :-/
   if (cl.HasMethod("ShowMembers"))
      return dict_type == kDictTypeReflex || cl.IsTmplt();

   // no streamer, no shadow
   if (cl.RootFlag() == G__NOSTREAMER) return false;

   if (dict_type == kDictTypeReflex) return true;

   return ((cl.RootFlag() & G__USEBYTECOUNT));
}

//______________________________________________________________________________
bool NeedTypedefShadowClass(G__ClassInfo& cl)
{
   // shadow class is a typedef if the class has a ClassDef, and is not a templated class
   return (cl.HasMethod("Class_Name") && !cl.IsTmplt());
}

//______________________________________________________________________________
bool NeedTemplateKeyword(G__ClassInfo &cl)
{
   if (cl.IsTmplt()) {
      char *templatename = StrDup(cl.Fullname());
      char *loc = strstr(templatename, "<");
      if (loc) *loc = 0;
      struct G__Definedtemplateclass *templ = G__defined_templateclass(templatename);
      if (templ) {

         int current = cl.Tagnum();
         G__IntList * ilist = templ->instantiatedtagnum;
         while(ilist) {
            if (ilist->i == current) {
               delete [] templatename;
               // This is an automatically instantiated templated class.
               return true;
            }
            ilist = ilist->next;
         }

         delete [] templatename;
         // This is a specialized templated class
         return false;

      } else {

         delete [] templatename;
         // It might be a specialization without us seeing the template definition
         return false;
      }
   }
   return false;
}

//______________________________________________________________________________
bool NeedTemplateKeyword(const clang::CXXRecordDecl *cl)
{
   clang::TemplateSpecializationKind kind = cl->getTemplateSpecializationKind();
   if (kind == clang::TSK_Undeclared ) {
      // Note a template;
      return false;
   } else if (kind == clang::TSK_ExplicitSpecialization) {
      // This is a specialized templated class
      return false;
   } else {
      // This is an automatically or explicitly instantiated templated class.
      return true;
   }  
}

bool HasCustomOperatorNew(G__ClassInfo& cl);
bool HasCustomOperatorNewPlacement(G__ClassInfo& cl);
bool HasCustomOperatorNewArrayPlacement(G__ClassInfo& cl);
bool HasDefaultConstructor(G__ClassInfo& cl,string *args=0);

//______________________________________________________________________________
bool HasCustomOperatorNew(const clang::RecordDecl *cl)
{
   // return true if we can find a custom operator new

   return false;
#if 0
   // Look for a custom operator new
   bool custom = false;
   G__ClassInfo gcl;
   long offset;
   const char *name = "operator new";
   const char *proto = "size_t";

   // first in the global namespace:
   G__MethodInfo methodinfo = gcl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   // in nesting space:
   gcl = cl.EnclosingSpace();
   methodinfo = gcl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   // in class
   methodinfo = cl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   return custom;
#endif
}

bool HasCustomOperatorNew(G__ClassInfo& cl)
{
   // return true if we can find a custom operator new

   // Look for a custom operator new
   bool custom = false;
   G__ClassInfo gcl;
   long offset;
   const char *name = "operator new";
   const char *proto = "size_t";

   // first in the global namespace:
   G__MethodInfo methodinfo = gcl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   // in nesting space:
   gcl = cl.EnclosingSpace();
   methodinfo = gcl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   // in class
   methodinfo = cl.GetMethod(name,proto,&offset);
   if  (methodinfo.IsValid()) {
      custom = true;
   }

   return custom;
}

//______________________________________________________________________________
bool HasCustomOperatorNewPlacement(G__ClassInfo& cl)
{
   // return true if we can find a custom operator new

   // Look for a custom operator new
   bool custom = false;
   G__ClassInfo gcl;
   long offset;
   const char *name = "operator new";
   const char *proto = "size_t";
   const char *protoPlacement = "size_t,void*";

   // first in the global namespace:
   G__MethodInfo methodinfo = gcl.GetMethod(name,proto,&offset);
   G__MethodInfo methodinfoPlacement = gcl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      // We have a custom new placement in the global namespace
      custom = true;
   }

   // in nesting space:
   gcl = cl.EnclosingSpace();
   methodinfo = gcl.GetMethod(name,proto,&offset);
   methodinfoPlacement = gcl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      custom = true;
   }

   // in class
   methodinfo = cl.GetMethod(name,proto,&offset);
   methodinfoPlacement = cl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      // We have a custom operator new with placement in the class
      // hierarchy.  We still need to check that it has not been
      // overloaded by a simple operator new.

      G__ClassInfo clNew(methodinfo.GetDefiningScopeTagnum());
      G__ClassInfo clPlacement(methodinfoPlacement.GetDefiningScopeTagnum());

      if (clNew.IsBase(clPlacement)) {
         // the operator new hides the operator new with placement
         custom = false;
      } else {
         custom = true;
      }
   }

   return custom;
}

//______________________________________________________________________________
bool HasCustomOperatorNewArrayPlacement(G__ClassInfo& cl)
{
   // return true if we can find a custom operator new

   // Look for a custom operator new[]
   bool custom = false;
   G__ClassInfo gcl;
   long offset;
   const char *name = "operator new[]";
   const char *proto = "size_t";
   const char *protoPlacement = "size_t,void*";

   // first in the global namespace:
   G__MethodInfo methodinfo = gcl.GetMethod(name,proto,&offset);
   G__MethodInfo methodinfoPlacement = gcl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      // We have a custom new[] placement in the global namespace
      custom = true;
   }

   // in nesting space:
   gcl = cl.EnclosingSpace();
   methodinfo = gcl.GetMethod(name,proto,&offset);
   methodinfoPlacement = gcl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      custom = true;
   }

   // in class
   methodinfo = cl.GetMethod(name,proto,&offset);
   methodinfoPlacement = cl.GetMethod(name,protoPlacement,&offset);
   if  (methodinfoPlacement.IsValid()) {
      // We have a custom operator new[] with placement in the class
      // hierarchy.  We still need to check that it has not been
      // overloaded by a simple operator new.

      G__ClassInfo clNew(methodinfo.GetDefiningScopeTagnum());
      G__ClassInfo clPlacement(methodinfoPlacement.GetDefiningScopeTagnum());

      if (clNew.IsBase(clPlacement)) {
         // the operator new[] hides the operator new with placement
         custom = false;
      } else {
         custom = true;
      }
   }

   return custom;
}

bool CheckConstructor(const clang::CXXRecordDecl *cl, const char *arg)
{
   if ( (arg == 0 || arg[0] == '\0') && !cl->hasUserDeclaredConstructor() ) {
      return true;
   }

   for(clang::CXXRecordDecl::ctor_iterator iter = cl->ctor_begin(), end = cl->ctor_end();
       iter != end;
       ++iter)
   {
      if (iter->getAccess() == clang::AS_public) {
         // We can reach this constructor.

         if (arg == 0 || arg[0] == '\0') {
            // We are looking for a constructor with zero non-default arguments.

            if (iter->getNumParams() == 0) {
               return true;
            }
            if ( (*iter->param_begin())->hasDefaultArg()) {
               return true;
            }
         } else {

            if (iter->getNumParams() == 1) {
               std::string realArg = (*iter->param_begin())->getType().getAsString();
               // fprintf(stderr,"Checking constructor arg %s against %s\n",realArg.c_str(),arg);
               if (realArg == arg) {
                  return true;
               }
            }
         }
      }
   }
   return false;
}

//______________________________________________________________________________
bool HasDefaultConstructor(const clang::CXXRecordDecl *cl, string *arg)
{
   // return true if we can find an constructor calleable without any arguments

   bool result = false;

   if (cl->isAbstract()) return false;

   for(unsigned int i=0; i<gIoConstructorTypes.size(); ++i) {
      string proto( gIoConstructorTypes[i].GetName() );
      int extra = (proto.size()==0) ? 0 : 1;
      if (extra==0) {
         // Looking for default constructor
         result = true;
      } else {
         proto += " *";
      }

      result = CheckConstructor(cl,proto.c_str());
      if (result && extra && arg) {
         *arg = "( (";
         *arg += proto;
         *arg += ")0 )";
      }

      // Check for private operator new
      if (result) {
         const char *name = "operator new";
         proto = "size_t";
         const clang::CXXMethodDecl *method = R__GetFuncWithProto(cl,name,proto.c_str());
         if (method && method->getAccess() != clang::AS_public) {
            result = false;
         }
         if (result) return true;
      }
   }
   return result;
}


//______________________________________________________________________________
bool NeedDestructor(const clang::CXXRecordDecl *cl)
{
   if (!cl) return false;

   if (cl->hasUserDeclaredDestructor()) {

      clang::CXXDestructorDecl *dest = cl->getDestructor();
      if (dest) {
         return (dest->getAccess() == clang::AS_public);
      } else {
         return true; // no destructor, so let's assume it means default?
      }
   }
   return true;
}

//______________________________________________________________________________
bool HasCustomStreamerMemberFunction(const RScanner::AnnotatedRecordDecl &cl)
{
   // Return true if the class has a custom member function streamer.

   static const char *proto = "TBuffer&";

   const clang::CXXRecordDecl* clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());
   const clang::CXXMethodDecl *method = R__GetFuncWithProto(clxx,"Streamer",proto);
   const clang::DeclContext *clxx_as_context = llvm::dyn_cast<clang::DeclContext>(clxx);

   return (method && method->getDeclContext() == clxx_as_context && ( cl.RequestNoStreamer() || !cl.RequestStreamerInfo()));
}

//______________________________________________________________________________
bool IsTemplateFloat16(G__ClassInfo &cl)
{
   // Return true if any of the argument is or contains a Float16.
   if (!cl.IsTmplt()) return false;

   static G__TypeInfo ti;
   char *current, *next;
   G__FastAllocString arg( cl.Name() );

   // arg is now is the name of class template instantiation.
   // We first need to find the start of the list of its template arguments
   // then we have a comma separated list of type names.  We want to return
   // the 'count+1'-th element in the list.
   int len = strlen(arg);
   int nesting = 0;
   current = 0;
   next = &(arg[0]);
   for (int c = 0; c<len; c++) {
      switch (arg[c]) {
      case '<':
         if (nesting==0) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
         }
         nesting++;
         break;
      case '>':
         nesting--;
         if (nesting==0) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
            if (current) {
               if (strcmp(current,"Float16_t")==0) return true;
               G__ClassInfo subcl(current);
               if (IsTemplateFloat16(subcl)) return true;
            }
         }
         break;
      case ',':
         if (nesting==1) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
            if (current) {
               if (strcmp(current,"Float16_t")==0) return true;
               G__ClassInfo subcl(current);
               if (IsTemplateFloat16(subcl)) return true;
            }
         }
         break;
      }
   }

   return false;
}

//______________________________________________________________________________
bool IsTemplateDouble32(G__ClassInfo &cl)
{
   // Return true if any of the argument is or contains a double32.
   if (!cl.IsTmplt()) return false;

   static G__TypeInfo ti;
   char *current, *next;
   G__FastAllocString arg( cl.Name() );

   // arg is now is the name of class template instantiation.
   // We first need to find the start of the list of its template arguments
   // then we have a comma separated list of type names.  We want to return
   // the 'count+1'-th element in the list.
   int len = strlen(arg);
   int nesting = 0;
   current = 0;
   next = &(arg[0]);
   for (int c = 0; c<len; c++) {
      switch (arg[c]) {
      case '<':
         if (nesting==0) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
         }
         nesting++;
         break;
      case '>':
         nesting--;
         if (nesting==0) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
            if (current) {
               if (strcmp(current,"Double32_t")==0) return true;
               G__ClassInfo subcl(current);
               if (IsTemplateDouble32(subcl)) return true;
            }
         }
         break;
      case ',':
         if (nesting==1) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
            if (current) {
               if (strcmp(current,"Double32_t")==0) return true;
               G__ClassInfo subcl(current);
               if (IsTemplateDouble32(subcl)) return true;
            }
         }
         break;
      }
   }

   return false;
}


//______________________________________________________________________________
bool IsSTLBitset(G__DataMemberInfo &m)
{
   // Is this a std::bitset

   const char *s = m.Type()->TrueName();
   if (!s) return false;

   string type(s);
   return TClassEdit::IsSTLBitset(type.c_str());
}

//______________________________________________________________________________
bool IsSTLBitset(G__BaseClassInfo &m)
{
   // Is this a std::bitset

   const char *s = m.Name();
   if (!s) return false;

   string type(s);
   return TClassEdit::IsSTLBitset(type.c_str());
}

//______________________________________________________________________________
int IsSTLContainer(G__DataMemberInfo &m)
{
   // Is this an STL container?

   const char *s = m.Type()->TrueName();
   if (!s) return kNotSTL;

   string type(s);
   int k = TClassEdit::IsSTLCont(type.c_str(),1);

   //    if (k) printf(" %s==%d\n",type.c_str(),k);

   return k;
}

//______________________________________________________________________________
int IsSTLContainer(const clang::FieldDecl &m)
{
   // Is this an STL container?

   clang::QualType type = m.getType();
   std::string type_name = type.getAsString(m.getASTContext().getPrintingPolicy()); // m.Type()->TrueName();

   int k = TClassEdit::IsSTLCont(type_name.c_str(),1);

   //    if (k) printf(" %s==%d\n",type.c_str(),k);

   return k;
}


//______________________________________________________________________________
int IsSTLContainer(G__BaseClassInfo &m)
{
   // Is this an STL container?

   const char *s = m.Name();
   if (!s) return kNotSTL;

   string type(s);
   int k = TClassEdit::IsSTLCont(type.c_str(),1);
   //   if (k) printf(" %s==%d\n",type.c_str(),k);
   return k;
}

//______________________________________________________________________________
int IsSTLContainer(const clang::CXXBaseSpecifier &base)
{
   // Is this an STL container?

   if (!IsStdClass(*base.getType()->getAsCXXRecordDecl())) {
      return kNotSTL;
   }

   int k = TClassEdit::IsSTLCont(R__GetQualifiedName(*base.getType()->getAsCXXRecordDecl()).c_str(),1);
   //   if (k) printf(" %s==%d\n",type.c_str(),k);
   return k;
}


//______________________________________________________________________________
bool IsStreamableObject(const clang::FieldDecl &m)
{
   const char *comment = R__GetComment( m );

   // Transient
   if (comment[0] == '!') return false;

   clang::QualType type = m.getType();

   if (type->isReferenceType()) {
      // Reference can not be streamed.
      return false;
   }

   std::string mTypeName = type.getAsString(m.getASTContext().getPrintingPolicy());
   if (!strcmp(mTypeName.c_str(), "string") || !strcmp(mTypeName.c_str(), "string*")) {
      return true;
   }
   if (!strcmp(mTypeName.c_str(), "std::string") || !strcmp(mTypeName.c_str(), "std::string*")) {
      return true;
   }

   const clang::Type *rawtype = type.getTypePtr()->getBaseElementTypeUnsafe ();

   if (rawtype->isPointerType()) {
      //Get to the 'raw' type.
      clang::QualType pointee;
      while ( (pointee = rawtype->getPointeeType()) , pointee.getTypePtrOrNull() && pointee.getTypePtr() != rawtype)
      {
        rawtype = pointee.getTypePtr();
      }      
   }

   if (rawtype->isFundamentalType() || rawtype->isEnumeralType()) {
      // not an ojbect.
      return false;
   }

   const clang::CXXRecordDecl *cxxdecl = rawtype->getAsCXXRecordDecl();
   if (cxxdecl && ClassInfo__HasMethod(cxxdecl,"Streamer")) {
      if (!(ClassInfo__HasMethod(cxxdecl,"Class_Version"))) return true;
      int version = GetClassVersion(cxxdecl);
      if (version > 0) return true;
   }
   return false;
}

//______________________________________________________________________________
int IsStreamable(G__DataMemberInfo &m)
{
   // Is this member a Streamable object?

   const char* mTypeName = ShortTypeName(m.Type()->Name());


   if ((m.Property() & G__BIT_ISSTATIC) ||
       strncmp(m.Title(), "!", 1) == 0        ||
       strcmp(m.Name(), "G__virtualinfo") == 0) return 0;

   if (((m.Type())->Property() & G__BIT_ISFUNDAMENTAL) ||
       ((m.Type())->Property() & G__BIT_ISENUM)) return 0;

   if (m.Property() & G__BIT_ISREFERENCE) return 0;

   if (IsSTLContainer(m)) {
      return 1;
   }

   if (!strcmp(mTypeName, "string") || !strcmp(mTypeName, "string*")) return 1;

   if ((m.Type())->HasMethod("Streamer")) {
      if (!(m.Type())->HasMethod("Class_Version")) return 1;
      int version = GetClassVersion(*m.Type());
      if (version > 0) return 1;
   }
   return 0;
}

//______________________________________________________________________________
G__TypeInfo &TemplateArg(G__DataMemberInfo &m, int count = 0)
{
   // Returns template argument. When count = 0 return first argument,
   // 1 second, etc.

   static G__TypeInfo ti;
   char *current, *next;
   G__FastAllocString arg( m.Name() );

   arg = m.Type()->TmpltArg();
   // arg is now a comma separated list of type names, and we want
   // to return the 'count+1'-th element in the list.
   int len = strlen(arg);
   int nesting = 0;
   int i = 0;
   current = 0;
   next = &(arg[0]);
   for (int c = 0; c<len && i<=count; c++) {
      switch (arg[c]) {
      case '<':
         nesting++; break;
      case '>':
         nesting--; break;
      case ',':
         if (nesting==0) {
            arg[c]=0;
            i++;
            current = next;
            next = &(arg[c+1]);
         }
         break;
      }
   }
   if (current) ti.Init(current);

   return ti;
}

//______________________________________________________________________________
G__TypeInfo &TemplateArg(G__BaseClassInfo &m, int count = 0)
{
   // Returns template argument. When count = 0 return first argument,
   // 1 second, etc.

   static G__TypeInfo ti;
   char *current, *next;
   G__FastAllocString arg( m.Name() );

   // arg is now is the name of class template instantiation.
   // We first need to find the start of the list of its template arguments
   // then we have a comma separated list of type names.  We want to return
   // the 'count+1'-th element in the list.
   int len = strlen(arg);
   int nesting = 0;
   int i = 0;
   current = 0;
   next = &(arg[0]);
   for (int c = 0; c<len && i<=count; c++) {
      switch (arg[c]) {
      case '<':
         if (nesting==0) {
            arg[c]=0;
            current = next;
            next = &(arg[c+1]);
         }
         nesting++;
         break;
      case '>':
         nesting--;
         break;
      case ',':
         if (nesting==1) {
            arg[c]=0;
            i++;
            current = next;
            next = &(arg[c+1]);
         }
         break;
      }
   }
   if (current) ti.Init(current);

   return ti;
}

//______________________________________________________________________________
void WriteAuxFunctions(const RScanner::AnnotatedRecordDecl &cl)
{
   // Write the functions that are need for the TGenericClassInfo.
   // This includes
   //    IsA
   //    operator new
   //    operator new[]
   //    operator delete
   //    operator delete[]

   G__ClassInfo clinfo(cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str());
   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());
   if (!clxx) {
      return;
   }

   string classname( GetLong64_Name(RStl::DropDefaultArg( cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str() ) ) );
   string mappedname = G__map_cpp_name((char*)classname.c_str());

   if ( ! TClassEdit::IsStdClass( classname.c_str() ) ) {

      // Prefix the full class name with '::' except for the STL
      // containers and std::string.  This is to request the
      // real class instead of the class in the namespace ROOT::Shadow
      classname.insert(0,"::");
   }

   (*dictSrcOut) << "namespace ROOT {" << std::endl;

   string args;
   if (HasDefaultConstructor(clxx,&args)) {
      // write the constructor wrapper only for concrete classes
      (*dictSrcOut) << "   // Wrappers around operator new" << std::endl
                    << "   static void *new_" << mappedname.c_str() << "(void *p) {" << std::endl
                    << "      return  p ? ";
      if (HasCustomOperatorNewPlacement(clinfo)) {
         (*dictSrcOut) << "new(p) " << classname.c_str() << args << " : ";
      } else {
         (*dictSrcOut) << "::new((::ROOT::TOperatorNewHelper*)p) " << classname.c_str() << args << " : ";
      }
      (*dictSrcOut) << "new " << classname.c_str() << args << ";" << std::endl
                    << "   }" << std::endl;

      if (args.size()==0 && NeedDestructor(clxx)) {
         // Can not can newArray if the destructor is not public.
         (*dictSrcOut) << "   static void *newArray_" << mappedname.c_str() << "(Long_t nElements, void *p) {" << std::endl;
         (*dictSrcOut) << "      return p ? ";
         if (HasCustomOperatorNewArrayPlacement(clinfo)) {
            (*dictSrcOut) << "new(p) " << classname.c_str() << "[nElements] : ";
         } else {
            (*dictSrcOut) << "::new((::ROOT::TOperatorNewHelper*)p) " << classname.c_str() << "[nElements] : ";
         }
         (*dictSrcOut) << "new " << classname.c_str() << "[nElements];" << std::endl;
         (*dictSrcOut) << "   }" << std::endl;
      }
   }

   if (NeedDestructor(clxx)) {
      (*dictSrcOut) << "   // Wrapper around operator delete" << std::endl
                    << "   static void delete_" << mappedname.c_str() << "(void *p) {" << std::endl
                    << "      delete ((" << classname.c_str() << "*)p);" << std::endl
                    << "   }" << std::endl

                    << "   static void deleteArray_" << mappedname.c_str() << "(void *p) {" << std::endl
                    << "      delete [] ((" << classname.c_str() << "*)p);" << std::endl
                    << "   }" << std::endl

                    << "   static void destruct_" << mappedname.c_str() << "(void *p) {" << std::endl
                    << "      typedef " << classname.c_str() << " current_t;" << std::endl
                    << "      ((current_t*)p)->~current_t();" << std::endl
                    << "   }" << std::endl;
   }

   if (HasDirectoryAutoAdd(clxx)) {
       (*dictSrcOut) << "   // Wrapper around the directory auto add." << std::endl
                     << "   static void directoryAutoAdd_" << mappedname.c_str() << "(void *p, TDirectory *dir) {" << std::endl
                     << "      ((" << classname.c_str() << "*)p)->DirectoryAutoAdd(dir);" << std::endl
                     << "   }" << std::endl;
   }

   if (HasCustomStreamerMemberFunction(cl)) {
      (*dictSrcOut) << "   // Wrapper around a custom streamer member function." << std::endl
      << "   static void streamer_" << mappedname.c_str() << "(TBuffer &buf, void *obj) {" << std::endl
      << "      ((" << classname.c_str() << "*)obj)->" << classname.c_str() << "::Streamer(buf);" << std::endl
      << "   }" << std::endl;
   }

   if (HasNewMerge(clxx)) {
      (*dictSrcOut) << "   // Wrapper around the merge function." << std::endl
      << "   static Long64_t merge_" << mappedname.c_str() << "(void *obj,TCollection *coll,TFileMergeInfo *info) {" << std::endl
      << "      return ((" << classname.c_str() << "*)obj)->Merge(coll,info);" << std::endl
      << "   }" << std::endl;
   } else if (HasOldMerge(clxx)) {
      (*dictSrcOut) << "   // Wrapper around the merge function." << std::endl
      << "   static Long64_t  merge_" << mappedname.c_str() << "(void *obj,TCollection *coll,TFileMergeInfo *) {" << std::endl
      << "      return ((" << classname.c_str() << "*)obj)->Merge(coll);" << std::endl
      << "   }" << std::endl;
   }

   if (HasResetAfterMerge(clxx)) {
      (*dictSrcOut) << "   // Wrapper around the Reset function." << std::endl
      << "   static void reset_" << mappedname.c_str() << "(void *obj,TFileMergeInfo *info) {" << std::endl
      << "      ((" << classname.c_str() << "*)obj)->ResetAfterMerge(info);" << std::endl
      << "   }" << std::endl;
   }
   (*dictSrcOut) << "} // end of namespace ROOT for class " << classname.c_str() << std::endl << std::endl;
}

//______________________________________________________________________________
void WriteStringOperators(FILE *fd)
{
   // Write static ANSI C++ string to TBuffer operators.

   fprintf(fd, "//_______________________________________");
   fprintf(fd, "_______________________________________\n");
   fprintf(fd, "static TBuffer &operator>>(TBuffer &b, string &s)\n{\n");
   fprintf(fd, "   // Reading string object.\n\n");
   fprintf(fd, "   R__ASSERT(b.IsReading());\n");
   fprintf(fd, "   char ch;\n");
   fprintf(fd, "   do {\n");
   fprintf(fd, "      b >> ch;\n");
   fprintf(fd, "      if (ch) s.append(1, ch);\n");
   fprintf(fd, "   } while (ch != 0);\n");
   fprintf(fd, "   return b;\n");
   fprintf(fd, "}\n");
   fprintf(fd, "//_______________________________________");
   fprintf(fd, "_______________________________________\n");
   fprintf(fd, "static TBuffer &operator<<(TBuffer &b, string s)\n{\n");
   fprintf(fd, "   // Writing string object.\n\n");
   fprintf(fd, "   R__ASSERT(b.IsWriting());\n");
   fprintf(fd, "   b.WriteString(s.c_str());\n");
   fprintf(fd, "   return b;\n");
   fprintf(fd, "}\n");
}

//______________________________________________________________________________
int ElementStreamer(G__TypeInfo &ti, const char *R__t,int rwmode,const char *tcl=0)
{
   enum {
      R__BIT_ISTOBJECT   = 0x10000000,
      R__BIT_HASSTREAMER = 0x20000000,
      kBIT_ISSTRING    = 0x40000000
   };
   
   long prop = ti.Property();
   string tiName(ti.Name());
   string objType(ShortTypeName(tiName.c_str()));
   string tiFullname;
   if (ti.Fullname())
      tiFullname = ti.Fullname();
   int isTObj = (ti.IsBase("TObject") || tiFullname == "TObject");
   int isStre = (ti.HasMethod("Streamer"));
   
   long kase = prop & (G__BIT_ISPOINTER|G__BIT_ISFUNDAMENTAL|G__BIT_ISENUM);
   if (isTObj)              kase |= R__BIT_ISTOBJECT;
   if (tiName == "string")  kase |= kBIT_ISSTRING;
   if (tiName == "string*") kase |= kBIT_ISSTRING;
   if (isStre)              kase |= R__BIT_HASSTREAMER;
   
   if (tcl == 0) {
      tcl = " internal error in rootcint ";
   }
   //    if (strcmp(objType,"string")==0) RStl::Instance().GenerateTClassFor( "string"  );
   
   if (rwmode == 0) {  //Read mode
      
      if (R__t) (*dictSrcOut) << "            " << tiName << " " << R__t << ";" << std::endl;
      switch (kase) {
            
         case G__BIT_ISFUNDAMENTAL:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            R__b >> " << R__t << ";" << std::endl;
            break;
            
         case G__BIT_ISPOINTER|R__BIT_ISTOBJECT|R__BIT_HASSTREAMER:
            if (!R__t)  return 1;
            (*dictSrcOut) << "            " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");"  << std::endl;
            break;
            
         case G__BIT_ISENUM:
            if (!R__t)  return 0;
            //             fprintf(fp, "            R__b >> (Int_t&)%s;\n",R__t);
            // On some platforms enums and not 'Int_t' and casting to a reference to Int_t
            // induces the silent creation of a temporary which is 'filled' __instead of__
            // the desired enum.  So we need to take it one step at a time.
            (*dictSrcOut) << "            Int_t readtemp;" << std::endl
            << "            R__b >> readtemp;" << std::endl
            << "            " << R__t << " = static_cast<" << tiName << ">(readtemp);" << std::endl;
            break;
            
         case R__BIT_HASSTREAMER:
         case R__BIT_HASSTREAMER|R__BIT_ISTOBJECT:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            " << R__t << ".Streamer(R__b);" << std::endl;
            break;
            
         case R__BIT_HASSTREAMER|G__BIT_ISPOINTER:
            if (!R__t)  return 1;
            //fprintf(fp, "            fprintf(stderr,\"info is %%p %%d\\n\",R__b.GetInfo(),R__b.GetInfo()?R__b.GetInfo()->GetOldVersion():-1);\n");
            (*dictSrcOut) << "            if (R__b.GetInfo() && R__b.GetInfo()->GetOldVersion()<=3) {" << std::endl;
            if (ti.Property() & G__BIT_ISABSTRACT) {
               (*dictSrcOut) << "               R__ASSERT(0);// " << objType << " is abstract. We assume that older file could not be produced using this streaming method." << std::endl;
            } else {
               (*dictSrcOut) << "               " << R__t << " = new " << objType << ";" << std::endl
               << "               " << R__t << "->Streamer(R__b);" << std::endl;
            }
            (*dictSrcOut) << "            } else {" << std::endl
            << "               " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");" << std::endl
            << "            }" << std::endl;
            break;
            
         case kBIT_ISSTRING:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            {TString R__str;" << std::endl
            << "             R__str.Streamer(R__b);" << std::endl
            << "             " << R__t << " = R__str.Data();}" << std::endl;
            break;
            
         case kBIT_ISSTRING|G__BIT_ISPOINTER:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            {TString R__str;"  << std::endl
            << "             R__str.Streamer(R__b);" << std::endl
            << "             " << R__t << " = new string(R__str.Data());}" << std::endl;
            break;
            
         case G__BIT_ISPOINTER:
            if (!R__t)  return 1;
            (*dictSrcOut) << "            " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");" << std::endl;
            break;
            
         default:
            if (!R__t) return 1;
            (*dictSrcOut) << "            R__b.StreamObject(&" << R__t << "," << tcl << ");" << std::endl;
            break;
      }
      
   } else {     //Write case
      
      switch (kase) {
            
         case G__BIT_ISFUNDAMENTAL:
         case G__BIT_ISPOINTER|R__BIT_ISTOBJECT|R__BIT_HASSTREAMER:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            R__b << " << R__t << ";" << std::endl;
            break;
            
         case G__BIT_ISENUM:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            {  void *ptr_enum = (void*)&" << R__t << ";\n";
            (*dictSrcOut) << "               R__b >> *reinterpret_cast<Int_t*>(ptr_enum); }" << std::endl;
            break;
            
         case R__BIT_HASSTREAMER:
         case R__BIT_HASSTREAMER|R__BIT_ISTOBJECT:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            ((" << objType << "&)" << R__t << ").Streamer(R__b);" << std::endl;
            break;
            
         case R__BIT_HASSTREAMER|G__BIT_ISPOINTER:
            if (!R__t)  return 1;
            (*dictSrcOut) << "            R__b.WriteObjectAny(" << R__t << "," << tcl << ");" << std::endl;
            break;
            
         case kBIT_ISSTRING:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            {TString R__str(" << R__t << ".c_str());" << std::endl
            << "             R__str.Streamer(R__b);};" << std::endl;
            break;
            
         case kBIT_ISSTRING|G__BIT_ISPOINTER:
            if (!R__t)  return 0;
            (*dictSrcOut) << "            {TString R__str(" << R__t << "->c_str());" << std::endl
            << "             R__str.Streamer(R__b);}" << std::endl;
            break;
            
         case G__BIT_ISPOINTER:
            if (!R__t)  return 1;
            (*dictSrcOut) << "            R__b.WriteObjectAny(" << R__t << "," << tcl <<");" << std::endl;
            break;
            
         default:
            if (!R__t)  return 1;
            (*dictSrcOut) << "            R__b.StreamObject((" << objType << "*)&" << R__t << "," << tcl << ");" << std::endl;
            break;
      }
   }
   return 0;
}

//______________________________________________________________________________
int ElementStreamer(const clang::NamedDecl &forcontext, const clang::QualType &qti, const char *R__t,int rwmode,const char *tcl=0)
{
   static const clang::CXXRecordDecl *TObject_decl = R__SlowClassSearch("TObject");
   enum {
      R__BIT_ISTOBJECT   = 0x10000000,
      R__BIT_HASSTREAMER = 0x20000000,
      kBIT_ISSTRING    = 0x40000000
   };

   const clang::Type &ti( * qti.getTypePtr() );
   string tiName;
   R__GetQualifiedName(tiName, clang::QualType(&ti,0), forcontext);
   
   string objType(ShortTypeName(tiName.c_str()));

   const clang::Type *rawtype = R__GetUnderlyingType(clang::QualType(&ti,0));
   string rawname;
   R__GetQualifiedName(rawname, clang::QualType(rawtype,0), forcontext);
   
   clang::CXXRecordDecl *cxxtype = rawtype->getAsCXXRecordDecl() ;
   int isStre = cxxtype && ClassInfo__HasMethod(cxxtype,"Streamer");
   int isTObj = cxxtype && (R__IsBase(cxxtype,TObject_decl) || rawname == "TObject");
 
   long kase = 0;   

   if (ti.isPointerType())           kase |= G__BIT_ISPOINTER;
   if (rawtype->isFundamentalType()) kase |= G__BIT_ISFUNDAMENTAL;
   if (rawtype->isEnumeralType())    kase |= G__BIT_ISENUM;


   if (isTObj)              kase |= R__BIT_ISTOBJECT;
   if (isStre)              kase |= R__BIT_HASSTREAMER;
   if (tiName == "string")  kase |= kBIT_ISSTRING;
   if (tiName == "string*") kase |= kBIT_ISSTRING;
   
   
   if (tcl == 0) {
      tcl = " internal error in rootcint ";
   }
   //    if (strcmp(objType,"string")==0) RStl::Instance().GenerateTClassFor( "string"  );

   if (rwmode == 0) {  //Read mode

      if (R__t) (*dictSrcOut) << "            " << tiName << " " << R__t << ";" << std::endl;
      switch (kase) {

      case G__BIT_ISFUNDAMENTAL:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            R__b >> " << R__t << ";" << std::endl;
         break;

      case G__BIT_ISPOINTER|R__BIT_ISTOBJECT|R__BIT_HASSTREAMER:
         if (!R__t)  return 1;
         (*dictSrcOut) << "            " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");"  << std::endl;
         break;

      case G__BIT_ISENUM:
         if (!R__t)  return 0;
         //             fprintf(fp, "            R__b >> (Int_t&)%s;\n",R__t);
         // On some platforms enums and not 'Int_t' and casting to a reference to Int_t
         // induces the silent creation of a temporary which is 'filled' __instead of__
         // the desired enum.  So we need to take it one step at a time.
         (*dictSrcOut) << "            Int_t readtemp;" << std::endl
                       << "            R__b >> readtemp;" << std::endl
                       << "            " << R__t << " = static_cast<" << tiName << ">(readtemp);" << std::endl;
         break;

      case R__BIT_HASSTREAMER:
      case R__BIT_HASSTREAMER|R__BIT_ISTOBJECT:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            " << R__t << ".Streamer(R__b);" << std::endl;
         break;

      case R__BIT_HASSTREAMER|G__BIT_ISPOINTER:
         if (!R__t)  return 1;
         //fprintf(fp, "            fprintf(stderr,\"info is %%p %%d\\n\",R__b.GetInfo(),R__b.GetInfo()?R__b.GetInfo()->GetOldVersion():-1);\n");
         (*dictSrcOut) << "            if (R__b.GetInfo() && R__b.GetInfo()->GetOldVersion()<=3) {" << std::endl;
         if (cxxtype && cxxtype->isAbstract()) {
            (*dictSrcOut) << "               R__ASSERT(0);// " << objType << " is abstract. We assume that older file could not be produced using this streaming method." << std::endl;
         } else {
            (*dictSrcOut) << "               " << R__t << " = new " << objType << ";" << std::endl
                          << "               " << R__t << "->Streamer(R__b);" << std::endl;
         }
         (*dictSrcOut) << "            } else {" << std::endl
                       << "               " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");" << std::endl
                       << "            }" << std::endl;
         break;

      case kBIT_ISSTRING:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            {TString R__str;" << std::endl
                       << "             R__str.Streamer(R__b);" << std::endl
                       << "             " << R__t << " = R__str.Data();}" << std::endl;
         break;

      case kBIT_ISSTRING|G__BIT_ISPOINTER:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            {TString R__str;"  << std::endl
                       << "             R__str.Streamer(R__b);" << std::endl
                       << "             " << R__t << " = new string(R__str.Data());}" << std::endl;
         break;

      case G__BIT_ISPOINTER:
         if (!R__t)  return 1;
         (*dictSrcOut) << "            " << R__t << " = (" << tiName << ")R__b.ReadObjectAny(" << tcl << ");" << std::endl;
         break;

      default:
         if (!R__t) return 1;
         (*dictSrcOut) << "            R__b.StreamObject(&" << R__t << "," << tcl << ");" << std::endl;
         break;
      }

   } else {     //Write case

      switch (kase) {

      case G__BIT_ISFUNDAMENTAL:
      case G__BIT_ISPOINTER|R__BIT_ISTOBJECT|R__BIT_HASSTREAMER:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            R__b << " << R__t << ";" << std::endl;
         break;

      case G__BIT_ISENUM:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            {  void *ptr_enum = (void*)&" << R__t << ";\n";
         (*dictSrcOut) << "               R__b >> *reinterpret_cast<Int_t*>(ptr_enum); }" << std::endl;
         break;

      case R__BIT_HASSTREAMER:
      case R__BIT_HASSTREAMER|R__BIT_ISTOBJECT:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            ((" << objType << "&)" << R__t << ").Streamer(R__b);" << std::endl;
         break;

      case R__BIT_HASSTREAMER|G__BIT_ISPOINTER:
         if (!R__t)  return 1;
         (*dictSrcOut) << "            R__b.WriteObjectAny(" << R__t << "," << tcl << ");" << std::endl;
         break;

      case kBIT_ISSTRING:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            {TString R__str(" << R__t << ".c_str());" << std::endl
                       << "             R__str.Streamer(R__b);};" << std::endl;
         break;

      case kBIT_ISSTRING|G__BIT_ISPOINTER:
         if (!R__t)  return 0;
         (*dictSrcOut) << "            {TString R__str(" << R__t << "->c_str());" << std::endl
                       << "             R__str.Streamer(R__b);}" << std::endl;
         break;

      case G__BIT_ISPOINTER:
         if (!R__t)  return 1;
         (*dictSrcOut) << "            R__b.WriteObjectAny(" << R__t << "," << tcl <<");" << std::endl;
         break;

      default:
         if (!R__t)  return 1;
         (*dictSrcOut) << "            R__b.StreamObject((" << objType << "*)&" << R__t << "," << tcl << ");" << std::endl;
         break;
      }
   }
   return 0;
}

//______________________________________________________________________________
int STLContainerStreamer(const clang::FieldDecl &m, int rwmode)
{
   // Create Streamer code for an STL container. Returns 1 if data member
   // was an STL container and if Streamer code has been created, 0 otherwise.

   int stltype = abs(IsSTLContainer(m));
   std::string mTypename;
   R__GetQualifiedName(mTypename, m.getType(),m);
   
   const clang::CXXRecordDecl* clxx = llvm::dyn_cast_or_null<clang::CXXRecordDecl>(R__GetUnderlyingRecordDecl(m.getType()));

   if (stltype!=0) {
      //        fprintf(stderr,"Add %s (%d) which is also %s\n",
      //                m.Type()->Name(), stltype, m.Type()->TrueName() );
      RStl::Instance().GenerateTClassFor(mTypename.c_str(), clxx );
   }
   if (stltype<=0) return 0;
   if (clxx->getTemplateSpecializationKind() == clang::TSK_Undeclared) return 0;
   
   const clang::ClassTemplateSpecializationDecl *tmplt_specialization = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl> (clxx);
   if (!tmplt_specialization) return 0;


   // string stlType( RStl::DropDefaultArg( m.Type()->Name() ) );
   //    string stlType( TClassEdit::ShortType(m.Type()->Name(),
   //                                          TClassEdit::kDropTrailStar|
   //                                          TClassEdit::kDropStlDefault) );
   string stlType( ShortTypeName(mTypename.c_str()) );
   string stlName;
   stlName = ShortTypeName(m.getName().data());

   string fulName1,fulName2;
   const char *tcl1=0,*tcl2=0;
   const clang::TemplateArgument &arg0( tmplt_specialization->getTemplateArgs().get(0) );
   clang::QualType ti = arg0.getAsType();

   if (ElementStreamer(m, ti, 0, rwmode)) {
      tcl1="R__tcl1";
      fulName1 = ti.getAsString(); // Should we be passing a context?
   }
   if (stltype==kMap || stltype==kMultiMap) {
      const clang::TemplateArgument &arg1( tmplt_specialization->getTemplateArgs().get(1) );
      clang::QualType tmplti = arg1.getAsType();
      if (ElementStreamer(m, tmplti, 0, rwmode)) {
         tcl2="R__tcl2";
         fulName2 = tmplti.getAsString(); // Should we be passing a context?
      }
   }

   int isArr = 0;
   int len = 1;
   int pa = 0;
   const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(m.getType().getTypePtr());
   if (arrayType) {
      isArr = 1;
      len =  R__GetFullArrayLength(arrayType);
      pa = 1;
      while (arrayType) {
         if (arrayType->getArrayElementTypeNoTypeQual()->isPointerType()) {
            pa = 3;
            break;
         }
         arrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual());
      }
   } else if (m.getType()->isPointerType()) {
      pa = 2;
   }
   if (rwmode == 0) {
      // create read code
      (*dictSrcOut) << "      {" << std::endl;
      if (isArr) {
         (*dictSrcOut) << "         for (Int_t R__l = 0; R__l < " << len << "; R__l++) {" << std::endl;
      }

      switch (pa) {
      case 0:         //No pointer && No array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl =  " << stlName.c_str() << ";" << std::endl;
         break;
      case 1:         //No pointer && array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl =  " << stlName.c_str() << "[R__l];" << std::endl;
         break;
      case 2:         //pointer && No array
         (*dictSrcOut) << "         delete *" << stlName.c_str() << ";"<< std::endl
                       << "         *" << stlName.c_str() << " = new " << stlType.c_str() << ";" << std::endl
                       << "         " << stlType.c_str() << " &R__stl = **" << stlName.c_str() << ";" << std::endl;
         break;
      case 3:         //pointer && array
         (*dictSrcOut) << "         delete " << stlName.c_str() << "[R__l];" << std::endl
                       << "         " << stlName.c_str() << "[R__l] = new " << stlType.c_str() << ";" << std::endl
                       << "         " << stlType.c_str() << " &R__stl = *" << stlName.c_str() << "[R__l];" << std::endl;
         break;
      }

      (*dictSrcOut) << "         R__stl.clear();" << std::endl;

      if (tcl1) {
         (*dictSrcOut) << "         TClass *R__tcl1 = TBuffer::GetClass(typeid(" << fulName1.c_str() << "));" << std::endl
                       << "         if (R__tcl1==0) {" << std::endl
                       << "            Error(\"" << stlName.c_str() << " streamer\",\"Missing the TClass object for "
                       << fulName1.c_str() << "!\");"  << std::endl
                       << "            return;" << std::endl
                       << "         }" << std::endl;
      }
      if (tcl2) {
         (*dictSrcOut) << "         TClass *R__tcl2 = TBuffer::GetClass(typeid(" << fulName2.c_str() << "));" << std::endl
                       << "         if (R__tcl2==0) {" << std::endl
                       << "            Error(\"" << stlName.c_str() << " streamer\",\"Missing the TClass object for "
                       << fulName2.c_str() <<"!\");" << std::endl
                       << "            return;" << std::endl
                       << "         }" << std::endl;
      }

      (*dictSrcOut) << "         int R__i, R__n;" << std::endl
                    << "         R__b >> R__n;" << std::endl;

      if (stltype==kVector) {
         (*dictSrcOut) << "         R__stl.reserve(R__n);" << std::endl;
      }
      (*dictSrcOut) << "         for (R__i = 0; R__i < R__n; R__i++) {" << std::endl;

      ElementStreamer(m, arg0.getAsType(), "R__t", rwmode, tcl1);
      if (stltype == kMap || stltype == kMultiMap) {     //Second Arg
         const clang::TemplateArgument &arg1( tmplt_specialization->getTemplateArgs().get(1) );
         ElementStreamer(m, arg1.getAsType(), "R__t2", rwmode, tcl2);
      }

      /* Need to go from
         type R__t;
         R__t.Stream;
         vec.push_back(R__t);
         to
         vec.push_back(type());
         R__t_p = &(vec.last());
         *R__t_p->Stream;

      */
      switch (stltype) {

      case kMap:
      case kMultiMap: {
         std::string keyName( ti.getAsString() );
         (*dictSrcOut) << "            typedef " << keyName << " Value_t;" << std::endl
                       << "            std::pair<Value_t const, " << tmplt_specialization->getTemplateArgs().get(1).getAsType().getAsString() << " > R__t3(R__t,R__t2);" << std::endl
                       << "            R__stl.insert(R__t3);" << std::endl;
         //fprintf(fp, "            R__stl.insert(%s::value_type(R__t,R__t2));\n",stlType.c_str());
         break;
      }
      case kSet:
      case kMultiSet:
         (*dictSrcOut) << "            R__stl.insert(R__t);" << std::endl;
         break;
      case kVector:
      case kList:
      case kDeque:
         (*dictSrcOut) << "            R__stl.push_back(R__t);" << std::endl;
         break;

      default:
         assert(0);
      }
      (*dictSrcOut) << "         }" << std::endl
                    << "      }" << std::endl;
      if (isArr) (*dictSrcOut) << "    }" << std::endl;

   } else {

      // create write code
      if (isArr) {
         (*dictSrcOut) << "         for (Int_t R__l = 0; R__l < " << len << "; R__l++) {" << std::endl;
      }
      (*dictSrcOut) << "      {" << std::endl;
      switch (pa) {
      case 0:         //No pointer && No array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl =  " << stlName.c_str() << ";" << std::endl;
         break;
      case 1:         //No pointer && array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl =  " << stlName.c_str() << "[R__l];" << std::endl;
         break;
      case 2:         //pointer && No array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl = **" << stlName.c_str() << ";" << std::endl;
         break;
      case 3:         //pointer && array
         (*dictSrcOut) << "         " << stlType.c_str() << " &R__stl = *" << stlName.c_str() << "[R__l];" << std::endl;
         break;
      }

      (*dictSrcOut) << "         int R__n=(&R__stl) ? int(R__stl.size()) : 0;" << std::endl
                    << "         R__b << R__n;" << std::endl
                    << "         if(R__n) {" << std::endl;

      if (tcl1) {
         (*dictSrcOut) << "         TClass *R__tcl1 = TBuffer::GetClass(typeid(" << fulName1.c_str() << "));" << std::endl
                       << "         if (R__tcl1==0) {" << std::endl
                       << "            Error(\"" << stlName.c_str() << " streamer\",\"Missing the TClass object for "
                       << fulName1.c_str() << "!\");" << std::endl
                       << "            return;" << std::endl
                       << "         }" << std::endl;
      }
      if (tcl2) {
         (*dictSrcOut) << "         TClass *R__tcl2 = TBuffer::GetClass(typeid(" << fulName2.c_str() << "));" << std::endl
                       << "         if (R__tcl2==0) {" << std::endl
                       << "            Error(\"" << stlName.c_str() << "streamer\",\"Missing the TClass object for " << fulName2.c_str() << "!\");" << std::endl
                       << "            return;" << std::endl
                       << "         }" << std::endl;
      }

      (*dictSrcOut) << "            " << stlType.c_str() << "::iterator R__k;" << std::endl
                    << "            for (R__k = R__stl.begin(); R__k != R__stl.end(); ++R__k) {" << std::endl;
      if (stltype == kMap || stltype == kMultiMap) {
         const clang::TemplateArgument &arg1( tmplt_specialization->getTemplateArgs().get(1) );
         clang::QualType tmplti = arg1.getAsType();
         ElementStreamer(m, ti, "((*R__k).first )",rwmode,tcl1);
         ElementStreamer(m, tmplti, "((*R__k).second)",rwmode,tcl2);
      } else {
         ElementStreamer(m, ti, "(*R__k)"         ,rwmode,tcl1);
      }

      (*dictSrcOut) << "            }" << std::endl
                    << "         }" << std::endl
                    << "      }" << std::endl;
      if (isArr) (*dictSrcOut) << "    }" << std::endl;
   }
   return 1;
}

//______________________________________________________________________________
int STLStringStreamer(const clang::FieldDecl &m, int rwmode)
{
   // Create Streamer code for a standard string object. Returns 1 if data
   // member was a standard string and if Streamer code has been created,
   // 0 otherwise.

   std::string mTypenameStr;
   R__GetQualifiedName(mTypenameStr, m.getType(),m);
   // Note: here we could to a direct type comparison!
   const char *mTypeName = ShortTypeName(mTypenameStr.c_str());
   if (!strcmp(mTypeName, "string")) {
      
      std::string fieldname =  m.getName().data();
      if (rwmode == 0) {
         // create read mode
         if (m.getType()->isConstantArrayType()) {
            if (m.getType().getTypePtr()->getArrayElementTypeNoTypeQual()->isPointerType()) {
               (*dictSrcOut) << "// Array of pointer to std::string are not supported (" << fieldname << "\n";
            } else {
               std::stringstream fullIdx;
               const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(m.getType().getTypePtr());
               int dim = 0;
               while (arrayType) {
                  (*dictSrcOut) << "      for (int R__i" << dim << "=0; R__i" << dim << "<"
                                << arrayType->getSize().getLimitedValue() << "; ++R__i" << dim << " )" << std::endl;
                  fullIdx << "[R__i" << dim << "]";
                  arrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual());
                  ++dim;
               }
               (*dictSrcOut) << "         { TString R__str; R__str.Streamer(R__b); "
                             << fieldname << fullIdx.str() << " = R__str.Data();}" << std::endl;
            }
         } else {
            (*dictSrcOut) << "      { TString R__str; R__str.Streamer(R__b); ";
            if (m.getType()->isPointerType())
               (*dictSrcOut) << "if (*" << fieldname << ") delete *" << fieldname << "; (*"
                             << fieldname << " = new string(R__str.Data())); }" << std::endl;
            else
               (*dictSrcOut) << fieldname << " = R__str.Data(); }" << std::endl;
         }
      } else {
         // create write mode
         if (m.getType()->isPointerType())
            (*dictSrcOut) << "      { TString R__str; if (*" << fieldname << ") R__str = (*"
                          << fieldname << ")->c_str(); R__str.Streamer(R__b);}" << std::endl;
         else if (m.getType()->isConstantArrayType()) {
            std::stringstream fullIdx;
            const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(m.getType().getTypePtr());
            int dim = 0;
            while (arrayType) {
               (*dictSrcOut) << "      for (int R__i" << dim << "=0; R__i" << dim << "<"
                             << arrayType->getSize().getLimitedValue() << "; ++R__i" << dim << " )" << std::endl;
               fullIdx << "[R__i" << dim << "]";
               arrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual());
               ++dim;
            }
            (*dictSrcOut) << "         { TString R__str(" << fieldname << fullIdx.str() << ".c_str()); R__str.Streamer(R__b);}" << std::endl;
         } else
            (*dictSrcOut) << "      { TString R__str = " << fieldname << ".c_str(); R__str.Streamer(R__b);}" << std::endl;
      }
      return 1;
   }
   return 0;
}

//______________________________________________________________________________
bool isPointerToPointer(const clang::FieldDecl &m)
{
   if (m.getType()->isPointerType()) {
      if (m.getType()->getPointeeType()->isPointerType()) {
         return true;
      }
   }
   return false;
}

//______________________________________________________________________________
void WriteArrayDimensions(const clang::QualType &type)
{
   // Write "[0]" for all but the 1st dimension.

   const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr());
   if (arrayType) {
      arrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual());
      while(arrayType) {
         (*dictSrcOut) << "[0]";
         arrayType = llvm::dyn_cast<clang::ConstantArrayType>(arrayType->getArrayElementTypeNoTypeQual());
      }
   }
}

//______________________________________________________________________________
void WriteClassFunctions(const clang::CXXRecordDecl *cl)
{
   // Write the code to set the class name and the initialization object.

   bool add_template_keyword = NeedTemplateKeyword(cl);

   string fullname;
   R__GetQualifiedName(fullname,*cl);

   string clsname = fullname;

   const clang::NamedDecl *nsdecl = llvm::dyn_cast<clang::NamedDecl>(cl->getEnclosingNamespaceContext());
   string nsname;
   if (nsdecl && nsdecl!=cl ) {
      R__GetQualifiedName(nsname,*nsdecl);
      clsname.erase (0, nsname.size() + 2);
   }

   int enclSpaceNesting = 0;
   if (!nsname.empty()) {
      G__ShadowMaker nestTempShadowMaker(*dictSrcOut, "");
      //G__ClassInfo clinfo(cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str());
      G__ClassInfo clinfo(R__GetQualifiedName(*cl).c_str());
      enclSpaceNesting = nestTempShadowMaker.WriteNamespaceHeader(clinfo);
   }

   (*dictSrcOut) << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "atomic_TClass_ptr " << clsname.c_str() << "::fgIsA(0);  // static to hold class pointer" << std::endl
                 << std::endl

                 << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "const char *" << clsname.c_str() << "::Class_Name()" << std::endl << "{" << std::endl
                 << "   return \"" << fullname.c_str() << "\";"  << std::endl <<"}" << std::endl << std::endl;

   (*dictSrcOut) << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "const char *" << clsname.c_str() << "::ImplFileName()"  << std::endl << "{" << std::endl
                 << "   return ::ROOT::GenerateInitInstanceLocal((const ::" << fullname.c_str()
                 << "*)0x0)->GetImplFileName();" << std::endl << "}" << std::endl << std::endl

                 << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) <<"template <> ";
   (*dictSrcOut) << "int " << clsname.c_str() << "::ImplFileLine()" << std::endl << "{" << std::endl
                 << "   return ::ROOT::GenerateInitInstanceLocal((const ::" << fullname.c_str()
                 << "*)0x0)->GetImplFileLine();" << std::endl << "}" << std::endl << std::endl

                 << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "void " << clsname.c_str() << "::Dictionary()" << std::endl << "{" << std::endl
                 << "   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::" << fullname.c_str()
                 << "*)0x0)->GetClass();" << std::endl
                 << "}" << std::endl << std::endl

                 << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "TClass *" << clsname.c_str() << "::Class()" << std::endl << "{" << std::endl;
   (*dictSrcOut) << "   if (!fgIsA) { R__LOCKGUARD2(gCINTMutex); if(!fgIsA) {fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::";
   (*dictSrcOut) << fullname.c_str() << "*)0x0)->GetClass();} }" << std::endl
                 << "   return fgIsA;" << std::endl
                 << "}" << std::endl << std::endl;

   while (enclSpaceNesting) {
      (*dictSrcOut) << "} // namespace " << nsname << std::endl;
      --enclSpaceNesting;
   }
}
//______________________________________________________________________________
void WriteClassInit(const RScanner::AnnotatedRecordDecl &cl_input)
{
   // Write the code to initialize the class name and the initialization object.

   const clang::CXXRecordDecl* cl = llvm::dyn_cast<clang::CXXRecordDecl>(cl_input.GetRecordDecl());

   if (cl==0) {
      return;
   }

   // We need to go back to CINT to preserve functionality.
   G__ClassInfo clinfo( cl_input.GetRequestedName()[0] ? cl_input.GetRequestedName() : R__GetQualifiedName(cl_input).c_str() );

   // coverity[fun_call_w_exception] - that's just fine.
   string classname = GetLong64_Name( RStl::DropDefaultArg( cl_input.GetRequestedName()[0] ? cl_input.GetRequestedName() : R__GetQualifiedName(cl_input).c_str() ) );
   string mappedname = G__map_cpp_name((char*)classname.c_str());
   string csymbol = classname;
   string args;

   if ( ! TClassEdit::IsStdClass( classname.c_str() ) ) {

      // Prefix the full class name with '::' except for the STL
      // containers and std::string.  This is to request the
      // real class instead of the class in the namespace ROOT::Shadow
      csymbol.insert(0,"::");
   }

   int stl = TClassEdit::IsSTLCont(classname.c_str());
   bool bset = TClassEdit::IsSTLBitset(classname.c_str());

   (*dictSrcOut) << "namespace ROOT {" << std::endl
   << "   void " << mappedname.c_str() << "_ShowMembers(void *obj, TMemberInspector &R__insp);"
   << std::endl;

   if (!ClassInfo__HasMethod(cl,"Dictionary") || R__IsTemplate(cl))
      (*dictSrcOut) << "   static void " << mappedname.c_str() << "_Dictionary();" << std::endl;

   if (HasDefaultConstructor(cl,&args)) {
      (*dictSrcOut) << "   static void *new_" << mappedname.c_str() << "(void *p = 0);" << std::endl;
      if (args.size()==0 && NeedDestructor(cl))
         (*dictSrcOut) << "   static void *newArray_" << mappedname.c_str()
         << "(Long_t size, void *p);" << std::endl;
   }
   if (NeedDestructor(cl)) {
      (*dictSrcOut) << "   static void delete_" << mappedname.c_str() << "(void *p);" << std::endl
      << "   static void deleteArray_" << mappedname.c_str() << "(void *p);" << std::endl
      << "   static void destruct_" << mappedname.c_str() << "(void *p);" << std::endl;
   }
   if (HasDirectoryAutoAdd(cl)) {
      (*dictSrcOut)<< "   static void directoryAutoAdd_" << mappedname.c_str() << "(void *obj, TDirectory *dir);" << std::endl;
   }
   if (HasCustomStreamerMemberFunction(cl_input)) {
      (*dictSrcOut)<< "   static void streamer_" << mappedname.c_str() << "(TBuffer &buf, void *obj);" << std::endl;
   }
   if (HasNewMerge(cl) || HasOldMerge(cl)) {
      (*dictSrcOut)<< "   static Long64_t merge_" << mappedname.c_str() << "(void *obj, TCollection *coll,TFileMergeInfo *info);" << std::endl;
   }
   if (HasResetAfterMerge(cl)) {
      (*dictSrcOut)<< "   static void reset_" << mappedname.c_str() << "(void *obj, TFileMergeInfo *info);" << std::endl;
   }

   //--------------------------------------------------------------------------
   // Check if we have any schema evolution rules for this class
   //--------------------------------------------------------------------------
   SchemaRuleClassMap_t::iterator rulesIt1 = G__ReadRules.find( R__GetQualifiedName(*cl).c_str() );
   SchemaRuleClassMap_t::iterator rulesIt2 = G__ReadRawRules.find( R__GetQualifiedName(*cl).c_str() );

   MembersTypeMap_t nameTypeMap;
   // Note Falling back on CINT for this ....
   CreateNameTypeMap( clinfo, nameTypeMap );

   //--------------------------------------------------------------------------
   // Process the read rules
   //--------------------------------------------------------------------------
   if( rulesIt1 != G__ReadRules.end() ) {
      int i = 0;
      (*dictSrcOut) << std::endl;
      (*dictSrcOut) << "   // Schema evolution read functions" << std::endl;
      std::list<SchemaRuleMap_t>::iterator rIt = rulesIt1->second.begin();
      while( rIt != rulesIt1->second.end() ) {

         //--------------------------------------------------------------------
         // Check if the rules refer to valid data members
         //--------------------------------------------------------------------
         if( !HasValidDataMembers( *rIt, nameTypeMap ) ) {
            rIt = rulesIt1->second.erase(rIt);
            continue;
         }

         //---------------------------------------------------------------------
         // Write the conversion function if necassary
         //---------------------------------------------------------------------
         if( rIt->find( "code" ) != rIt->end() ) {
            WriteReadRuleFunc( *rIt, i++, mappedname, nameTypeMap, *dictSrcOut );
         }
         ++rIt;
      }
   }

   //--------------------------------------------------------------------------
   // Process the read raw rules
   //--------------------------------------------------------------------------
   if( rulesIt2 != G__ReadRawRules.end() ) {
      int i = 0;
      (*dictSrcOut) << std::endl;
      (*dictSrcOut) << "   // Schema evolution read raw functions" << std::endl;
      std::list<SchemaRuleMap_t>::iterator rIt = rulesIt2->second.begin();
      while( rIt != rulesIt2->second.end() ) {

         //--------------------------------------------------------------------
         // Check if the rules refer to valid data members
         //--------------------------------------------------------------------
         if( !HasValidDataMembers( *rIt, nameTypeMap ) ) {
            rIt = rulesIt2->second.erase(rIt);
            continue;
         }

         //---------------------------------------------------------------------
         // Write the conversion function
         //---------------------------------------------------------------------
         if( rIt->find( "code" ) == rIt->end() )
            continue;

         WriteReadRawRuleFunc( *rIt, i++, mappedname, nameTypeMap, *dictSrcOut );
         ++rIt;
      }
   }

   (*dictSrcOut) << std::endl << "   // Function generating the singleton type initializer" << std::endl;

#if 0
   fprintf(fp, "#if defined R__NAMESPACE_TEMPLATE_IMP_BUG\n");
   fprintf(fp, "   template <> ::ROOT::TGenericClassInfo *::ROOT::GenerateInitInstanceLocal< %s >(const %s*)\n   {\n",
           cl.Fullname(), cl.Fullname() );
   fprintf(fp, "#else\n");
   fprintf(fp, "   template <> ::ROOT::TGenericClassInfo *GenerateInitInstanceLocal< %s >(const %s*)\n   {\n",
           classname.c_str(), classname.c_str() );
   fprintf(fp, "#endif\n");
#endif

   (*dictSrcOut) << "   static TGenericClassInfo *GenerateInitInstanceLocal(const " << csymbol.c_str() << "*)" << std::endl << "   {" << std::endl;



   // Note this is falling back on CINT :(
   if (NeedShadowClass(clinfo)) {
      (*dictSrcOut) << "      // Make sure the shadow class has the right sizeof" << std::endl;
      if (G__ShadowMaker::IsStdPair(clinfo)) {
         // Some compilers don't recognize ::pair even after a 'using namespace std;'
         // and there is no risk of confusion since it is a template.
         //fprintf(fp, "      R__ASSERT(sizeof(%s)", classname.c_str() );
      } else {
         std::string clfullname;
         shadowMaker->GetFullShadowName(clinfo, clfullname);
         (*dictSrcOut) << "      R__ASSERT(sizeof(" << csymbol.c_str() << ")"
         << " == sizeof(" << clfullname.c_str() << "));" << std::endl;
      }
   }



   (*dictSrcOut) << "      " << csymbol.c_str() << " *ptr = 0;" << std::endl;

   //fprintf(fp, "      static ::ROOT::ClassInfo< %s > \n",classname.c_str());
   if (ClassInfo__HasMethod(cl,"IsA") ) {
      (*dictSrcOut) << "      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< "
      << csymbol.c_str() << " >(0);" << std::endl;
   }
   else {
      (*dictSrcOut) << "      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid("
      << csymbol.c_str() << "),0);" << std::endl;
   }
   (*dictSrcOut) << "      static ::ROOT::TGenericClassInfo " << std::endl

   << "         instance(\"" << classname.c_str() << "\", ";

   if (ClassInfo__HasMethod(cl,"Class_Version")) {
      (*dictSrcOut) << csymbol.c_str() << "::Class_Version(), ";
   } else if (bset) {
      (*dictSrcOut) << "2, "; // bitset 'version number'
   } else if (stl) {
      (*dictSrcOut) << "-2, "; // "::TStreamerInfo::Class_Version(), ";
   } else if( clinfo.RootFlag() & G__HASVERSION ) {
      (*dictSrcOut) << clinfo.Version() << ", ";
   } else { // if (cl.RootFlag() & G__USEBYTECOUNT ) {

      // Need to find out if the operator>> is actually defined for this class.
      G__ClassInfo gcl;
      long offset;
      const char *versionFunc = "GetClassVersion";
      int ncha = strlen(classname.c_str())+strlen(versionFunc)+5;
      char *funcname= new char[ncha];
      snprintf(funcname,ncha,"%s<%s >",versionFunc,classname.c_str());
      ncha = strlen(classname.c_str())+ 10 ;
      char *proto = new char[ncha];
      snprintf(proto,ncha,"%s*",classname.c_str());
      G__MethodInfo methodinfo = gcl.GetMethod(versionFunc,proto,&offset);
      delete [] funcname;
      delete [] proto;

      if (methodinfo.IsValid() &&
          //          methodinfo.ifunc()->para_p_tagtable[methodinfo.Index()][0] == cl.Tagnum() &&
          strstr(methodinfo.FileName(),"Rtypes.h") == 0) {

         // GetClassVersion was defined in the header file.
         //fprintf(fp, "GetClassVersion((%s *)0x0), ",classname.c_str());
         (*dictSrcOut) << "GetClassVersion< " << classname.c_str() << " >(), ";
      }
      //static char temporary[1024];
      //sprintf(temporary,"GetClassVersion<%s>( (%s *) 0x0 )",classname.c_str(),classname.c_str());
      //fprintf(stderr,"DEBUG: %s has value %d\n",classname.c_str(),(int)G__int(G__calc(temporary)));
   }

   std::string filename = R__GetFileName(cl_input);
   if (filename.length() > 0) {
      for (unsigned int i=0; i<filename.length(); i++) {
         if (filename[i]=='\\') filename[i]='/';
      }
   }
   (*dictSrcOut) << "\"" << filename << "\", " << R__GetLineNumber(cl_input) << "," << std::endl
                 << "                  typeid(" << csymbol.c_str() << "), DefineBehavior(ptr, ptr)," << std::endl
                 << "                  ";
   //   fprintf(fp, "                  (::ROOT::ClassInfo< %s >::ShowMembersFunc_t)&::ROOT::ShowMembers,%d);\n", classname.c_str(),cl.RootFlag());
   if (!NeedShadowClass(clinfo)) {
      if (!ClassInfo__HasMethod(cl,"ShowMembers")) (*dictSrcOut) << "0, ";
   } else {
      if (!ClassInfo__HasMethod(cl,"ShowMembers"))
         (*dictSrcOut) << "&" << mappedname.c_str() << "_ShowMembers, ";
   }

   if (ClassInfo__HasMethod(cl,"Dictionary") && !R__IsTemplate(cl)) {
      (*dictSrcOut) << "&" << csymbol.c_str() << "::Dictionary, ";
   } else {
      (*dictSrcOut) << "&" << mappedname.c_str() << "_Dictionary, ";
   }

   (*dictSrcOut) << "isa_proxy, " << clinfo.RootFlag() << "," << std::endl
                 << "                  sizeof(" << csymbol.c_str() << ") );" << std::endl;
   if (HasDefaultConstructor(cl,&args)) {
      (*dictSrcOut) << "      instance.SetNew(&new_" << mappedname.c_str() << ");" << std::endl;
      if (args.size()==0 && NeedDestructor(cl))
         (*dictSrcOut) << "      instance.SetNewArray(&newArray_" << mappedname.c_str() << ");" << std::endl;
   }
   if (NeedDestructor(cl)) {
      (*dictSrcOut) << "      instance.SetDelete(&delete_" << mappedname.c_str() << ");" << std::endl
                    << "      instance.SetDeleteArray(&deleteArray_" << mappedname.c_str() << ");" << std::endl
                    << "      instance.SetDestructor(&destruct_" << mappedname.c_str() << ");" << std::endl;
   }
   if (HasDirectoryAutoAdd(cl)) {
      (*dictSrcOut) << "      instance.SetDirectoryAutoAdd(&directoryAutoAdd_" << mappedname.c_str() << ");" << std::endl;
   }
   if (HasCustomStreamerMemberFunction(cl_input)) {
      // We have a custom member function streamer or an older (not StreamerInfo based) automatic streamer.
      (*dictSrcOut) << "      instance.SetStreamerFunc(&streamer_" << mappedname.c_str() << ");" << std::endl;
   }
   if (HasNewMerge(cl) || HasOldMerge(cl)) {
      (*dictSrcOut) << "      instance.SetMerge(&merge_" << mappedname.c_str() << ");" << std::endl;
   }
   if (HasResetAfterMerge(cl)) {
      (*dictSrcOut) << "      instance.SetResetAfterMerge(&reset_" << mappedname.c_str() << ");" << std::endl;      
   }
   if (bset) {
      (*dictSrcOut) << "      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::"
                    << "Pushback" << "<TStdBitsetHelper< " << classname.c_str() << " > >()));" << std::endl;

      // (*dictSrcOut) << "      instance.SetStreamer(::ROOT::std_bitset_helper" << strchr(csymbol.c_str(),'<') << "::Streamer);\n";
      gNeedCollectionProxy = true;
   } else if (stl != 0 && ((stl>0 && stl<8) || (stl<0 && stl>-8)) )  {
      int idx = classname.find("<");
      int stlType = (idx!=(int)std::string::npos) ? TClassEdit::STLKind(classname.substr(0,idx).c_str()) : 0;
      const char* methodTCP=0;
      switch(stlType)  {
         case TClassEdit::kVector:
         case TClassEdit::kList:
         case TClassEdit::kDeque:
            methodTCP="Pushback";
            break;
         case TClassEdit::kMap:
         case TClassEdit::kMultiMap:
            methodTCP="MapInsert";
            break;
         case TClassEdit::kSet:
         case TClassEdit::kMultiSet:
            methodTCP="Insert";
            break;
      }
      (*dictSrcOut) << "      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::"
      << methodTCP << "< " << classname.c_str() << " >()));" << std::endl;

      gNeedCollectionProxy = true;
   }

   //---------------------------------------------------------------------------
   // Pass the schema evolution rules to TGenericClassInfo
   //---------------------------------------------------------------------------
   if( (rulesIt1 != G__ReadRules.end() && rulesIt1->second.size()>0) || (rulesIt2 != G__ReadRawRules.end()  && rulesIt2->second.size()>0) ) {
      (*dictSrcOut) << std::endl << "      ROOT::TSchemaHelper* rule;" << std::endl;
   }

   if( rulesIt1 != G__ReadRules.end() ) {
      (*dictSrcOut) << std::endl;
      (*dictSrcOut) << "      // the io read rules" << std::endl;
      (*dictSrcOut) << "      std::vector<ROOT::TSchemaHelper> readrules(";
      (*dictSrcOut) << rulesIt1->second.size() << ");" << std::endl;
      WriteSchemaList( rulesIt1->second, "readrules", *dictSrcOut );
      (*dictSrcOut) << "      instance.SetReadRules( readrules );" << std::endl;
   }

   if( rulesIt2 != G__ReadRawRules.end() ) {
      (*dictSrcOut) << std::endl;
      (*dictSrcOut) << "      // the io read raw rules" << std::endl;
      (*dictSrcOut) << "      std::vector<ROOT::TSchemaHelper> readrawrules(";
      (*dictSrcOut) << rulesIt2->second.size() << ");" << std::endl;
      WriteSchemaList( rulesIt2->second, "readrawrules", *dictSrcOut );
      (*dictSrcOut) << "      instance.SetReadRawRules( readrawrules );" << std::endl;
   }

   (*dictSrcOut) << "      return &instance;"  << std::endl
   << "   }" << std::endl;

   if (!stl && !bset && !IsTemplateDouble32(clinfo) && !IsTemplateFloat16(clinfo)) {
      // The GenerateInitInstance for STL are not unique and should not be externally accessible
      (*dictSrcOut) << "   TGenericClassInfo *GenerateInitInstance(const " << csymbol.c_str() << "*)" << std::endl
      << "   {\n      return GenerateInitInstanceLocal((" <<  csymbol.c_str() << "*)0);\n   }"
      << std::endl;
   }

   (*dictSrcOut) << "   // Static variable to force the class initialization" << std::endl;
   // must be one long line otherwise R__UseDummy does not work


   (*dictSrcOut)
   << "   static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstanceLocal((const "
   << csymbol.c_str() << "*)0x0); R__UseDummy(_R__UNIQUE_(Init));" << std::endl;

   if (!ClassInfo__HasMethod(cl,"Dictionary") || R__IsTemplate(cl)) {
      (*dictSrcOut) <<  std::endl << "   // Dictionary for non-ClassDef classes" << std::endl
      << "   static void " << mappedname.c_str() << "_Dictionary() {" << std::endl;
      (*dictSrcOut) << "      ::ROOT::GenerateInitInstanceLocal((const " << csymbol.c_str();
      (*dictSrcOut) << "*)0x0)->GetClass();" << std::endl
      << "   }" << std::endl << std::endl;
   }

   (*dictSrcOut) << "} // end of namespace ROOT" << std::endl << std::endl;

}

//______________________________________________________________________________
void WriteNamespaceInit(const clang::NamespaceDecl *cl)
{
   // Write the code to initialize the namespace name and the initialization object.

   // coverity[fun_call_w_exception] - that's just fine.
   string classname = GetLong64_Name( RStl::DropDefaultArg( R__GetQualifiedName(*cl).c_str() ) );
   string mappedname = G__map_cpp_name((char*)classname.c_str());

   int nesting = 0;
   // We should probably unwind the namespace to properly nest it.
   if (classname!="ROOT") {
      string right = classname;
      int pos = right.find(":");
      if (pos==0) {
         right = right.substr(2);
         pos = right.find(":");
      }
      while(pos>=0) {
         string left = right.substr(0,pos);
         right = right.substr(pos+2);
         pos = right.find(":");
         ++nesting;
         (*dictSrcOut) << "namespace " << left << " {" << std::endl;
      }

      ++nesting;
      (*dictSrcOut) << "namespace " << right << " {" << std::endl;
   }

   (*dictSrcOut) << "   namespace ROOT {" << std::endl;

#if !defined(R__SGI) && !defined(R__AIX)
   (*dictSrcOut) << "      inline ::ROOT::TGenericClassInfo *GenerateInitInstance();" << std::endl;
#endif

   if (!Namespace__HasMethod(cl,"Dictionary"))
      (*dictSrcOut) << "      static void " << mappedname.c_str() << "_Dictionary();" << std::endl;
   (*dictSrcOut) << std::endl

   << "      // Function generating the singleton type initializer" << std::endl

#if !defined(R__SGI) && !defined(R__AIX)
   << "      inline ::ROOT::TGenericClassInfo *GenerateInitInstance()" << std::endl
   << "      {" << std::endl
#else
   << "      ::ROOT::TGenericClassInfo *GenerateInitInstance()" << std::endl
   << "      {" << std::endl
#endif

   << "         static ::ROOT::TGenericClassInfo " << std::endl

   << "            instance(\"" << classname.c_str() << "\", ";

   if (Namespace__HasMethod(cl,"Class_Version")) {
      (*dictSrcOut) << "::" << classname.c_str() << "::Class_Version(), ";
   } else {
#if defined(NEED_FUNC_LOOKUP)
      // Need to find out if the operator>> is actually defined for this class.
      G__ClassInfo gcl;
      long offset;
      const char *versionFunc = "GetClassVersion";
      int ncha = strlen(classname.c_str())+strlen(versionFunc)+5;
      char *funcname= new char[ncha];
      snprintf(funcname,ncha,"%s<%s >",versionFunc,classname.c_str());
      ncha = strlen(classname.c_str())+ 10 ;
      char *proto = new char[ncha];
      snprintf(proto,ncha,"%s*",classname.c_str());
      G__MethodInfo methodinfo = gcl.GetMethod(versionFunc,proto,&offset);
      delete [] funcname;
      delete [] proto;

      if (methodinfo.IsValid() &&
          strstr(methodinfo.FileName(),"Rtypes.h") == 0) {
         (*dictSrcOut) << "GetClassVersion< " << classname.c_str() << " >(), ";
      } else {
         (*dictSrcOut) << "0 /*version*/, ";
      }
#else
      (*dictSrcOut) << "0 /*version*/, ";
#endif
   }

   std::string filename = R__GetFileName(cl);
   for (unsigned int i=0; i<filename.length(); i++) {
      if (filename[i]=='\\') filename[i]='/';
   }
   (*dictSrcOut) << "\"" << filename << "\", " << R__GetLineNumber(cl) << "," << std::endl
                 << "                     ::ROOT::DefineBehavior((void*)0,(void*)0)," << std::endl
                 << "                     ";

   if (Namespace__HasMethod(cl,"Dictionary")) {
      (*dictSrcOut) << "&::" << classname.c_str() << "::Dictionary, ";
   } else {
      (*dictSrcOut) << "&" << mappedname.c_str() << "_Dictionary, ";
   }

   (*dictSrcOut) << 0 << ");" << std::endl

   << "         return &instance;" << std::endl
   << "      }" << std::endl
   << "      // Insure that the inline function is _not_ optimized away by the compiler\n"
   << "      ::ROOT::TGenericClassInfo *(*_R__UNIQUE_(InitFunctionKeeper))() = &GenerateInitInstance;  " << std::endl
   << "      // Static variable to force the class initialization" << std::endl
   // must be one long line otherwise R__UseDummy does not work
   << "      static ::ROOT::TGenericClassInfo *_R__UNIQUE_(Init) = GenerateInitInstance();"
   << " R__UseDummy(_R__UNIQUE_(Init));" << std::endl;

   if (!Namespace__HasMethod(cl,"Dictionary")) {
      (*dictSrcOut) <<  std::endl << "      // Dictionary for non-ClassDef classes" << std::endl
      << "      static void " << mappedname.c_str() << "_Dictionary() {" << std::endl
      << "         GenerateInitInstance()->GetClass();" << std::endl
      << "      }" << std::endl << std::endl;
   }

   (*dictSrcOut) << "   }" << std::endl;
   while(nesting--) {
      (*dictSrcOut) << "}" << std::endl;
   }
   (*dictSrcOut) <<  std::endl;
}

//______________________________________________________________________________
const char *ShortTypeName(const char *typeDesc)
{
   // Return the absolute type of typeDesc.
   // E.g.: typeDesc = "class TNamed**", returns "TNamed".
   // we remove * and const keywords. (we do not want to remove & ).
   // You need to use the result immediately before it is being overwritten.

   static char t[4096];
   static const char* constwd = "const ";
   static const char* constwdend = "const";

   const char *s;
   char *p=t;
   int lev=0;
   for (s=typeDesc;*s;s++) {
      if (*s=='<') lev++;
      if (*s=='>') lev--;
      if (lev==0 && *s=='*') continue;
      if (lev==0 && (strncmp(constwd,s,strlen(constwd))==0
                     ||strcmp(constwdend,s)==0 ) ) {
         s+=strlen(constwd)-1; // -1 because the loop adds 1
         continue;
      }
      if (lev==0 && *s==' ' && *(s+1)!='*') { p = t; continue;}
      if (p - t > (long)sizeof(t)) {
         printf("ERROR (rootcint): type name too long for StortTypeName: %s\n",
                typeDesc);
         p[0] = 0;
         return t;
      }
      *p++ = *s;
   }
   p[0]=0;

   return t;
}

//______________________________________________________________________________
std::string ShortTypeName(const clang::FieldDecl &m)
{
   // Return the absolute type of typeDesc.
   // E.g.: typeDesc = "class TNamed**", returns "TNamed".
   // we remove * and const keywords. (we do not want to remove & ).
   // You need to use the result immediately before it is being overwritten.
   
   const clang::Type *rawtype = m.getType().getTypePtr();
   
   //Get to the 'raw' type.
   clang::QualType pointee;
   while ( rawtype->isPointerType() && ((pointee = rawtype->getPointeeType()) , pointee.getTypePtrOrNull()) && pointee.getTypePtr() != rawtype)
   {
      rawtype = pointee.getTypePtr();
   }

   std::string result;
   R__GetQualifiedName(result, clang::QualType(rawtype,0), m);
   return result;
}

const clang::FieldDecl *R__GetDataMemberFromAll(const clang::CXXRecordDecl &cl, const char *what)
{
   // Return a data member name 'what' in the class described by 'cl' if any.

   for(clang::RecordDecl::field_iterator field_iter = cl.field_begin(), end = cl.field_end();
       field_iter != end;
       ++field_iter)
   {
      if (field_iter->getNameAsString() == what) {
         return &*field_iter;
      }
   }
   return 0;
   
}

bool CXXRecordDecl__FindOrdinaryMember ( const clang::CXXBaseSpecifier *Specifier,
                                           clang::CXXBasePath &Path,
                                           void *Name
                                           )
{
   clang::RecordDecl *BaseRecord = Specifier->getType()->getAs<clang::RecordType>()->getDecl();
   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(BaseRecord);
   if (clxx == 0) return false;
   
   return 0 != R__GetDataMemberFromAll(*clxx,(const char*)Name);
}


const clang::FieldDecl *R__GetDataMemberFromAllParents(const clang::CXXRecordDecl &cl, const char *what)
{
   // Return a data member name 'what' in any of the base classes of the class described by 'cl' if any.

   clang::CXXBasePaths Paths;
   Paths.setOrigin(const_cast<clang::CXXRecordDecl*>(&cl));
   cl.lookupInBases(&CXXRecordDecl__FindOrdinaryMember,
                    (void*) what,
                    Paths);
   return 0;
}

// ValidArrayIndex return a static string (so use it or copy it immediatly, do not
// call GrabIndex twice in the same expression) containing the size of the
// array data member.
// In case of error, or if the size is not specified, GrabIndex returns 0.
// If errnum is not null, *errnum updated with the error number:
//   Cint::G__DataMemberInfo::G__VALID     : valid array index
//   Cint::G__DataMemberInfo::G__NOT_INT   : array index is not an int
//   Cint::G__DataMemberInfo::G__NOT_DEF   : index not defined before array 
//                                          (this IS an error for streaming to disk)
//   Cint::G__DataMemberInfo::G__IS_PRIVATE: index exist in a parent class but is private
//   Cint::G__DataMemberInfo::G__UNKNOWN   : index is not known
// If errstr is not null, *errstr is updated with the address of a static
//   string containing the part of the index with is invalid.

enum R__DataMemberInfo__ValidArrayIndex_error_code { VALID, NOT_INT, NOT_DEF, IS_PRIVATE, UNKNOWN };
const char* R__DataMemberInfo__ValidArrayIndex(const clang::FieldDecl &m, int *errnum, char **errstr) 
{
   const char* title = R__GetComment( m );
   
   // Let's see if the user provided us with some information
   // with the format: //[dimension] this is the dim of the array
   // dimension can be an arithmetical expression containing, literal integer,
   // the operator *,+ and - and data member of integral type.  In addition the
   // data members used for the size of the array need to be defined prior to
   // the array.
   
   if (errnum) *errnum = VALID;
   
   if ((strncmp(title, "[", 1)!=0) ||
       (strstr(title,"]")     ==0)  ) return 0;
   
   G__FastAllocString working(G__INFO_TITLELEN);
   static char indexvar[G__INFO_TITLELEN];
   strncpy(indexvar, title + 1, sizeof(indexvar) - 1);
   strstr(indexvar,"]")[0] = '\0';
   
   // now we should have indexvar=dimension
   // Let's see if this is legal.
   // which means a combination of data member and digit separated by '*','+','-'
   // First we remove white spaces.
   unsigned int i,j;
   size_t indexvarlen = strlen(indexvar);
   for ( i=0,j=0; i<=indexvarlen; i++) {
      if (!isspace(indexvar[i])) {
         working.Set(j++, indexvar[i]);
      }
   };
   
   // Now we go through all indentifiers
   const char * tokenlist = "*+-";
   char *current = working;
   current = strtok(current,tokenlist);
   
   while (current!=0) {
      // Check the token
      if (isdigit(current[0])) {
         for(i=0;i<strlen(current);i++) {
            if (!isdigit(current[0])) {
               // Error we only access integer.
               //NOTE: *** Need to print an error;
               //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is not an interger\n",
               //        member.MemberOf()->Name(), member.Name(), current);
               if (errstr) *errstr = current;
               if (errnum) *errnum = NOT_INT;
               return 0;
            }
         }
      } else { // current token is not a digit
         // first let's see if it is a data member:
         int found = 0;
         const clang::CXXRecordDecl *parent_clxx = llvm::dyn_cast<clang::CXXRecordDecl>(m.getParent());
         const clang::FieldDecl *index1 = R__GetDataMemberFromAll(*parent_clxx, current );
         if ( index1 ) {
            if ( R__IsInt(index1) ) {
               found = 1;
               // Let's see if it has already been written down in the
               // Streamer.
               // Let's see if we already wrote it down in the
               // streamer.
               for(clang::RecordDecl::field_iterator field_iter = parent_clxx->field_begin(), end = parent_clxx->field_end();
                   field_iter != end;
                   ++field_iter)
               {
                  if ( field_iter->getNameAsString() == m.getNameAsString() ) {
                     // we reached the current data member before
                     // reaching the index so we have not written it yet!
                     //NOTE: *** Need to print an error;
                     //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) has not been defined before the array \n",
                     //        member.MemberOf()->Name(), member.Name(), current);
                     if (errstr) *errstr = current;
                     if (errnum) *errnum = NOT_DEF;
                     return 0;
                  }
                  if ( field_iter->getNameAsString() == index1->getNameAsString() ) {
                     break;
                  }
               } // end of while (m_local.Next())
            } else {
               //NOTE: *** Need to print an error;
               //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is not int \n",
               //        member.MemberOf()->Name(), member.Name(), current);
               if (errstr) *errstr = current;
               if (errnum) *errnum = NOT_INT;
               return 0;
            }
         } else {
            // There is no variable by this name in this class, let see
            // the base classes!:
            index1 = R__GetDataMemberFromAllParents( *parent_clxx, current );
            if ( index1 ) {
               if ( R__IsInt(index1) ) {
                  found = 1;
               } else {
                  // We found a data member but it is the wrong type
                  //NOTE: *** Need to print an error;
                  //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is not int \n",
                  //  member.MemberOf()->Name(), member.Name(), current);
                  if (errnum) *errnum = NOT_INT;
                  if (errstr) *errstr = current;
                  //NOTE: *** Need to print an error;
                  //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is not int \n",
                  //  member.MemberOf()->Name(), member.Name(), current);
                  if (errnum) *errnum = NOT_INT;
                  if (errstr) *errstr = current;
                  return 0;
               }
               if ( found && (index1->getAccess() == clang::AS_private) ) {
                  //NOTE: *** Need to print an error;
                  //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is a private member of %s \n",
                  if (errstr) *errstr = current;
                  if (errnum) *errnum = IS_PRIVATE;
                  return 0;
               }
            }
            if (!found) {
               //NOTE: *** Need to print an error;
               //fprintf(stderr,"*** Datamember %s::%s: size of array (%s) is not known \n",
               //        member.MemberOf()->Name(), member.Name(), indexvar);
               if (errstr) *errstr = indexvar;
               if (errnum) *errnum = UNKNOWN;
               return 0;
            } // end of if not found
         } // end of if is a data member of the class
      } // end of if isdigit
      
      current = strtok(0,tokenlist);
   } // end of while loop on tokens      
   
   return indexvar;
   
}
   
//______________________________________________________________________________
const char *GrabIndex(const clang::FieldDecl &member, int printError)
{
   // GrabIndex returns a static string (so use it or copy it immediatly, do not
   // call GrabIndex twice in the same expression) containing the size of the
   // array data member.
   // In case of error, or if the size is not specified, GrabIndex returns 0.

   int error;
   char *where = 0;

   const char *index = R__DataMemberInfo__ValidArrayIndex(member,&error, &where);
   if (index==0 && printError) {
      const char *errorstring;
      switch (error) {
      case G__DataMemberInfo::NOT_INT:
         errorstring = "is not an integer";
         break;
      case G__DataMemberInfo::NOT_DEF:
         errorstring = "has not been defined before the array";
         break;
      case G__DataMemberInfo::IS_PRIVATE:
         errorstring = "is a private member of a parent class";
         break;
      case G__DataMemberInfo::UNKNOWN:
         errorstring = "is not known";
         break;
      default:
         errorstring = "UNKNOWN ERROR!!!!";
      }

      if (where==0) {
         Error(0, "*** Datamember %s::%s: no size indication!\n",
               member.getParent()->getName().data(), member.getName().data());
      } else {
         Error(0,"*** Datamember %s::%s: size of array (%s) %s!\n",
               member.getParent()->getName().data(), member.getName().data(), where, errorstring);
      }
   }
   return index;
}

//______________________________________________________________________________
void WriteStreamer(const RScanner::AnnotatedRecordDecl &cl)
{
   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());
   if (clxx == 0) return;
   
   bool add_template_keyword = NeedTemplateKeyword(clxx);
   
   string fullname;
   R__GetQualifiedName(fullname,cl);
   
   string clsname = fullname;
   
   const clang::NamedDecl *nsdecl = llvm::dyn_cast<clang::NamedDecl>(clxx->getEnclosingNamespaceContext());
   string nsname;
   if (nsdecl && nsdecl!=cl ) {
      R__GetQualifiedName(nsname,*nsdecl);
      clsname.erase (0, nsname.size() + 2);
   }
   
   G__ClassInfo clinfo(R__GetQualifiedName(cl).c_str());

   int enclSpaceNesting = 0;
   if (!nsname.empty()) {
      G__ShadowMaker nestTempShadowMaker(*dictSrcOut, "");
      enclSpaceNesting = nestTempShadowMaker.WriteNamespaceHeader(clinfo);
   }
   
   (*dictSrcOut) << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "void " << clsname << "::Streamer(TBuffer &R__b)"  << std::endl << "{" << std::endl
                 << "   // Stream an object of class " << fullname << "." << std::endl << std::endl;

   // In case of VersionID<=0 write dummy streamer only calling
   // its base class Streamer(s). If no base class(es) let Streamer
   // print error message, i.e. this Streamer should never have been called.
   int version = GetClassVersion(clxx);
   if (version <= 0) {
      // We also need to look at the base classes.
      int basestreamer = 0;
      for(clang::CXXRecordDecl::base_class_const_iterator iter = clxx->bases_begin(), end = clxx->bases_end();
          iter != end;
          ++iter)
      {
         if (ClassInfo__HasMethod(iter->getType()->getAsCXXRecordDecl (),"Streamer")) {
            string base_fullname;
            R__GetQualifiedName(base_fullname,* iter->getType()->getAsCXXRecordDecl ());

            if (strstr(base_fullname.c_str(),"::")) {
               // there is a namespace involved, trigger MS VC bug workaround
               (*dictSrcOut) << "   //This works around a msvc bug and should be harmless on other platforms" << std::endl
                             << "   typedef " << base_fullname << " baseClass" << basestreamer << ";" << std::endl
                             << "   baseClass" << basestreamer << "::Streamer(R__b);" << std::endl;
            }
            else {
               (*dictSrcOut) << "   " << base_fullname << "::Streamer(R__b);" << std::endl;
            }
            basestreamer++;
         }
      }
      if (!basestreamer) {
         (*dictSrcOut) << "   ::Error(\"" << fullname << "::Streamer\", \"version id <=0 in ClassDef,"
            " dummy Streamer() called\"); if (R__b.IsReading()) { }" << std::endl;
      }
      (*dictSrcOut) << "}" << std::endl << std::endl;
      while (enclSpaceNesting) {
         (*dictSrcOut) << "} // namespace " << nsname.c_str() << std::endl;
         --enclSpaceNesting;
      }
      return;
   }

   // loop twice: first time write reading code, second time writing code
   string classname = fullname;
   if (strstr(fullname.c_str(),"::")) {
      // there is a namespace involved, trigger MS VC bug workaround
      (*dictSrcOut) << "   //This works around a msvc bug and should be harmless on other platforms" << std::endl
                    << "   typedef ::" << fullname << " thisClass;" << std::endl;
      classname = "thisClass";
   }
   for (int i = 0; i < 2; i++) {

      int decli = 0;

      if (i == 0) {
         (*dictSrcOut) << "   UInt_t R__s, R__c;" << std::endl;
         (*dictSrcOut) << "   if (R__b.IsReading()) {" << std::endl;
         (*dictSrcOut) << "      Version_t R__v = R__b.ReadVersion(&R__s, &R__c); if (R__v) { }" << std::endl;
      } else {
         (*dictSrcOut) << "      R__b.CheckByteCount(R__s, R__c, " << classname.c_str() << "::IsA());" << std::endl;
         (*dictSrcOut) << "   } else {" << std::endl;
         (*dictSrcOut) << "      R__c = R__b.WriteVersion(" << classname.c_str() << "::IsA(), kTRUE);" << std::endl;
      }

      // Stream base class(es) when they have the Streamer() method
      int base=0;
      for(clang::CXXRecordDecl::base_class_const_iterator iter = clxx->bases_begin(), end = clxx->bases_end();
          iter != end;
          ++iter)
      {
         if (ClassInfo__HasMethod(iter->getType()->getAsCXXRecordDecl (),"Streamer")) {
            string base_fullname;
            R__GetQualifiedName(base_fullname,* iter->getType()->getAsCXXRecordDecl ());
            
            if (strstr(base_fullname.c_str(),"::")) {
               // there is a namespace involved, trigger MS VC bug workaround
               (*dictSrcOut) << "      //This works around a msvc bug and should be harmless on other platforms" << std::endl
                             << "      typedef " << base_fullname << " baseClass" << base << ";" << std::endl
                             << "      baseClass" << base << "::Streamer(R__b);" << std::endl;
               ++base;
            }
            else {
               (*dictSrcOut) << "      " << base_fullname << "::Streamer(R__b);" << std::endl;
            }
         }
      }
      // Stream data members
      // Loop over the non static data member.
      for(clang::RecordDecl::field_iterator field_iter = clxx->field_begin(), end = clxx->field_end();
          field_iter != end;
          ++field_iter)
      {
         const char *comment = R__GetComment( *field_iter );

         clang::QualType type = field_iter->getType();
         std::string type_name = type.getAsString(clxx->getASTContext().getPrintingPolicy());

         const clang::Type *underling_type = R__GetUnderlyingType(type);
         
         // we skip:
         //  - static members
         //  - members with an ! as first character in the title (comment) field
         //  - the member G__virtualinfo inserted by the CINT RTTI system

         //special case for Float16_t
         int isFloat16=0;
         if (strstr(type_name.c_str(),"Float16_t")) isFloat16=1;

         //special case for Double32_t
         int isDouble32=0;
         if (strstr(type_name.c_str(),"Double32_t")) isDouble32=1;

         // No need to test for static, there are not in this 'list.
         //   !(m.Property() & G__BIT_ISSTATIC)
         // No need to test for CINT artifiical members:
         //   strcmp(m.Name(), "G__virtualinfo")
         if (strncmp(comment, "!", 1)) {

            // fundamental type: short, int, long, etc....
            if (underling_type->isFundamentalType() || underling_type->isEnumeralType()) {
               if (type.getTypePtr()->isConstantArrayType() &&
                   type.getTypePtr()->getArrayElementTypeNoTypeQual()->isPointerType() ) 
//               if (m.Property() & G__BIT_ISARRAY &&
//                   m.Property() & G__BIT_ISPOINTER) 
               {
                  const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr());
                  int s = R__GetFullArrayLength(arrayType);

                  if (!decli) {
                     (*dictSrcOut) << "      int R__i;" << std::endl;
                     decli = 1;
                  }
                  (*dictSrcOut) << "      for (R__i = 0; R__i < " << s << "; R__i++)" << std::endl;
                  if (i == 0) {
                     Error(0, "*** Datamember %s::%s: array of pointers to fundamental type (need manual intervention)\n", fullname.c_str(), field_iter->getName().data());
                     (*dictSrcOut) << "         ;//R__b.ReadArray(" << field_iter->getName().data() << ");" << std::endl;
                  } else {
                     (*dictSrcOut) << "         ;//R__b.WriteArray(" << field_iter->getName().data() << ", __COUNTER__);" << std::endl;
                  }
               } else if (type.getTypePtr()->isPointerType()) {
                  const char *indexvar = GrabIndex(*field_iter, i==0);
                  if (indexvar==0) {
                     if (i == 0) {
                        Error(0,"*** Datamember %s::%s: pointer to fundamental type (need manual intervention)\n", fullname.c_str(), field_iter->getName().data());
                        (*dictSrcOut) << "      //R__b.ReadArray(" << field_iter->getName().data() << ");" << std::endl;
                     } else {
                        (*dictSrcOut) << "      //R__b.WriteArray(" << field_iter->getName().data() << ", __COUNTER__);" << std::endl;
                     }
                  } else {
                     if (i == 0) {
                        (*dictSrcOut) << "      delete [] " << field_iter->getName().data() << ";" << std::endl
                                      << "      " << GetNonConstMemberName(*field_iter) << " = new "
                                      << ShortTypeName(*field_iter) << "[" << indexvar << "];" << std::endl;
                        if (isFloat16) {
                           (*dictSrcOut) << "      R__b.ReadFastArrayFloat16(" <<  GetNonConstMemberName(*field_iter)
                                         << "," << indexvar << ");" << std::endl;
                        } else if (isDouble32) {
                           (*dictSrcOut) << "      R__b.ReadFastArrayDouble32(" <<  GetNonConstMemberName(*field_iter)
                                         << "," << indexvar << ");" << std::endl;
                        } else {
                           (*dictSrcOut) << "      R__b.ReadFastArray(" << GetNonConstMemberName(*field_iter)
                                         << "," << indexvar << ");" << std::endl;
                        }
                     } else {
                        if (isFloat16) {
                           (*dictSrcOut) << "      R__b.WriteFastArrayFloat16("
                                         << field_iter->getName().data() << "," << indexvar << ");" << std::endl;
                        } else if (isDouble32) {
                           (*dictSrcOut) << "      R__b.WriteFastArrayDouble32("
                                         << field_iter->getName().data() << "," << indexvar << ");" << std::endl;
                        } else {
                           (*dictSrcOut) << "      R__b.WriteFastArray("
                                         << field_iter->getName().data() << "," << indexvar << ");" << std::endl;
                        }
                     }
                  }
               } else if (type.getTypePtr()->isArrayType()) {
                  if (i == 0) {
                     if (type.getTypePtr()->getArrayElementTypeNoTypeQual()->isArrayType()) { // if (m.ArrayDim() > 1) {
                        if ( underling_type->isEnumeralType() )
                           (*dictSrcOut) << "      R__b.ReadStaticArray((Int_t*)" << field_iter->getName().data() << ");" << std::endl;
                        else {
                           if (isFloat16) {
                              (*dictSrcOut) << "      R__b.ReadStaticArrayFloat16((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ");" << std::endl;
                           } else if (isDouble32) {
                              (*dictSrcOut) << "      R__b.ReadStaticArrayDouble32((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ");" << std::endl;
                           } else {
                              (*dictSrcOut) << "      R__b.ReadStaticArray((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ");" << std::endl;
                           }
                        }
                     } else {
                        if ( underling_type->isEnumeralType() ) {
                           (*dictSrcOut) << "      R__b.ReadStaticArray((Int_t*)" << field_iter->getName().data() << ");" << std::endl;
                        } else {
                           if (isFloat16) {
                              (*dictSrcOut) << "      R__b.ReadStaticArrayFloat16(" << field_iter->getName().data() << ");" << std::endl;
                           } else if (isDouble32) {
                              (*dictSrcOut) << "      R__b.ReadStaticArrayDouble32(" << field_iter->getName().data() << ");" << std::endl;
                           } else {
                              (*dictSrcOut) << "      R__b.ReadStaticArray((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ");" << std::endl;
                           }
                        }
                     }
                  } else {
                     const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr());
                     int s = R__GetFullArrayLength(arrayType);

                     if (type.getTypePtr()->getArrayElementTypeNoTypeQual()->isArrayType()) {// if (m.ArrayDim() > 1) {
                        if ( underling_type->isEnumeralType() )
                           (*dictSrcOut) << "      R__b.WriteArray((Int_t*)" << field_iter->getName().data() << ", "
                                         << s << ");" << std::endl;
                        else
                           if (isFloat16) {
                              (*dictSrcOut) << "      R__b.WriteArrayFloat16((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           } else if (isDouble32) {
                              (*dictSrcOut) << "      R__b.WriteArrayDouble32((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           } else {
                              (*dictSrcOut) << "      R__b.WriteArray((" << R__TrueName(*field_iter)
                                            << "*)" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           }
                     } else {
                        if ( underling_type->isEnumeralType() )
                           (*dictSrcOut) << "      R__b.WriteArray((Int_t*)" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                        else
                           if (isFloat16) {
                              (*dictSrcOut) << "      R__b.WriteArrayFloat16(" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           } else if (isDouble32) {
                              (*dictSrcOut) << "      R__b.WriteArrayDouble32(" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           } else {
                              (*dictSrcOut) << "      R__b.WriteArray(" << field_iter->getName().data() << ", " << s << ");" << std::endl;
                           }
                     }
                  }
               } else if ( underling_type->isEnumeralType() ) {
                  if (i == 0) {
                     (*dictSrcOut) << "      void *ptr_" << field_iter->getName().data() << " = (void*)&" << field_iter->getName().data() << ";\n";
                     (*dictSrcOut) << "      R__b >> *reinterpret_cast<Int_t*>(ptr_" << field_iter->getName().data() << ");" << std::endl;
                  } else
                     (*dictSrcOut) << "      R__b << (Int_t)" << field_iter->getName().data() << ";" << std::endl;
               } else {
                  if (isFloat16) {
                     if (i == 0)
                        (*dictSrcOut) << "      {float R_Dummy; R__b >> R_Dummy; " << GetNonConstMemberName(*field_iter)
                                      << "=Float16_t(R_Dummy);}" << std::endl;
                     else
                        (*dictSrcOut) << "      R__b << float(" << GetNonConstMemberName(*field_iter) << ");" << std::endl;
                  } else if (isDouble32) {
                     if (i == 0)
                        (*dictSrcOut) << "      {float R_Dummy; R__b >> R_Dummy; " << GetNonConstMemberName(*field_iter)
                                      << "=Double32_t(R_Dummy);}" << std::endl;
                     else
                        (*dictSrcOut) << "      R__b << float(" << GetNonConstMemberName(*field_iter) << ");" << std::endl;
                  } else {
                     if (i == 0)
                        (*dictSrcOut) << "      R__b >> " << GetNonConstMemberName(*field_iter) << ";" << std::endl;
                     else
                        (*dictSrcOut) << "      R__b << " << GetNonConstMemberName(*field_iter) << ";" << std::endl;
                  }
               }
            } else {
               // we have an object...

               // check if object is a standard string
               if (STLStringStreamer(*field_iter, i))
                  continue;

               // check if object is an STL container
               if (STLContainerStreamer(*field_iter, i))
                  continue;

               // handle any other type of objects
               if (type.getTypePtr()->isConstantArrayType() &&
                   type.getTypePtr()->getArrayElementTypeNoTypeQual()->isPointerType()) 
               {
                  const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr());
                  int s = R__GetFullArrayLength(arrayType);

                  if (!decli) {
                     (*dictSrcOut) << "      int R__i;" << std::endl;
                     decli = 1;
                  }
                  (*dictSrcOut) << "      for (R__i = 0; R__i < " << s << "; R__i++)" << std::endl;
                  if (i == 0)
                     (*dictSrcOut) << "         R__b >> " << GetNonConstMemberName(*field_iter);
                  else {
                     if (R__IsBase(*field_iter,"TObject") && R__IsBase(*field_iter,"TArray"))
                        (*dictSrcOut) << "         R__b << (TObject*)" << field_iter->getName().data();
                     else
                        (*dictSrcOut) << "         R__b << " << GetNonConstMemberName(*field_iter);
                  }
                  WriteArrayDimensions(field_iter->getType());
                  (*dictSrcOut) << "[R__i];" << std::endl;
               } else if (type.getTypePtr()->isPointerType()) {
                  // This is always good. However, in case of a pointer
                  // to an object that is guarenteed to be there and not
                  // being referenced by other objects we could use
                  //     xx->Streamer(b);
                  // Optimize this with control statement in title.
                  if (isPointerToPointer(*field_iter)) {
                     if (i == 0) {
                        Error(0, "*** Datamember %s::%s: pointer to pointer (need manual intervention)\n", fullname.c_str(), field_iter->getName().data());
                        (*dictSrcOut) << "      //R__b.ReadArray(" << field_iter->getName().data() << ");" << std::endl;
                     } else {
                        (*dictSrcOut) << "      //R__b.WriteArray(" << field_iter->getName().data() << ", __COUNTER__);";
                     }
                  } else {
                     if (R__GetQualifiedName(*R__GetUnderlyingType(field_iter->getType()),*field_iter) == "TClonesArray") {
                        (*dictSrcOut) << "      " << field_iter->getName().data() << "->Streamer(R__b);" << std::endl;
                     } else {
                        if (i == 0) {
                           // The following:
                           //    if (strncmp(m.Title(),"->",2) != 0) fprintf(fp, "      delete %s;\n", GetNonConstMemberName(*field_iter).c_str());
                           // could be used to prevent a memory leak since the next statement could possibly create a new object.
                           // In the TStreamerInfo based I/O we made the previous statement conditional on TStreamerInfo::CanDelete
                           // to allow the user to prevent some inadvisable deletions.  So we should be offering this flexibility
                           // here to and should not (technically) rely on TStreamerInfo for it, so for now we leave it as is.
                           // Note that the leak should happen from here only if the object is stored in an unsplit object
                           // and either the user request an old branch or the streamer has been customized.
                           (*dictSrcOut) << "      R__b >> " << GetNonConstMemberName(*field_iter) << ";" << std::endl;
                        } else {
                           if (R__IsBase(*field_iter,"TObject") && R__IsBase(*field_iter,"TArray"))
                              (*dictSrcOut) << "      R__b << (TObject*)" << field_iter->getName().data() << ";" << std::endl;
                           else
                              (*dictSrcOut) << "      R__b << " << GetNonConstMemberName(*field_iter) << ";" << std::endl;
                        }
                     }
                  }
               } else if (type.getTypePtr()->isArrayType()) {
                  const clang::ConstantArrayType *arrayType = llvm::dyn_cast<clang::ConstantArrayType>(type.getTypePtr());
                  int s = R__GetFullArrayLength(arrayType);

                  if (!decli) {
                     (*dictSrcOut) << "      int R__i;" << std::endl;
                     decli = 1;
                  }
                  (*dictSrcOut) << "      for (R__i = 0; R__i < " << s << "; R__i++)" << std::endl;
                  std::string mTypeNameStr;
                  R__GetQualifiedName(mTypeNameStr,field_iter->getType(),*field_iter);
                  const char *mTypeName = mTypeNameStr.c_str();
                  const char *constwd = "const ";
                  if (strncmp(constwd,mTypeName,strlen(constwd))==0) {
                     mTypeName += strlen(constwd);
                     (*dictSrcOut) << "         const_cast< " << mTypeName << " &>(" << field_iter->getName().data();
                     WriteArrayDimensions(field_iter->getType());
                     (*dictSrcOut) << "[R__i]).Streamer(R__b);" << std::endl;
                  } else {
                     (*dictSrcOut) << "         " << GetNonConstMemberName(*field_iter);
                     WriteArrayDimensions(field_iter->getType());
                     (*dictSrcOut) << "[R__i].Streamer(R__b);" << std::endl;
                  }
               } else {
                  if (ClassInfo__HasMethod(R__GetUnderlyingRecordDecl(field_iter->getType()),"Streamer")) 
                     (*dictSrcOut) << "      " << GetNonConstMemberName(*field_iter) << ".Streamer(R__b);" << std::endl;
                  else {
                     (*dictSrcOut) << "      R__b.StreamObject(&(" << field_iter->getName().data() << "),typeid("
                                   << field_iter->getName().data() << "));" << std::endl;               //R__t.Streamer(R__b);\n");
                     //VP                     if (i == 0)
                     //VP                        Error(0, "*** Datamember %s::%s: object has no Streamer() method (need manual intervention)\n",
                     //VP                                  fullname, field_iter->getName().data());
                     //VP                     fprintf(fp, "      //%s.Streamer(R__b);\n", m.Name());
                  }
               }
            }
         }
      }
   }
   (*dictSrcOut) << "      R__b.SetByteCount(R__c, kTRUE);" << std::endl;
   (*dictSrcOut) << "   }" << std::endl
                 << "}" << std::endl << std::endl;

   while (enclSpaceNesting) {
      (*dictSrcOut) << "} // namespace " << nsname.c_str() << std::endl;
      --enclSpaceNesting;
   }
}

//______________________________________________________________________________
void WriteAutoStreamer(const RScanner::AnnotatedRecordDecl &cl)
{

   // Write Streamer() method suitable for automatic schema evolution.

   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());
   if (clxx == 0) return;
   
   bool add_template_keyword = NeedTemplateKeyword(clxx);
   
   // We also need to look at the base classes.
   for(clang::CXXRecordDecl::base_class_const_iterator iter = clxx->bases_begin(), end = clxx->bases_end();
       iter != end;
       ++iter)
   {
      int k = IsSTLContainer(*iter);
      if (k!=0) {
         RStl::Instance().GenerateTClassFor( cl.GetRequestedName(), iter->getType()->getAsCXXRecordDecl () );
      }
   }
   
   string fullname;
   R__GetQualifiedName(fullname,cl);
   
   string clsname = fullname;

   const clang::NamedDecl *nsdecl = llvm::dyn_cast<clang::NamedDecl>(clxx->getEnclosingNamespaceContext());
   string nsname;
   if (nsdecl && nsdecl!=cl ) {
      R__GetQualifiedName(nsname,*nsdecl);
      clsname.erase (0, nsname.size() + 2);
   }
   
   int enclSpaceNesting = 0;
   if (!nsname.empty()) {
      G__ShadowMaker nestTempShadowMaker(*dictSrcOut, "");
      G__ClassInfo clinfo(R__GetQualifiedName(cl).c_str());
      enclSpaceNesting = nestTempShadowMaker.WriteNamespaceHeader(clinfo);
   }

   (*dictSrcOut) << "//_______________________________________"
                 << "_______________________________________" << std::endl;
   if (add_template_keyword) (*dictSrcOut) << "template <> ";
   (*dictSrcOut) << "void " << clsname << "::Streamer(TBuffer &R__b)" << std::endl
                 << "{" << std::endl
                 << "   // Stream an object of class " << fullname << "." << std::endl << std::endl
                 << "   if (R__b.IsReading()) {" << std::endl
                 << "      R__b.ReadClassBuffer(" << fullname << "::Class(),this);" << std::endl
                 << "   } else {" << std::endl
                 << "      R__b.WriteClassBuffer(" << fullname << "::Class(),this);" << std::endl
                 << "   }" << std::endl
                 << "}" << std::endl << std::endl;

   while (enclSpaceNesting) {
      (*dictSrcOut) << "} // namespace " << nsname << std::endl;
      --enclSpaceNesting;
   }
}

//______________________________________________________________________________
void WritePointersSTL(const RScanner::AnnotatedRecordDecl &cl)
{
   // Write interface function for STL members

   string a;
   string clName(G__map_cpp_name(R__GetFileName(cl.GetRecordDecl()).data()));
   int version = GetClassVersion(cl.GetRecordDecl());
   if (version == 0) return;
   if (version < 0 && !(cl.RequestStreamerInfo()) ) return;


   const clang::CXXRecordDecl *clxx = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());
   if (clxx == 0) return;

   // We also need to look at the base classes.
   for(clang::CXXRecordDecl::base_class_const_iterator iter = clxx->bases_begin(), end = clxx->bases_end();
       iter != end;
       ++iter)
   {
      int k = IsSTLContainer(*iter);
      if (k!=0) {
         RStl::Instance().GenerateTClassFor( cl.GetRequestedName(), iter->getType()->getAsCXXRecordDecl () );
      }
   }

   // Loop over the non static data member.
   for(clang::RecordDecl::field_iterator field_iter = clxx->field_begin(), end = clxx->field_end();
       field_iter != end;
       ++field_iter)
   {
      const char *comment = R__GetComment( *field_iter );

      int pCounter = 0;
      clang::QualType type = field_iter->getType();
      std::string type_name = type.getAsString(clxx->getASTContext().getPrintingPolicy());
//      type->dump();
//      fprintf(stderr,"\n");
      if (type->isPointerType()) {
         const char *leftb = strchr(comment,'[');
         if (leftb) {
            const char *rightb = strchr(leftb,']');
            if (rightb) {
               pCounter++;
               a = type_name;
               if (strstr(type_name.c_str(),"**")) pCounter++;
               char *astar = (char*)strchr(a.c_str(),'*');
               if (!astar) {
                  Error(0, "Expected '*' in type name '%s' of member '%s'\n", a.c_str(), field_iter->getName().data());
               } else {
                  *astar = 0;
               }
            }
         }
      }

      //member is a string
      {
         const char*shortTypeName = ShortTypeName(type_name.c_str());
         if (!strcmp(shortTypeName, "string")) {
            continue;
         }
      }

      if (!IsStreamableObject(*field_iter)) continue;

      int k = IsSTLContainer( *field_iter );
      if (k!=0) {
         //          fprintf(stderr,"Add %s which is also",m.Type()->Name());
         //          fprintf(stderr," %s\n",R__TrueName(*field_iter) );
         RStl::Instance().GenerateTClassFor( "", R__SlowClassSearch(type_name.c_str()) );
      }      
   }

}


//______________________________________________________________________________
void WriteBodyShowMembers(G__ClassInfo& cl, bool outside)
{
   //#define R__SHOWMEMBERS_IN_TCLING
#ifdef R__SHOWMEMBERS_IN_TCLING
   std::string getClass;
   if (cl.HasMethod("IsA") && !outside) {
      getClass = csymbol + "::IsA()";
   } else {
      getClass = "::ROOT::GenerateInitInstanceLocal((const ";
      getClass += csymbol + "*)0x0)->GetClass()";
   }
   (*dictSrcOut) << "      ((TCintWithCling*)gInterpreter)->InspectMembers(R__insp, obj, "
      << getClass << ");" << std::endl;
#else
   string csymbol = cl.Fullname();
   if ( ! TClassEdit::IsStdClass( csymbol.c_str() ) ) {

      // Prefix the full class name with '::' except for the STL
      // containers and std::string.  This is to request the
      // real class instead of the class in the namespace ROOT::Shadow
      csymbol.insert(0,"::");
   }

   const char *prefix = "";

   (*dictSrcOut) << "      // Inspect the data members of an object of class " << cl.Fullname() << "." << std::endl;

   std::string clfullname;
   shadowMaker->GetFullShadowName(cl, clfullname);
   if (outside) {
      (*dictSrcOut) << "      typedef " << clfullname.c_str() << " ShadowClass;" << std::endl
                    << "      ShadowClass *sobj = (ShadowClass*)obj;" << std::endl
                    << "      if (sobj) { } // Dummy usage just in case there is no datamember." << std::endl << std::endl;
      prefix = "sobj->";
   }

   if (cl.HasMethod("IsA") && !outside) {
#ifdef  WIN32
      // This is to work around a bad msvc C++ bug.
      // This code would work in the general case, but why bother....and
      // we want to remember to eventually remove it ...

      if (strstr(csymbol.c_str(),"::")) {
         // there is a namespace involved, trigger MS VC bug workaround
         (*dictSrcOut) << "      typedef " << csymbol.c_str() << " msvc_bug_workaround;" << std::endl
                       << "      TClass *R__cl = msvc_bug_workaround::IsA();" << std::endl;
      } else
         (*dictSrcOut) << "      TClass *R__cl = " << csymbol.c_str() << "::IsA();" << std::endl;
#else
      (*dictSrcOut) << "      TClass *R__cl = " << csymbol.c_str() << "::IsA();" << std::endl;
#endif
   } else {
      (*dictSrcOut) << "      TClass *R__cl  = ::ROOT::GenerateInitInstanceLocal((const " << csymbol.c_str() << "*)0x0)->GetClass();" << std::endl;
   }
   (*dictSrcOut) << "      if (R__cl || R__insp.IsA()) { }" << std::endl;

   // Inspect data members
   G__DataMemberInfo m(cl);
   char cdim[1024];
   string cvar;
   string clName(G__map_cpp_name((char *)cl.Fullname()));
   string fun;
   int version = GetClassVersion(cl);
   int clflag = 1;
   if (version == 0 || cl.RootFlag() == 0) clflag = 0;
   if (version < 0 && !(cl.RootFlag() & G__USEBYTECOUNT) ) clflag = 0;

   while (m.Next()) {

      // we skip:
      //  - static members
      //  - the member G__virtualinfo inserted by the CINT RTTI system

      fun = string("R__") + clName + "_" + m.Name(); // sprintf(fun,"R__%s_%s",clName,m.Name());
      if (!(m.Property() & G__BIT_ISSTATIC) &&
          strcmp(m.Name(), "G__virtualinfo")) {

         // fundamental type: short, int, long, etc....
         if (((m.Type())->Property() & G__BIT_ISFUNDAMENTAL) ||
             ((m.Type())->Property() & G__BIT_ISENUM)) {
            if (m.Property() & G__BIT_ISARRAY &&
                m.Property() & G__BIT_ISPOINTER) {
               cvar = '*';
               cvar += m.Name();
               for (int dim = 0; dim < m.ArrayDim(); dim++) {
                  snprintf(cdim,1024, "[%ld]", m.MaxIndex(dim));
                  cvar += cdim;
               }
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << cvar << "\", &"
                             << prefix << m.Name() << ");" << std::endl;
            } else if (m.Property() & G__BIT_ISPOINTER) {
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"*" << m.Name() << "\", &"
                             << prefix << m.Name() << ");" << std::endl;
            } else if (m.Property() & G__BIT_ISARRAY) {
               cvar = m.Name();
               bool vardim = false;
               for (int dim = 0; dim < m.ArrayDim(); dim++) {
                  int maxInd = m.MaxIndex(dim);
                  if (maxInd < 0) {
                     strlcpy(cdim,"[]",1024);
                     vardim = true;
                  } else {
                     snprintf(cdim, 1024,"[%d]", maxInd);
                  }
                  cvar += cdim;
               }
               if (vardim) {
                  (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << cvar << "\", &"
                                << prefix << m.Name() << ");" << std::endl;
               } else {
                  (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << cvar << "\", "
                                << prefix << m.Name() << ");" << std::endl;
               }

            } else {
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << m.Name() << "\", &"
                             << prefix << m.Name() << ");" << std::endl;
            }
         } else {
            // we have an object

            if (m.Property() & G__BIT_ISARRAY &&
                m.Property() & G__BIT_ISPOINTER) {
               cvar = '*';
               cvar += m.Name();
               for (int dim = 0; dim < m.ArrayDim(); dim++) {
                  snprintf(cdim,1024, "[%ld]", m.MaxIndex(dim));
                  cvar += cdim;
               }
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << cvar << "\", &"
                             << prefix << m.Name() << ");" << std::endl;
            } else if (m.Property() & G__BIT_ISPOINTER) {
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"*" << m.Name() << "\", &" << prefix << m.Name() << ");" << std::endl;
            } else if (m.Property() & G__BIT_ISARRAY) {
               cvar = m.Name();
               for (int dim = 0; dim < m.ArrayDim(); dim++) {
                  snprintf(cdim,1024, "[%ld]", m.MaxIndex(dim));
                  cvar += cdim;
               }
               (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << cvar << "\", "
                             << prefix << m.Name() << ");" << std::endl;
            } else if (m.Property() & G__BIT_ISREFERENCE) {
               // For reference we do not know what do not ... let's do nothing (hopefully the referenced objects is saved somewhere else!

            } else {
               if ((m.Type())->HasMethod("ShowMembers")) {
                  (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << m.Name() << "\", &"
                                << prefix << m.Name() << ");" << std::endl;
                  (*dictSrcOut) << "      R__insp.InspectMember(" << GetNonConstMemberName(m,prefix)
                                << ", \"" << m.Name() << ".\");"  << std::endl;
               } else {
                  // NOTE: something to be added here!
                  (*dictSrcOut) << "      R__insp.Inspect(R__cl, R__insp.GetParent(), \"" << m.Name()
                                << "\", (void*)&" << prefix << m.Name() << ");" << std::endl;
                  /* if (can call ShowStreamer) */

                  string compareName(clName);
                  compareName += "::";

                  if (strlen(m.Type()->Name()) &&
                      compareName != m.Type()->Name() ) {
                     // Filter out the unamed type from with a the class.

                     string typeWithDefaultStlName( RStl::DropDefaultArg(m.Type()->Name()) );
                     //TClassEdit::ShortType(m.Type()->Name(),TClassEdit::kRemoveDefaultAlloc) );
                     string typeName( GetLong64_Name( m.Type()->Name() ) );

                     (*dictSrcOut) << "      R__insp.InspectMember(\"" << typeName << "\", (void*)&"
                                   << prefix << m.Name() << ", \""<< m.Name() << ".\", "
                                   << (!strncmp(m.Title(), "!", 1)?"true":"false")
                                   <<  ");" << std::endl;
                  }
               }
            }
         }
      }
   }

   // Write ShowMembers for base class(es) when they have the ShowMember() method
   G__BaseClassInfo b(cl);

   int base = 0;
   while (b.Next()) {
      base++;
      if (b.HasMethod("ShowMembers")) {
         if (outside) {
            (*dictSrcOut) << "      sobj->" << b.Fullname() << "::ShowMembers(R__insp);" << std::endl;
         } else {
            if (strstr(b.Fullname(),"::")) {
               // there is a namespace involved, trigger MS VC bug workaround
               (*dictSrcOut) << "      //This works around a msvc bug and should be harmless on other platforms" << std::endl
                             << "      typedef " << b.Fullname() << " baseClass" << base << ";" << std::endl
                             << "      baseClass" << base << "::ShowMembers(R__insp);" << std::endl;
            } else {
               (*dictSrcOut) << "      " << b.Fullname() << "::ShowMembers(R__insp);" << std::endl;
            }
         }
      } else {
         string baseclass = FixSTLName(b.Fullname());
         // We used to use a dynamic_cast for cast to the parent class.  This
         // was not necessary and actually crippling.  In this situations
         // (casting from child to parent) the C-style cast is returning the
         // same result as the dynamic_cast but has the advantage (for us) of
         // being able to apply the case even if the parent is inherited from
         // privately.

         //string baseclassWithDefaultStlName( m.Type()->Name()); //  RStl::DropDefaultArg(m.Type()->Name()) );
         //string baseclassWithDefaultStlName( TClassEdit::ShortType(baseclass.c_str(),
         //                                                          TClassEdit::kRemoveDefaultAlloc) );
         if (outside) {
            (*dictSrcOut) << "      R__insp.GenericShowMembers(\"" << baseclass.c_str() << "\", ( ::" << baseclass.c_str()
                          << " * )( (::" << cl.Fullname() << "*) obj ), false);" << std::endl;
         } else {
            (*dictSrcOut) << "      R__insp.GenericShowMembers(\"" << baseclass.c_str() << "\", ( ::" << baseclass.c_str()
                          << " *) (this ), false);" << std::endl;
         }
      }
   }
#endif // R__SHOWMEMBERS_IN_TCLING
}

//______________________________________________________________________________
void WriteShowMembers(const RScanner::AnnotatedRecordDecl &cl, bool outside = false)
{
   G__ClassInfo clinfo(cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str());
   const clang::CXXRecordDecl* cxxdecl = llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl());

   (*dictSrcOut) << "//_______________________________________";
   (*dictSrcOut) << "_______________________________________" << std::endl;

   string classname = GetLong64_Name( RStl::DropDefaultArg( cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str() ) );
   string mappedname = G__map_cpp_name((char*)classname.c_str());

   if (outside || clinfo.IsTmplt()) {
      (*dictSrcOut) << "namespace ROOT {" << std::endl

                    << "   void " << mappedname.c_str() << "_ShowMembers(void *obj, TMemberInspector &R__insp)"
                    << std::endl << "   {" << std::endl;
      WriteBodyShowMembers(clinfo, outside || clinfo.IsTmplt());
      (*dictSrcOut) << "   }" << std::endl << std::endl;
      (*dictSrcOut) << "}" << std::endl << std::endl;
   }

   if (!outside) {
      G__ClassInfo ns = clinfo.EnclosingSpace();
      string clsname = clinfo.Fullname();
      string nsname;
      if (ns.IsValid()) {
         nsname = ns.Fullname();
         clsname.erase (0, nsname.size() + 2);
      }
      bool add_template_keyword = NeedTemplateKeyword(cxxdecl);
      int enclSpaceNesting = 0;
      if (!nsname.empty()) {
         G__ShadowMaker nestTempShadowMaker(*dictSrcOut, "");
         enclSpaceNesting = nestTempShadowMaker.WriteNamespaceHeader(clinfo);
      }
      if (add_template_keyword) (*dictSrcOut) << "template <> ";
      (*dictSrcOut) << "void " << clsname << "::ShowMembers(TMemberInspector &R__insp)"
                    << std::endl << "{" << std::endl;
      if (!clinfo.IsTmplt()) {
         WriteBodyShowMembers(clinfo, outside);
      } else {
         string clnameNoDefArg = GetLong64_Name( RStl::DropDefaultArg( cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str() ) );
         string mappednameNoDefArg = G__map_cpp_name((char*)clnameNoDefArg.c_str());

         (*dictSrcOut) <<  "   ::ROOT::" << mappednameNoDefArg.c_str() << "_ShowMembers(this, R__insp);" << std::endl;
      }
      (*dictSrcOut) << "}" << std::endl << std::endl;

      while (enclSpaceNesting) {
         (*dictSrcOut) << "} // namespace " << nsname << std::endl;
         --enclSpaceNesting;
      }
   }
}

//______________________________________________________________________________
void WriteClassCode(const RScanner::AnnotatedRecordDecl &cl)
{
   std::string fullname;
   R__GetQualifiedName(fullname,cl);
   if (TClassEdit::IsSTLCont(fullname.c_str()) ) {
      // coverity[fun_call_w_exception] - that's just fine.
      RStl::Instance().GenerateTClassFor(cl.GetRequestedName(), llvm::dyn_cast<clang::CXXRecordDecl>(cl.GetRecordDecl()));
      return;
   }

   G__ClassInfo clinfo(cl.GetRequestedName()[0] ? cl.GetRequestedName() : R__GetQualifiedName(cl).c_str());

   if (ClassInfo__HasMethod(cl,"Streamer")) {
      if (clinfo.RootFlag()) WritePointersSTL(cl); // In particular this detect if the class has a version number.
      if (!(cl.RequestNoStreamer())) {
         if ((cl.RequestStreamerInfo() /*G__AUTOSTREAMER*/)) {
            WriteAutoStreamer(cl);
         } else {
            WriteStreamer(cl);
         }
      } else
         Info(0, "Class %s: Do not generate Streamer() [*** custom streamer ***]\n", clinfo.Fullname());
   } else {
      Info(0, "Class %s: Streamer() not declared\n", clinfo.Fullname());

      if (cl.RequestStreamerInfo()) WritePointersSTL(cl);
   }
   if (ClassInfo__HasMethod(cl,"ShowMembers")) {
      WriteShowMembers(cl);
   } else {
      if (NeedShadowClass(clinfo)) {
         WriteShowMembers(cl, true);
      }
   }
   WriteAuxFunctions(cl);
}

//______________________________________________________________________________
void GenerateLinkdef(int *argc, char **argv, int iv)
{
   FILE *fl = fopen(autold, "w");
   if (fl==0) {
      Error(0, "Could not write the automatically generated Linkdef: %s\n", autold);
      exit(1);
   }

   fprintf(fl, "#ifdef __CINT__\n\n");
   fprintf(fl, "#pragma link off all globals;\n");
   fprintf(fl, "#pragma link off all classes;\n");
   fprintf(fl, "#pragma link off all functions;\n\n");

   for (int i = iv; i < *argc; i++) {
      char *s, trail[3];
      int   nostr = 0, noinp = 0, bcnt = 0, l = strlen(argv[i])-1;
      for (int j = 0; j < 3; j++) {
         if (argv[i][l] == '-') {
            argv[i][l] = '\0';
            nostr = 1;
            l--;
         }
         if (argv[i][l] == '!') {
            argv[i][l] = '\0';
            noinp = 1;
            l--;
         }
         if (argv[i][l] == '+') {
            argv[i][l] = '\0';
            bcnt = 1;
            l--;
         }
      }
      if (nostr || noinp) {
         trail[0] = 0;
         if (nostr) strlcat(trail, "-",3);
         if (noinp) strlcat(trail, "!",3);
      }
      if (bcnt) {
         strlcpy(trail, "+",3);
         if (nostr)
            Error(0, "option + mutual exclusive with -\n");
      }
      char *cls = strrchr(argv[i], '/');
      if (!cls) cls = strrchr(argv[i], '\\');
      if (cls)
         cls++;
      else
         cls = argv[i];
      if ((s = strrchr(cls, '.'))) *s = '\0';
      if (nostr || noinp || bcnt)
         fprintf(fl, "#pragma link C++ class %s%s;\n", cls, trail);
      else
         fprintf(fl, "#pragma link C++ class %s;\n", cls);
      if (s) *s = '.';
   }

   fprintf(fl, "\n#endif\n");
   fclose(fl);
}

//______________________________________________________________________________
bool Which(const char *fname, string& pname)
{
   // Find file name in path specified via -I statements to CINT.
   // Can be only called after G__main(). Return pointer to static
   // space containing full pathname or 0 in case file not found.

   FILE *fp = 0;

   pname = fname;
#ifdef WIN32
   fp = fopen(pname.c_str(), "rb");
#else
   fp = fopen(pname.c_str(), "r");
#endif
   if (fp) {
      fclose(fp);
      return true;
   }

   struct G__includepath *ipath = G__getipathentry();

   while (!fp && ipath->pathname) {
      pname = ipath->pathname;
#ifdef WIN32
      pname += "\\";
      static const char* fopenopts = "rb";
#else
      pname += "/";
      static const char* fopenopts = "r";
#endif
      pname += fname;
      fp = fopen(pname.c_str(), fopenopts);
      ipath = ipath->next;
   }
   if (fp) {
      fclose(fp);
      return true;
   }
   pname = "";
   return false;
}

//______________________________________________________________________________
char *StrDup(const char *str)
{
   // Duplicate the string str. The returned string has to be deleted by
   // the user.

   if (!str) return 0;

   // allocate 20 extra characters in case of eg, vector<vector<T>>
   int nch = strlen(str)+20;
   char *s = new char[nch];
   if (s) strlcpy(s, str,nch);

   return s;
}

//______________________________________________________________________________
char *Compress(const char *str)
{
   // Remove all blanks from the string str. The returned string has to be
   // deleted by the user.

   if (!str) return 0;

   const char *p = str;
   // allocate 20 extra characters in case of eg, vector<vector<T>>
   char *s, *s1 = new char[strlen(str)+20];
   s = s1;

   while (*p) {
      // keep space for A<const B>!
      if (*p != ' ' || (p - str > 0 && isalnum(*(p-1))))
         *s++ = *p;
      p++;
   }
   *s = '\0';

   return s1;
}

//______________________________________________________________________________
const char *CopyArg(const char *original)
{
   // If the argument starts with MODULE/inc, strip it
   // to make it the name we can use in #includes.

#ifdef ROOTBUILD
   if (R__IsSelectionFile(original)) {
      return original;
   }

   const char *inc = strstr(original, "\\inc\\");
   if (!inc)
      inc = strstr(original, "/inc/");
   if (inc && strlen(inc) > 5)
      return inc + 5;
   return original;
#else
   return original;
#endif
}

//______________________________________________________________________________
void StrcpyWithEsc(string& escaped, const char *original)
{
   // Copy original into escaped BUT make sure that the \ characters
   // are properly escaped (on Windows temp files have \'s).

   int j = 0;
   escaped = "";
   while (original[j] != '\0') {
      if (original[j] == '\\')
         escaped += '\\';
      escaped += original[j++];
   }
}

//______________________________________________________________________________
void StrcpyArg(string& dest, const char *original)
{
   // Copy the command line argument, stripping MODULE/inc if
   // necessary.

   dest = CopyArg( original );
}

//______________________________________________________________________________
void StrcpyArgWithEsc(string& escaped, const char *original)
{
   // Copy the command line argument, stripping MODULE/inc if
   // necessary and then escaping string.

   escaped = CopyArg( original );
}

//______________________________________________________________________________
void ReplaceFile(const char *tmpdictname, const char *dictname)
{
   // Unlink dictname and move tmpdictname into dictname

#ifdef WIN32
   int tries=0;
   bool success=false;
   while (!success && ++tries<51) {
      success = (unlink(dictname) != -1);
      if (!success && tries<50)
         if (errno!=EACCES) break;
         else Sleep(200);
   }
   if (success) {
      success=false;
      tries=0;
      while (!success && ++tries<52) {
         success = (rename(tmpdictname, dictname) != -1);
         if (!success && tries<51)
            if (errno!=EACCES) break;
            else Sleep(200);
      }
   }
   if (!success)
      Error(0, "rootcint: failed to rename %s to %s in ReplaceBundleInDict() after %d tries (error is %d)\n",
            tmpdictname, dictname, tries, errno);
#else
   if (unlink(dictname) == -1 || rename(tmpdictname, dictname) == -1)
      Error(0, "rootcint: failed to rename %s to %s in ReplaceBundleInDict()\n",
            tmpdictname, dictname);
#endif

}

//______________________________________________________________________________
void ReplaceBundleInDict(const char *dictname, const string &bundlename)
{
   // Replace the bundlename in the dict.cxx and .h file by the contents
   // of the bundle.

   // First patch dict.cxx. Create tmp file and copy dict.cxx to this file.
   // When discovering a line like:
   //   G__add_compiledheader("bundlename");
   // replace it by the appropriate number of lines contained in the bundle.

   FILE *fpd = fopen(dictname, "r");
   if (!fpd) {
      Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
            dictname);
      return;
   }

   string tmpdictname(dictname);
   tmpdictname += "_+_+_+rootcinttmp";
   FILE *tmpdict = fopen(tmpdictname.c_str(), "w");
   if (!tmpdict) {
      Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
            tmpdictname.c_str());
      fclose(fpd);
      return;
   }

   string esc_bundlename;
   StrcpyWithEsc(esc_bundlename, bundlename.c_str());

   string checkline("  G__add_compiledheader(\"");
   checkline += esc_bundlename;
   checkline += "\");";
   int clen = checkline.length();

   char line[BUFSIZ];
   if (tmpdict && fpd) {
      while (fgets(line, BUFSIZ, fpd)) {
         if (!strncmp(line, checkline.c_str(), clen)) {
            FILE *fb = fopen(bundlename.c_str(), "r");
            if (!fb) {
               Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
                     bundlename.c_str());
               fclose(fpd);
               fclose(tmpdict);
               remove(tmpdictname.c_str());
               return;
            }
            while (fgets(line, BUFSIZ, fb)) {
               char *s = strchr(line, '"');
               if (!s) continue;
               s++;
               char *s1 = strrchr(s, '"');
               if (R__IsSelectionFile(s))
                  continue;
               if (s1) {
                  *s1 = 0;
                  fprintf(tmpdict, "  G__add_compiledheader(\"%s\");\n", s);
               }
            }
            fclose(fb);
         } else
            fprintf(tmpdict, "%s", line);
      }
   }

   fclose(tmpdict);
   fclose(fpd);

   ReplaceFile(tmpdictname.c_str(),dictname);

   // Next patch dict.h. Create tmp file and copy dict.h to this file.
   // When discovering a line like:
   //   #include "bundlename"
   // replace it by the appropriate number of lines contained in the bundle.

   // make dict.h
   string dictnameh(dictname);
   size_t dh = dictnameh.rfind('.');
   if (dh != std::string::npos) {
      dictnameh.erase(dh + 1);
      dictnameh += "h";
   } else {
      Error(0, "rootcint: failed create dict.h in ReplaceBundleInDict()\n");
      return;
   }

   fpd = fopen(dictnameh.c_str(), "r");
   if (!fpd) {
      Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
            dictnameh.c_str());
      return;
   }
   tmpdict = fopen(tmpdictname.c_str(), "w");
   if (!tmpdict) {
      Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
            tmpdictname.c_str());
      fclose(fpd);
      return;
   }

   checkline = "#include \"";
   checkline += esc_bundlename + "\"";
   clen = checkline.length();

   if (tmpdict && fpd) {
      while (fgets(line, BUFSIZ, fpd)) {
         if (!strncmp(line, checkline.c_str(), clen)) {
            FILE *fb = fopen(bundlename.c_str(), "r");
            if (!fb) {
               Error(0, "rootcint: failed to open %s in ReplaceBundleInDict()\n",
                     bundlename.c_str());
               fclose(tmpdict);
               fclose(fpd);
               return;
            }
            while (fgets(line, BUFSIZ, fb)) {
               char *s = strchr(line, '<');
               if (!s) continue;
               s++;
               char *s1 = strrchr(s, '>');
               if (R__IsSelectionFile(s))
                  continue;
               if (s1) {
                  *s1 = 0;
                  fprintf(tmpdict, "#include \"%s\"\n", s);
               }
            }
            fclose(fb);
         } else
            fprintf(tmpdict, "%s", line);
      }
   }

   fclose(tmpdict);
   fclose(fpd);

   ReplaceFile(tmpdictname.c_str(),dictnameh.c_str());
}

string bundlename;
string tname;
string dictsrc;

//______________________________________________________________________________
void CleanupOnExit(int code)
{
   // Removes tmp files, and (if code!=0) output files.

   if (!bundlename.empty()) unlink(bundlename.c_str());
   if (!tname.empty()) unlink(tname.c_str());
   if (autold[0]) unlink(autold);
   if (code) {
      if (!dictsrc.empty()) {
         unlink(dictsrc.c_str());
         // also remove the .d file belonging to dictsrc
         size_t posExt=dictsrc.rfind('.');
         if (posExt!=string::npos) {
            dictsrc.replace(posExt, dictsrc.length(), ".d");
            unlink(dictsrc.c_str());
         }
      }
   }
   // also remove the .def file created by CINT.
   {
      size_t posExt=dictsrc.rfind('.');
      if (posExt!=string::npos) {
         dictsrc.replace(posExt, dictsrc.length(), ".def");
         unlink(dictsrc.c_str());

         size_t posSlash=dictsrc.rfind('/');
         if (posSlash==string::npos) {
            posSlash=dictsrc.rfind('\\');
         }
         if (posSlash!=string::npos) {
            dictsrc.replace(0,posSlash+1,"");
            unlink(dictsrc.c_str());
         }
      }
   }
}

enum ESourceFileKind {
   kSFKNotC,
   kSFKHeader,
   kSFKSource,
   kSFKLinkdef
};

//______________________________________________________________________________
static ESourceFileKind GetSourceFileKind(const char* filename)
{
   // Check whether the file's extension is compatible with C or C++.
   // Return whether source, header, Linkdef or nothing.
   if (filename[0] == '-') return kSFKNotC;

   const size_t len = strlen(filename);
   const char* ext = filename + len - 1;
   while (ext >= filename && *ext != '.') --ext;
   if (ext < filename || *ext != '.') return kSFKNotC;
   ++ext;
   const size_t lenExt = filename + len - ext;

   ESourceFileKind ret = kSFKNotC;
   switch (lenExt) {
   case 1: {
      const char last = toupper(filename[len - 1]);
      if (last == 'H') ret = kSFKHeader;
      else if (last == 'C') ret = kSFKSource;
      break;
   }
   case 2: {
      if (filename[len - 2] == 'h' && filename[len - 1] == 'h')
         ret = kSFKHeader;
      else if (filename[len - 2] == 'c' && filename[len - 1] == 'c')
         ret = kSFKSource;
      break;
   }
   case 3: {
      const char last = filename[len - 1];
      if ((last == 'x' || last == 'p')
          && filename[len - 2] == last) {
         if (filename[len - 3] == 'h') ret = kSFKHeader;
         else if (filename[len - 3] == 'c') ret = kSFKSource;
      }
   }
   } // switch extension length

   static const size_t lenLinkdefdot = 8;
   if (ret == kSFKHeader && len - lenExt >= lenLinkdefdot) {
      if ((strstr(filename,"LinkDef") || strstr(filename,"Linkdef") ||
           strstr(filename,"linkdef")) && strstr(filename,".h")) {
         ret = kSFKLinkdef;
      }
   }
   return ret;
}


//______________________________________________________________________________
static int GenerateModule(const char* dictname, const std::vector<std::string>& args)
{
   // Generate the clang module given the arguments.
   // Returns != 0 on error.
   std::string clangInvocation(R__CLANG);
   std::string dictNameStem(dictname);
   size_t posDotPcmFile = dictNameStem.find('.');
   if (posDotPcmFile != std::string::npos) {
      // remove extension.
      dictNameStem.erase(posDotPcmFile, dictNameStem.length() - posDotPcmFile);
   }
   std::string moduleMapName = std::string("lib/") + dictNameStem + "_dict.map";

   clangInvocation += " -Xclang -emit-module -Xclang -fmodules -Xclang -fmodule-cache-path -Xclang lib -Xclang -fmodule-name=" + dictNameStem + "_dict -o lib/";
   clangInvocation += dictNameStem + "_dict.pcm -x c++ -c " + moduleMapName;

   // Parse arguments
   vector<std::string> headers;
   for (size_t iPcmArg = 1 /*skip argv0*/, nPcmArg = args.size();
        iPcmArg < nPcmArg; ++iPcmArg) {
      ESourceFileKind sfk = GetSourceFileKind(args[iPcmArg].c_str());
      if (sfk == kSFKHeader || sfk == kSFKSource) {
         headers.push_back(args[iPcmArg]);
      } else if (sfk == kSFKNotC) {
         clangInvocation += std::string(" ") + args[iPcmArg];
      }
   }

   // Generate module map
   {
      std::ofstream moduleMap(moduleMapName.c_str());
      moduleMap << "module " << dictNameStem << "_dict { " << std::endl;
      for (size_t iH = 0, eH = headers.size(); iH < eH; ++iH) {
         moduleMap << "header \"" << headers[iH] << "\"" << std::endl;
      }
      moduleMap << "}" << std::endl;
   }

   // Dictionary initialization code for loading the module
   (*dictSrcOut) << "namespace {\n"
      "  static struct DictInit {\n"
      "    DictInit() {\n"
      "      static const char* headers[] = {\n";

   {
      char currWorkDir[1024];
#ifdef WIN32
      ::_getcwd(currWorkDir, sizeof(currWorkDir));
#else
      getcwd(currWorkDir, sizeof(currWorkDir));
#endif
      size_t lenCurrWorkDir = strlen(currWorkDir);
      if (lenCurrWorkDir + 1 < sizeof(currWorkDir)) {
         currWorkDir[lenCurrWorkDir++] = '/';
         currWorkDir[lenCurrWorkDir] = 0;
      }
      for (size_t iH = 0, eH = headers.size(); iH < eH; ++iH) {
         if (headers[iH].substr(0, lenCurrWorkDir) == currWorkDir) {
            // Convert to path relative to $PWD.
            // If that's not what the caller wants, she should pass -I to rootcint and a
            // different relative path to the header files.
            headers[iH].erase(0, lenCurrWorkDir);
         }
#ifdef ROOTBUILD
         // For ROOT, convert module directories like core/base/inc/ to include/
         int posInc = headers[iH].find("/inc/");
         if (posInc != -1) {
            headers[iH] = /*std::string("include") +*/ headers[iH].substr(posInc + 5, -1);
         }
#endif
         (*dictSrcOut) << "             \"" << headers[iH] << "\"," << std::endl;
      }
   }
   (*dictSrcOut) << 
      "      0 };\n"
      "      TCintWithCling__RegisterModule(\"" << dictNameStem << "\", headers);\n"
      "    }\n"
      "  } __TheInitializer;\n"
      "}" << std::endl;

   printf("Would like to generate PCM: %s\n",clangInvocation.c_str());
   int ret = system(clangInvocation.c_str());
   //unlink(moduleMapName.c_str());
   return ret;
}


// cross-compiling for iOS and iOS simulator (assumes host is Intel Mac OS X)
#if defined(R__IOSSIM) || defined(R__IOS)
#ifdef __x86_64__
#undef __x86_64__
#endif
#ifdef __i386__
#undef __i386__
#endif
#ifdef R__IOSSIM
#define __i386__ 1
#endif
#ifdef R__IOS
#define __arm__ 1
#endif
#endif

//______________________________________________________________________________
int main(int argc, char **argv)
{
   if (argc < 2) {
      fprintf(stderr,
              "Usage: %s [-v][-v0-4] [-cint|-reflex|-gccxml] [-l] [-f] [out.cxx] [-c] file1.h[+][-][!] file2.h[+][-][!]...[LinkDef.h]\n",
              argv[0]);
      fprintf(stderr, "For more extensive help type: %s -h\n", argv[0]);
      return 1;
   }

   char dictname[1024];
   int i, j, ic, ifl, force;
   int icc = 0;
   int use_preprocessor = 0;
   int longheadername = 0;
   bool requestAllSymbols = false; // Would be set to true is we decide to support an option like --deep.
   string dictpathname;
   string libfilename;
   const char *env_dict_type=getenv("ROOTDICTTYPE");
   int dicttype = 0; // 09-07-07 -- 0 for dict, 1 for ShowMembers

   if (env_dict_type) {
      if (!strcmp(env_dict_type, "cint"))
         dict_type=kDictTypeCint;
      else if (!strcmp(env_dict_type, "reflex"))
         dict_type=kDictTypeReflex;
      else if (!strcmp(env_dict_type, "gccxml"))
         dict_type=kDictTypeGCCXML;
   }

   // coverity[secure_coding] - pid can have up to 47 digits!
   snprintf(autold,64, autoldtmpl, getpid());

   ic = 1;
   if (!strcmp(argv[ic], "-v")) {
      gErrorIgnoreLevel = kInfo; // The default is kError
      ic++;
   } else if (!strcmp(argv[ic], "-v0")) {
      gErrorIgnoreLevel = kFatal; // Explicitly remove all messages
      ic++;
   } else if (!strcmp(argv[ic], "-v1")) {
      gErrorIgnoreLevel = kError; // Only error message (default)
      ic++;
   } else if (!strcmp(argv[ic], "-v2")) {
      gErrorIgnoreLevel = kWarning; // error and warning message
      ic++;
   } else if (!strcmp(argv[ic], "-v3")) {
      gErrorIgnoreLevel = kNote; // error, warning and note
      ic++;
   } else if (!strcmp(argv[ic], "-v4")) {
      gErrorIgnoreLevel = kInfo; // Display all information (same as -v)
      ic++;
   }
   if (ic < argc) {
      if (!strcmp(argv[ic], "-cint")) {
         dict_type = kDictTypeCint;
         ic++;
      } else if (!strcmp(argv[ic], "-reflex")) {
         dict_type = kDictTypeReflex;
         ic++;
      } else if (!strcmp(argv[ic], "-gccxml")) {
         dict_type = kDictTypeGCCXML;
         ic++;
      }
   }

   if (dict_type==kDictTypeGCCXML) {
      int rc =  system("genreflex-rootcint --gccxml-available");
      if (rc) dict_type=kDictTypeReflex; // fall back to reflex
   }

   const char* libprefix = "--lib-list-prefix=";

   ifl = 0;
   while (ic < argc && strncmp(argv[ic], "-",1)==0
          && strcmp(argv[ic], "-f")!=0 ) {
      if (!strcmp(argv[ic], "-l")) {

         longheadername = 1;
         ic++;
      } else if (!strncmp(argv[ic],libprefix,strlen(libprefix))) {

         gLiblistPrefix = argv[ic]+strlen(libprefix);

         string filein = gLiblistPrefix + ".in";
         FILE *fp;
         if ((fp = fopen(filein.c_str(), "r")) == 0) {
            Error(0, "%s: The input list file %s does not exist\n", argv[0], filein.c_str());
            return 1;
         }
         fclose(fp);

         ic++;
      } else {
         break;
      }
   }

   if (ic < argc && !strcmp(argv[ic], "-f")) {
      force = 1;
      ic++;
   } else if (argc > 1 && (!strcmp(argv[1], "-?") || !strcmp(argv[1], "-h"))) {
      fprintf(stderr, "%s\n", help);
      return 1;
   } else if (ic < argc && !strncmp(argv[ic], "-",1)) {
      fprintf(stderr,"Usage: %s [-v][-v0-4] [-reflex] [-l] [-f] [out.cxx] [-c] file1.h[+][-][!] file2.h[+][-][!]...[LinkDef.h]\n",
              argv[0]);
      fprintf(stderr,"Only one verbose flag is authorized (one of -v, -v0, -v1, -v2, -v3, -v4)\n"
              "and must be before the -f flags\n");
      fprintf(stderr,"For more extensive help type: %s -h\n", argv[0]);
      return 1;
   } else {
      force = 0;
   }

#if defined(R__WIN32) && !defined(R__WINGCC)
   // cygwin's make is presenting us some cygwin paths even though
   // we are windows native. Convert them as good as we can.
   for (int iic = ic; iic < argc; ++iic) {
      std::string iiarg(argv[iic]);
      if (FromCygToNativePath(iiarg)) {
         size_t len = iiarg.length();
         // yes, we leak.
         char* argviic = new char[len + 1];
         strlcpy(argviic, iiarg.c_str(), len + 1);
         argv[iic] = argviic;
      }
   }
#endif

   string header("");
   if (ic < argc && (strstr(argv[ic],".C")  || strstr(argv[ic],".cpp") ||
       strstr(argv[ic],".cp") || strstr(argv[ic],".cxx") ||
       strstr(argv[ic],".cc") || strstr(argv[ic],".c++"))) {
      FILE *fp;
      if ((fp = fopen(argv[ic], "r")) != 0) {
         fclose(fp);
         if (!force) {
            Error(0, "%s: output file %s already exists\n", argv[0], argv[ic]);
            return 1;
         }
      }
      //string header( argv[ic] );
      header = argv[ic];
      int loc = strrchr(argv[ic],'.') - argv[ic];
      header[loc+1] = 'h';
      header[loc+2] = '\0';
      if ((fp = fopen(header.c_str(), "r")) != 0) {
         fclose(fp);
         if (!force) {
            Error(0, "%s: output file %s already exists\n", argv[0], header.c_str());
            return 1;
         } else {
            for (int k = ic+1; k < argc; ++k) {
               if (*argv[k] != '-' && *argv[k] != '+') {
                  if (strcmp(header.c_str(),argv[k])==0) {
                     Error(0, "%s: output file %s would overwrite one of the input files!\n", argv[0], header.c_str());
                     return 1;
                  }
                  if (strcmp(argv[ic],argv[k])==0) {
                     Error(0, "%s: output file %s would overwrite one of the input files!\n", argv[0],argv[ic]);
                     return 1;
                  }
               }
            }
         }
      }

      dictsrc=argv[ic];
      fp = fopen(argv[ic], "w");
      if (fp) fclose(fp);    // make sure file is created and empty
      ifl = ic;
      ic++;

      // remove possible pathname to get the dictionary name
      if (strlen(argv[ifl]) > (sizeof(dictname)-1)) {
         Error(0, "rootcint: dictionary name too long (more than %d characters): %s\n",
               sizeof(dictname)-1,argv[ifl]);
         CleanupOnExit(1);
         return 1;
      }
      strncpy(dictname, argv[ifl], sizeof(dictname)-1);
      char *p = 0;
      // find the right part of then name.
      for (p = dictname + strlen(dictname)-1;p!=dictname;--p) {
         if (*p =='/' ||  *p =='\\') {
            *p = 0;
            if (p == dictname) {
               dictpathname = "/";
            } else {
               dictpathname = dictname;
            }
            break;
         }
      }
      if (!p)
         p = dictname;
      else if (p != dictname) {
         p++;
         memmove(dictname, p, strlen(p)+1);
      }
   } else if (!strcmp(argv[1], "-?") || !strcmp(argv[1], "-h")) {
      fprintf(stderr, "%s\n", help);
      return 1;
   } else {
      ic = 1;
      if (force) ic = 2;
      ifl = 0;
   }

   // If the user request use of a preprocessor we are going to bundle
   // all the files into one so that cint considers them one compilation
   // unit and so that each file that contains code guard is really
   // included only once.
   for (i = 1; i < argc; i++)
      if (strcmp(argv[i], "-p") == 0) use_preprocessor = 1;

#ifndef __CINT__
   int   argcc, iv, il;
   std::vector<std::string> path;
   char *argvv[500];

   std::vector<std::string> clingArgs;
   clingArgs.push_back(argv[0]);
   clingArgs.push_back("-I.");
   clingArgs.push_back("-DROOT_Math_VectorUtil_Cint"); // ignore that little problem maker

#ifndef ROOTBUILD
# ifndef ROOTINCDIR
   SetRootSys();
   if (getenv("ROOTSYS")) {
      std::string incl_rootsys = std::string("-I") + getenv("ROOTSYS");
      path.push_back(incl_rootsys + "/include");
      path.push_back(incl_rootsys + "/src");
   } else {
      Error(0, "%s: environment variable ROOTSYS not defined\n", argv[0]);
      return 1;
   }
# else
   path.push_back(std::string("-I") + ROOTINCDIR);
# endif
#else
   path.push_back("-Iinclude");
#endif

   argvv[0] = argv[0];
   argcc = 1;

   if (ic < argc && !strcmp(argv[ic], "-c")) {
      icc++;
      if (ifl) {
         char *s;
         ic++;
         argvv[argcc++] = (char *)"-q0";
         argvv[argcc++] = (char *)"-n";
         int ncha = strlen(argv[ifl])+1;
         argvv[argcc] = (char *)calloc(ncha, 1);
         strlcpy(argvv[argcc], argv[ifl],ncha); argcc++;
         argvv[argcc++] = (char *)"-N";
         s = strrchr(dictname,'.');
         argvv[argcc] = (char *)calloc(strlen(dictname), 1);
         strncpy(argvv[argcc], dictname, s-dictname); argcc++;

         while (ic < argc && (*argv[ic] == '-' || *argv[ic] == '+')) {
            if (strcmp("+P", argv[ic]) == 0 ||
                strcmp("+V", argv[ic]) == 0 ||
                strcmp("+STUB", argv[ic]) == 0) {
               // break when we see positional options
               break;
            }
            if (strcmp("-pipe", argv[ic])!=0 && strcmp("-pthread", argv[ic])!=0) {
               // filter out undesirable options
               if (strcmp("-fPIC", argv[ic]) && strcmp("-fpic", argv[ic])
                   && strcmp("-p", argv[ic])) {
                  clingArgs.push_back(argv[ic]);
               }
               argvv[argcc++] = argv[ic++];
            } else {
               ic++;
            }
         }

         for (i = 0; i < (int)path.size(); i++) {
            argvv[argcc++] = (char*)path[i].c_str();
            clingArgs.push_back(path[i].c_str());
         }

#ifdef __hpux
         argvv[argcc++] = (char *)"-I/usr/include/X11R5";
#endif
         switch (gErrorIgnoreLevel) {
         case kInfo:     argvv[argcc++] = (char *)"-J4"; break;
         case kNote:     argvv[argcc++] = (char *)"-J3"; break;
         case kWarning:  argvv[argcc++] = (char *)"-J2"; break;
         case kError:    argvv[argcc++] = (char *)"-J1"; break;
         case kSysError:
         case kFatal:    argvv[argcc++] = (char *)"-J0"; break;
         default:        argvv[argcc++] = (char *)"-J1"; break;
         }

         if (!use_preprocessor) {
            // If the compiler's preprocessor is not used
            // we still need to declare the compiler specific flags
            // so that the header file are properly parsed.
#ifdef __KCC
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__KCC=%ld", (long)__KCC); argcc++;
#endif
#ifdef __INTEL_COMPILER
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__INTEL_COMPILER=%ld", (long)__INTEL_COMPILER); argcc++;
#endif
#ifdef __xlC__
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__xlC__=%ld", (long)__xlC__); argcc++;
#endif
#ifdef __GNUC__
            argvv[argcc] = (char *)calloc(64, 1);
            // coverity[secure_coding] - sufficient space
            snprintf(argvv[argcc],64, "-D__GNUC__=%ld", (long)__GNUC__); argcc++;
#endif
#ifdef __GNUC_MINOR__
            argvv[argcc] = (char *)calloc(64, 1);
            // coverity[secure_coding] - sufficient space
            snprintf(argvv[argcc],64, "-D__GNUC_MINOR__=%ld", (long)__GNUC_MINOR__); argcc++;
#endif
#ifdef __HP_aCC
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64 "-D__HP_aCC=%ld", (long)__HP_aCC); argcc++;
#endif
#ifdef __sun
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__sun=%ld", (long)__sun); argcc++;
#endif
#ifdef __SUNPRO_CC
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__SUNPRO_CC=%ld", (long)__SUNPRO_CC); argcc++;
#endif
#ifdef __ia64__
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__ia64__=%ld", (long)__ia64__); argcc++;
#endif
#ifdef __x86_64__
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__x86_64__=%ld", (long)__x86_64__); argcc++;
#endif
#ifdef __i386__
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__i386__=%ld", (long)__i386__); argcc++;
#endif
#ifdef __arm__
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D__arm__=%ld", (long)__arm__); argcc++;
#endif
#ifdef R__B64
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-DR__B64"); argcc++;
#endif
#ifdef _WIN32
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D_WIN32=%ld",(long)_WIN32); argcc++;
#endif
#ifdef WIN32
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-DWIN32=%ld",(long)WIN32); argcc++;
#endif
#ifdef _MSC_VER
            argvv[argcc] = (char *)calloc(64, 1);
            snprintf(argvv[argcc],64, "-D_MSC_VER=%ld",(long)_MSC_VER); argcc++;
#endif
         }
#ifdef ROOTBUILD
         argvv[argcc++] = (char *)"-DG__NOCINTDLL";
         clingArgs.push_back(argvv[argcc - 1]);
#endif
         argvv[argcc++] = (char *)"-DTRUE=1";
         clingArgs.push_back(argvv[argcc - 1]);
         argvv[argcc++] = (char *)"-DFALSE=0";
         clingArgs.push_back(argvv[argcc - 1]);
         argvv[argcc++] = (char *)"-Dexternalref=extern";
         argvv[argcc++] = (char *)"-DSYSV";
         argvv[argcc++] = (char *)"-D__MAKECINT__";
         // NO! clang needs to see the truth.
         // clingArgs.push_back(argvv[argcc - 1]);
         argvv[argcc++] = (char *)"-V";        // include info on private members
         if (dict_type==kDictTypeReflex) {
            argvv[argcc++] = (char *)"-c-3";
         } else {
            argvv[argcc++] = (char *)"-c-10";
         }
         argvv[argcc++] = (char *)"+V";        // turn on class comment mode
         if (!use_preprocessor) {
#ifdef ROOTBUILD
            argvv[argcc++] = (char *)"TObject.h";
            argvv[argcc++] = (char *)"TMemberInspector.h";
            //argvv[argcc++] = (char *)"base/inc/TObject.h";
            //argvv[argcc++] = (char *)"base/inc/TMemberInspector.h";
#else
            argvv[argcc++] = (char *)"TObject.h";
            argvv[argcc++] = (char *)"TMemberInspector.h";
#endif
         }
      } else {
         Error(0, "%s: option -c can only be used when an output file has been specified\n", argv[0]);
         return 1;
      }
   }
   iv = 0;
   il = 0;

   std::vector<std::string> pcmArgs;
   for (size_t parg = 0, n = clingArgs.size(); parg < n; ++parg) {
      if (clingArgs[i] != "-c")
         pcmArgs.push_back(clingArgs[parg]);
   }

   // cling-only arguments
   clingArgs.push_back("-fsyntax-only");
   std::string interpInclude("-I");
#ifndef ROOTBUILD
# ifndef ROOTINCDIR
   std::string rootsys = getenv("ROOTSYS");
   interpInclude += rootsys + "/etc";
# else
   interpInclude += ROOTETCDIR;
# endif
#else
   interpInclude += "etc";
#endif
   clingArgs.push_back(interpInclude.c_str());

   std::vector<const char*> clingArgsC;
   for (size_t iclingArgs = 0, nclingArgs = clingArgs.size();
        iclingArgs < nclingArgs; ++iclingArgs) {
      clingArgsC.push_back(clingArgs[iclingArgs].c_str());
   }
   cling::Interpreter interp(clingArgsC.size(), &clingArgsC[0],
                             getenv("LLVMDIR"));
   interp.declare("namespace std {} using namespace std;");
   gInterp = &interp;

   // flags used only for the pragma parser:
   clingArgs.push_back("-D__CINT__");
   clingArgs.push_back("-D__MAKECINT__");
   char platformDefines[64] = {0};
#ifdef __KCC
   snprintf(platformDefines, 64, "-DG__KCC=%ld", (long)__KCC);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __INTEL_COMPILER
   snprintf(platformDefines, 64, "-DG__INTEL_COMPILER=%ld", (long)__INTEL_COMPILER);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __xlC__
   snprintf(platformDefines, 64, "-DG__xlC=%ld", (long)__xlC__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __GNUC__
   snprintf(platformDefines, 64, "-DG__GNUC=%ld", (long)__GNUC__);
   snprintf(platformDefines, 64, "-DG__GNUC_VER=%ld", (long)__GNUC__*1000 + __GNUC_MINOR__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __GNUC_MINOR__
   snprintf(platformDefines, 64, "-DG__GNUC_MINOR=%ld", (long)__GNUC_MINOR__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __HP_aCC
   snprintf(platformDefines, 64, "-DG__HP_aCC=%ld", (long)__HP_aCC);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __sun
   snprintf(platformDefines, 64, "-DG__sun=%ld", (long)__sun);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __SUNPRO_CC
   snprintf(platformDefines, 64, "-DG__SUNPRO_CC=%ld", (long)__SUNPRO_CC);
   clingArgs.push_back(platformDefines);
#endif
#ifdef _STLPORT_VERSION
   // stlport version, used on e.g. SUN
   snprintf(platformDefines, 64, "-DG__STLPORT_VERSION=%ld", (long)_STLPORT_VERSION);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __ia64__
   snprintf(platformDefines, 64, "-DG__ia64=%ld", (long)__ia64__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __x86_64__
   snprintf(platformDefines, 64, "-DG__x86_64=%ld", (long)__x86_64__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __i386__
   snprintf(platformDefines, 64, "-DG__i386=%ld", (long)__i386__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef __arm__
   snprintf(platformDefines, 64, "-DG__arm=%ld", (long)__arm__);
   clingArgs.push_back(platformDefines);
#endif
#ifdef _WIN32
   snprintf(platformDefines, 64, "-DG__WIN32=%ld",(long)_WIN32);
   clingArgs.push_back(platformDefines);
#else
# ifdef WIN32
   snprintf(platformDefines, 64, "-DG__WIN32=%ld",(long)WIN32);
   clingArgs.push_back(platformDefines);
# endif
#endif
#ifdef _MSC_VER
   snprintf(platformDefines, 64, "-DG__MSC_VER=%ld",(long)_MSC_VER);
   clingArgs.push_back(platformDefines);
   snprintf(platformDefines, 64, "-DG__VISUAL=%ld",(long)_MSC_VER);
   clingArgs.push_back(platformDefines);
#endif

   std::string interpPragmaSource;
   std::list<std::string> includedFilesForBundle;
   string esc_arg;
   bool insertedBundle = false;
   FILE *bundle = 0;
   if (use_preprocessor) {
      bundlename = R__tmpnam();
      bundlename += ".h";
      bundle = fopen(bundlename.c_str(), "w");
      if (!bundle) {
         Error(0, "%s: failed to open %s, usage of external preprocessor by CINT is not optimal\n",
               argv[0], bundlename.c_str());
         use_preprocessor = 0;
      } else {
         // use <> instead of "" otherwise the CPP will search first
         // for these files in /tmp (location of the bundle.h) where
         // it might not find the files (if starting with ./ or ../)
         // or, even worse, pick up a wrong version placed in /tmp.
         fprintf(bundle,"#include <TObject.h>\n");
         fprintf(bundle,"#include <TMemberInspector.h>\n");
      }
   }
   for (i = ic; i < argc; i++) {
      if (!iv && *argv[i] != '-' && *argv[i] != '+') {
         if (!icc) {
            for (j = 0; j < (int)path.size(); j++) {
               argvv[argcc++] = (char*)path[j].c_str();
            }
            argvv[argcc++] = (char *)"+V";
         }
         iv = argcc;
      }
      if (R__IsSelectionFile(argv[i])) {
         il = i;
         if (i != argc-1) {
            Error(0, "%s: %s must be last file on command line\n", argv[0], argv[i]);
            if (use_preprocessor) {
               fclose(bundle);
            }
            return 1;
         }
      }
      if (!strcmp(argv[i], "-c")) {
         Error(0, "%s: option -c must come directly after the output file\n", argv[0]);
         if (use_preprocessor) {
            fclose(bundle);
         }
         return 1;
      }
      if (use_preprocessor && *argv[i] != '-' && *argv[i] != '+') {
         StrcpyArgWithEsc(esc_arg, argv[i]);
         if (use_preprocessor) {
            // see comment about <> and "" above
            fprintf(bundle,"#include <%s>\n", esc_arg.c_str());
            includedFilesForBundle.push_back(argv[i]);
            if (!insertedBundle) {
               argvv[argcc++] = (char*)bundlename.c_str();
               insertedBundle = true;
            }
         }
         interp.declare(std::string("#include \"") + argv[i] + "\"");
         interpPragmaSource += std::string("#include \"") + argv[i] + "\"\n";
         pcmArgs.push_back(argv[i]);
      } else {
         if (strcmp("-pipe", argv[ic])!=0) {
            // filter out undesirable options
            string argkeep;
            // coverity[tainted_data] The OS should already limit the argument size, so we are safe here
            StrcpyArg(argkeep, argv[i]);
            int ncha = argkeep.length()+1;
            // coverity[tainted_data] The OS should already limit the argument size, so we are safe here
            argvv[argcc++] = (char*)calloc(ncha,1);
            // coverity[tainted_data] The OS should already limit the argument size, so we are safe here
            strlcpy(argvv[argcc-1],argkeep.c_str(),ncha);

            if (*argv[i] != '-' && *argv[i] != '+') {
               // Looks like a file
               interp.declare(std::string("#include \"") + argv[i] + "\"");
               interpPragmaSource += std::string("#include \"") + argv[i] + "\"\n";
               pcmArgs.push_back(argv[i]);
            }
         }
      }
   }
   if (use_preprocessor) {
      fclose(bundle);
   }

   if (!iv) {
      Error(0, "%s: no input files specified\n", argv[0]);
      return 1;
   }

   if (!il) {
      // replace bundlename by headers for autolinkdef
      char* bundleAutoLinkdef = argvv[argcc - 1];
      if (insertedBundle)
         --argcc;
      for (std::list<std::string>::const_iterator iHdr = includedFilesForBundle.begin();
           iHdr != includedFilesForBundle.end(); ++iHdr) {
         argvv[argcc] = StrDup(iHdr->c_str());
         ++argcc;
      }
      GenerateLinkdef(&argcc, argvv, iv);
      for (int iarg = argcc - includedFilesForBundle.size();
           iarg < argcc; ++iarg)
         delete [] argvv[iarg];
      argcc -= includedFilesForBundle.size();
      if (insertedBundle)
         ++argcc;
      argvv[argcc - 1] = bundleAutoLinkdef;

      argvv[argcc++] = autold;
   }

   G__ShadowMaker::VetoShadow(); // we create them ourselves
   G__setothermain(2);
   G__set_ioctortype_handler( (int (*)(const char*))AddConstructorType );
   G__set_beforeparse_hook( BeforeParseInit );
   if (G__main(argcc, argvv) < 0) {
      Error(0, "%s: error loading headers...\n", argv[0]);
      CleanupOnExit(1);
      return 1;
   } else {
      if (ifl) {
         FILE *fpd = fopen(argv[ifl], "r");
         if (fpd==0) {
            // The dictionary file was not created by CINT.
            // There mush have been an error.
            Error(0, "%s: error loading headers...\n", argv[0]);
            CleanupOnExit(1);
            return 1;
         }
         fclose(fpd);
      }
   }
   G__setglobalcomp(0); // G__NOLINK
#endif

   // We ran cint to load the in-memory database,
   // so that the I/O code can be properly generated.
   // So now let's call GCCXML if requested
   if (dict_type==kDictTypeGCCXML) {
      string gccxml_rootcint_call;
#ifndef ROOTBUILD
# ifndef ROOTBINDIR
      if (getenv("ROOTSYS")) {
         gccxml_rootcint_call=getenv("ROOTSYS");
#  ifdef WIN32
         gccxml_rootcint_call+="\\bin\\";
#  else
         gccxml_rootcint_call+="/bin/";
#  endif
      }
# else
      gccxml_rootcint_call=ROOTBINDIR;
#  ifdef WIN32
      gccxml_rootcint_call+="\\";
#  else
      gccxml_rootcint_call+="/";
#  endif
# endif
#else
# ifdef WIN32
      gccxml_rootcint_call="bin\\";
# else
      gccxml_rootcint_call="bin/";
# endif
#endif
      gccxml_rootcint_call+="genreflex-rootcint";

      for (int iarg=1; iarg<argc; ++iarg) {
         gccxml_rootcint_call+=" ";
         gccxml_rootcint_call+=argv[iarg];

         if (!strcmp(argv[iarg], "-c")) {
            for (i = 0; i < (int)path.size(); i++) {
               gccxml_rootcint_call+=" ";
               gccxml_rootcint_call+=path[i].c_str();
            }
            gccxml_rootcint_call+=" -DR__GCCXML";
#ifdef ROOTBUILD
            gccxml_rootcint_call+=" -DG__NOCINTDLL";
#endif
            gccxml_rootcint_call+=" -DTRUE=1";
            gccxml_rootcint_call+=" -DFALSE=0";
            gccxml_rootcint_call+=" -DR__EXTERN=extern";
            gccxml_rootcint_call+=" -Dexternalref=extern";
            gccxml_rootcint_call+=" -DSYSV";
#ifdef ROOTBUILD
            //gccxml_rootcint_call+=" base/inc/TROOT.h";
            gccxml_rootcint_call+=" base/inc/TObject.h";
            gccxml_rootcint_call+=" base/inc/TMemberInspector.h";
#else
            gccxml_rootcint_call+=" TObject.h";
            gccxml_rootcint_call+=" TMemberInspector.h";
#endif
         }
      }
      //printf("Calling %s\n", gccxml_rootcint_call.c_str());
      int rc=system(gccxml_rootcint_call.c_str());
      if (rc) {
         CleanupOnExit(rc);
         return rc;
      }
   }

   if (use_preprocessor && icc)
      ReplaceBundleInDict(argv[ifl], bundlename);

   // Check if code goes to stdout or cint file, use temporary file
   // for prepending of the rootcint generated code (STK)
   std::ofstream fileout;
   if (ifl) {
      tname = R__tmpnam();
      fileout.open(tname.c_str());
      dictSrcOut = &fileout;
      if (!(*dictSrcOut)) {
         Error(0, "rootcint: failed to open %s in main\n",
               tname.c_str());
         CleanupOnExit(1);
         return 1;
      }
   } else
      dictSrcOut = &std::cout;

   string main_dictname(argv[ifl]);
   size_t dh = main_dictname.rfind('.');
   if (dh != std::string::npos) {
      main_dictname.erase(dh);
   }
   // Need to replace all the characters not allowed in a symbol ...
   main_dictname = G__map_cpp_name((char*)main_dictname.c_str());

   time_t t = time(0);
   (*dictSrcOut) << "//"  << std::endl
                 << "// File generated by " << argv[0] << " at " << ctime(&t) << std::endl
                 << "// Do NOT change. Changes will be lost next time file is generated" << std::endl
                 << "//" << std::endl << std::endl

                 << "#define R__DICTIONARY_FILENAME " << main_dictname << std::endl
                 << "#include \"RConfig.h\" //rootcint 4834" << std::endl
                 << "#if !defined(R__ACCESS_IN_SYMBOL)" << std::endl
                 << "//Break the privacy of classes -- Disabled for the moment" << std::endl
                 << "#define private public" << std::endl
                 << "#define protected public" << std::endl
                 << "#endif" << std::endl
                 << std::endl;
#ifndef R__SOLARIS
   (*dictSrcOut) << "// Since CINT ignores the std namespace, we need to do so in this file." << std::endl
                 << "namespace std {} using namespace std;" << std::endl << std::endl;
   int linesToSkip = 16; // number of lines up to here.
#else
   int linesToSkip = 13; // number of lines up to here.
#endif

   (*dictSrcOut) << "#include \"TClass.h\"" << std::endl
                 << "#include \"TCintWithCling.h\"" << std::endl
                 << "#include \"TBuffer.h\"" << std::endl
                 << "#include \"TMemberInspector.h\"" << std::endl
                 << "#include \"TInterpreter.h\"" << std::endl
                 << "#include \"TVirtualMutex.h\"" << std::endl
                 << "#include \"TError.h\"" << std::endl << std::endl
                 << "#ifndef G__ROOT" << std::endl
                 << "#define G__ROOT" << std::endl
                 << "#endif" << std::endl << std::endl
                 << "#include \"RtypesImp.h\"" << std::endl
                 << "#include \"TIsAProxy.h\"" << std::endl
                 << "#include \"TFileMergeInfo.h\"" << std::endl;
   (*dictSrcOut) << std::endl;
#ifdef R__SOLARIS
   (*dictSrcOut) << "// Since CINT ignores the std namespace, we need to do so in this file." << std::endl
                 << "namespace std {} using namespace std;" << std::endl << std::endl;
#endif

   //---------------------------------------------------------------------------
   // Write schema evolution reelated headers and declarations
   //---------------------------------------------------------------------------
   if( !G__ReadRules.empty() || !G__ReadRawRules.empty() ) {
      (*dictSrcOut) << "#include \"TBuffer.h\"" << std::endl;
      (*dictSrcOut) << "#include \"TVirtualObject.h\"" << std::endl;
      (*dictSrcOut) << "#include <vector>" << std::endl;
      (*dictSrcOut) << "#include \"TSchemaHelper.h\"" << std::endl << std::endl;

      std::list<std::string>           includes;
      std::list<std::string>::iterator it;
      GetRuleIncludes( includes );
      for( it = includes.begin(); it != includes.end(); ++it )
         (*dictSrcOut) << "#include <" << *it << ">" << std::endl;
      (*dictSrcOut) << std::endl;
   }

   // Loop over all command line arguments and write include statements.
   // Skip options and any LinkDef.h.
   if (ifl && !icc) {
      for (i = ic; i < argc; i++) {
         if (*argv[i] != '-' && *argv[i] != '+' &&
             !(R__IsSelectionFile(argv[i])))
            (*dictSrcOut) << "#include \"" << argv[i] << "\"" << std::endl;
      }
      (*dictSrcOut) << std::endl;
   }

   string linkdefFilename;
   if (il) {
      linkdefFilename = argv[il];
   } else {
      bool found = Which(argv[il], linkdefFilename);
      if (!found) {
         Error(0, "%s: cannot open file %s\n", argv[0], argv[il]);
         CleanupOnExit(1);
         return 1;
      }
   }   

   SelectionRules selectionRules;

   if (requestAllSymbols) {
      selectionRules.SetDeep(true);
   } else if (R__IsSelectionXml(linkdefFilename.c_str())) {

      selectionRules.SetSelectionFileType(SelectionRules::kSelectionXMLFile);

      std::ifstream file(linkdefFilename.c_str());
      if(file.is_open()){
         Info(0,"Selection XML file");

         XMLReader xmlr;
         if (!xmlr.Parse(file, selectionRules)) {
            Error(0,"Parsing XML file %s",linkdefFilename.c_str());
         }
         else {
            Info(0,"XML file successfully parsed");
         }            
         file.close();
      }
      else {
         Error(0,"XML file %s couldn't be opened!",linkdefFilename.c_str());
      }

   } else if (R__IsLinkdefFile(linkdefFilename.c_str())) {

      std::ifstream file(linkdefFilename.c_str());
      if(file.is_open()){
         Info(0,"Linkdef file");

         file.close();
      }
      else {
         Error(0,"Linkdef file %s couldn't be opened!",linkdefFilename.c_str());
      }

      selectionRules.SetSelectionFileType(SelectionRules::kLinkdefFile);

      LinkdefReader ldefr;
      if (!ldefr.Parse(selectionRules, interpPragmaSource, clingArgs, getenv("LLVMDIR"))) {
         Error(0,"Parsing Linkdef file %s",linkdefFilename.c_str());
      }
      else {
         Info(0,"Linkdef file successfully parsed");
      }

   } else {

      Error(0,"Unrecognized selection file: %s",linkdefFilename.c_str());

   }

   selectionRules.SearchNames(interp);

   RScanner scan(selectionRules);
   clang::CompilerInstance* CI = interp.getCI();
   scan.Scan(CI->getASTContext());

   bool has_input_error = false;

// SELECTION LOOP
   // Check for error in the class layout before doing anything else.
   RScanner::ClassColl_t::const_iterator iter = scan.fSelectedClasses.begin();
   RScanner::ClassColl_t::const_iterator end = scan.fSelectedClasses.end();
   for( ; iter != end; ++iter) 
   {
      if (ClassInfo__HasMethod(*iter,"Streamer")) {
         if (iter->RequestNoInputOperator()) {
            int version = GetClassVersion(*iter);
            if (version!=0) {
               // Only Check for input operator is the object is I/O has
               // been requested.
               has_input_error |= CheckInputOperator(*iter,dicttype);
            }
         }
      }
      has_input_error |= !CheckClassDef(*iter);
   }

   if (has_input_error) {
      // Be a little bit makefile friendly and remove the dictionary in case of error.
      // We could add an option -k to keep the file even in case of error.
      CleanupOnExit(1);
      exit(1);
   }

   // 26-07-07
   // dont generate the showmembers if we only want
   // all the memfunc_setup stuff (stub-less calls)
   if(dicttype==0 || dicttype==1) {
      //
      // We will loop over all the classes several times.
      // In order we will call
      //
      //     WriteShadowClass
      //     WriteClassInit (code to create the TGenericClassInfo)
      //     check for constructor and operator input
      //     WriteClassFunctions (declared in ClassDef)
      //     WriteClassCode (Streamer,ShowMembers,Auxiliary functions)
      //

      //
      // Loop over all classes and write the Shadow class if needed
      //
      // Open LinkDef file for reading, so that we can process classes
      // in order of appearence in this file (STK)
      FILE *fpld = 0;
      if (!il) {
         // Open auto-generated file
         fpld = fopen(autold, "r");
      } else {
         // Open file specified on command line
         string filename;
         bool found = Which(argv[il], filename);
         if (!found) {
            Error(0, "%s: cannot open file %s\n", argv[0], argv[il]);
            CleanupOnExit(1);
            return 1;
         }
         fpld = fopen(filename.c_str(), "r");
      }
      if (!fpld) {
         Error(0, "%s: cannot open file %s\n", argv[0], il ? argv[il] : autold);
         CleanupOnExit(1);
         return 1;
      }

      // Read LinkDef file and process the #pragma link C++ ioctortype
      char consline[256];
      while (fgets(consline, 256, fpld)) {
         static const char* ioctorTokens[] = {"pragma", "link", "C++", "ioctortype", 0};
         size_t tokpos = 0;
         bool constype = ParsePragmaLine(consline, ioctorTokens, &tokpos);

         if (constype) {
            char *request = strtok(consline + tokpos, "-!+;");
            // just in case remove trailing space and tab
            while (*request == ' ') request++;
            int len = strlen(request)-1;
            while (request[len]==' ' || request[len]=='\t') request[len--] = '\0';
            request = Compress(request); //no space between tmpl arguments allowed
            AddConstructorType(request);

         }
      }
      rewind(fpld);
      AddConstructorType("TRootIOCtor");
      AddConstructorType("");

      const char* shadowNSName="ROOT";
      if (dict_type != kDictTypeCint)
         shadowNSName = "ROOT::Reflex";
      G__ShadowMaker myShadowMaker((*dictSrcOut), shadowNSName, NeedShadowClass,
                                   dict_type==kDictTypeCint ? NeedTypedefShadowClass : 0);
      shadowMaker = &myShadowMaker;

      G__ShadowMaker::VetoShadow(false);
      // coverity[fun_call_w_exception] - that's just fine.
      shadowMaker->WriteAllShadowClasses();

      //
      // Loop over all classes and create Streamer() & Showmembers() methods
      //

// SELECTION LOOP
      RScanner::NamespaceColl_t::const_iterator ns_iter = scan.fSelectedNamespaces.begin();
      RScanner::NamespaceColl_t::const_iterator ns_end = scan.fSelectedNamespaces.end();
      for( ; ns_iter != ns_end; ++ns_iter) {
         WriteNamespaceInit(*ns_iter);         
      }

      if (strstr(dictname,"rootcint_") != dictname) {
         // Modules only for "regular" dictionaries, not for cintdlls
         GenerateModule(dictname, pcmArgs);
      }

      iter = scan.fSelectedClasses.begin();
      end = scan.fSelectedClasses.end();
      for( ; iter != end; ++iter) 
      {
         if (!iter->GetRecordDecl()->isCompleteDefinition()) {
            Error(0,"A dictionary has been requested for %s but there is no declaration!\n",R__GetQualifiedName(* iter->GetRecordDecl()).c_str());
            continue;
         }
         if (iter->RequestOnlyTClass()) {
            fprintf(stderr,"Skipping %s\n",R__GetQualifiedName(* iter->GetRecordDecl()).c_str());
            // For now delay those for later.
            continue;
         }
         fprintf(stderr,"Seeing %s\n",R__GetQualifiedName(* iter->GetRecordDecl()).c_str());
         const clang::CXXRecordDecl* CRD = llvm::dyn_cast<clang::CXXRecordDecl>(iter->GetRecordDecl());
         if (CRD) {
            std::string qualname( CRD->getQualifiedNameAsString() );
            if (IsStdClass(*CRD) && 0 != TClassEdit::STLKind(CRD->getName().data() /* unqualified name without template arguement */) ) {
                  // coverity[fun_call_w_exception] - that's just fine.
               RStl::Instance().GenerateTClassFor( iter->GetRequestedName(), CRD );
            } else {
               WriteClassInit(*iter);
            }               
         }
      }

      //
      // Write all TBuffer &operator>>(...), Class_Name(), Dictionary(), etc.
      // first to allow template specialisation to occur before template
      // instantiation (STK)
      //
// SELECTION LOOP
      iter = scan.fSelectedClasses.begin();
      end = scan.fSelectedClasses.end();
      for( ; iter != end; ++iter) 
      {
         if (!iter->GetRecordDecl()->isCompleteDefinition()) {
            continue;
         }                       
         if (iter->RequestOnlyTClass()) {
            // For now delay those for later.
            continue;
         }
         const clang::CXXRecordDecl* cxxdecl = llvm::dyn_cast<clang::CXXRecordDecl>(iter->GetRecordDecl());
         if (cxxdecl && ClassInfo__HasMethod(*iter,"Class_Name")) {
            WriteClassFunctions(cxxdecl);
         }
      }

      // Keep track of classes processed by reading Linkdef file.
      // When all classes in LinkDef are done, loop over all classes known
      // to CINT output the ones that were not in the LinkDef. This can happen
      // in case "#pragma link C++ defined_in" is used.
      //const int kMaxClasses = 2000;
      //char *clProcessed[kMaxClasses];

// LINKDEF SELECTION LOOP
      // Loop to get the shadow class for the class marker 'RequestOnlyTClass' (but not the
      // STL class which is done via RStl::Instance().WriteClassInit(0);
      // and the ClassInit
      iter = scan.fSelectedClasses.begin();
      end = scan.fSelectedClasses.end();
      for( ; iter != end; ++iter) 
      {
         if (!iter->GetRecordDecl()->isCompleteDefinition()) {
            continue;
         }
         if (!iter->RequestOnlyTClass()) {
            continue;
         }
         G__ClassInfo clinfo( iter->GetRequestedName()[0] ? iter->GetRequestedName() : R__GetQualifiedName(*iter).c_str() );
         fprintf(stderr,"Doing create %s %d\n",R__GetQualifiedName(*iter).c_str(),NeedShadowClass(clinfo));
         if (NeedShadowClass(clinfo)) {
            (*dictSrcOut) << "namespace ROOT {" << std::endl
            << "   namespace Shadow {" << std::endl;
            // coverity[fun_call_w_exception] - that's just fine.
            shadowMaker->WriteShadowClass(clinfo);
            (*dictSrcOut) << "   } // Of namespace ROOT::Shadow" << std::endl
            << "} // Of namespace ROOT" << std::endl << std::endl;
         }
         if (G__ShadowMaker::IsSTLCont(clinfo.Name()) == 0 ) {
            WriteClassInit(*iter);
         }
      }
      // Loop to write all the ClassCode
      iter = scan.fSelectedClasses.begin();
      end = scan.fSelectedClasses.end();
      for( ; iter != end; ++iter) 
      {
         if (!iter->GetRecordDecl()->isCompleteDefinition()) {
            continue;
         }
         WriteClassCode(*iter);
      }

      //RStl::Instance().WriteStreamer(fp); //replaced by new Markus code
      // coverity[fun_call_w_exception] - that's just fine.
      RStl::Instance().WriteClassInit(0);

      fclose(fpld);

      if (!il) remove(autold);
      if (use_preprocessor) remove(bundlename.c_str());

   } // (stub-less calls)

   // Append CINT dictionary to file containing Streamers and ShowMembers
   if (ifl) {
      char line[BUFSIZ];
      FILE *fpd = fopen(argv[ifl], "r");
      FILE* fp = fopen(tname.c_str(), "a");

      if (fp && fpd)
         while (fgets(line, BUFSIZ, fpd))
            fprintf(fp, "%s", line);

      if (fp)  fclose(fp);
      if (fpd) fclose(fpd);

      // copy back to dictionary file
      fpd = fopen(argv[ifl], "w");
      fp  = fopen(tname.c_str(), "r");

      if (fp && fpd) {

         // make name of dict include file "aapDict.cxx" -> "aapDict.h"
         int  nl = 0;
         // 07-11-07
         // Include the temporaries here to get one file with everything
         char *s = strrchr(dictname, '.');
         if (s) *s = 0;
         string inclf(dictname); inclf += ".h";
         string inclfTmp1(dictname); inclfTmp1 += "Tmp1.cxx";
         string inclfTmp2(dictname); inclfTmp2 += "Tmp2.cxx";
         if (s) *s = '.';

         // during copy put dict include on top and remove later reference
         while (fgets(line, BUFSIZ, fp)) {
            if (!strncmp(line, "#include", 8) && strstr(line, "\" //newlink 3678 "))
               continue;
            fprintf(fpd, "%s", line);


            // 'linesToSkip' is because we want to put it after #defined private/protected
            if (++nl == linesToSkip && icc) {
               switch (dict_type) {
               case kDictTypeGCCXML:
                  fprintf(fpd, "#define G__DICTIONARY gccxml\n");
                  break;
               case kDictTypeReflex:
                  fprintf(fpd, "#define G__DICTIONARY reflex\n");
                  break;
               default:;
               }
               if (longheadername && dictpathname.length() ) {
                  fprintf(fpd, "#include \"%s/%s\"\n", dictpathname.c_str(), inclf.c_str());
               } else {
                  fprintf(fpd, "#include \"%s\"\n", inclf.c_str());
               }

               // 07-11-07
               // Put the includes to temporary files when generating the third dictionary
               if(dicttype==3){
                  if (longheadername && dictpathname.length() ) {
                     fprintf(fpd, "#include \"%s/%s\"\n", dictpathname.c_str(), inclfTmp1.c_str());
                     fprintf(fpd, "#include \"%s/%s\"\n", dictpathname.c_str(), inclfTmp2.c_str());
                  } else {
                     fprintf(fpd, "#include \"%s\"\n", inclfTmp1.c_str());
                     fprintf(fpd, "#include \"%s\"\n", inclfTmp2.c_str());
                  }
               }

               if (gNeedCollectionProxy) {
                  fprintf(fpd, "\n#include \"TCollectionProxyInfo.h\"");
               }
            }
         }
      }

      if (fp)  fclose(fp);
      if (fpd) fclose(fpd);
      remove(tname.c_str());
   }

   if (gLiblistPrefix.length()) {
      string liblist_filename = gLiblistPrefix + ".out";

      ofstream outputfile( liblist_filename.c_str(), ios::out );
      if (!outputfile) {
         Error(0,"%s: Unable to open output lib file %s\n",
               argv[0], liblist_filename.c_str());
      } else {
         const size_t endStr = gLibsNeeded.find_last_not_of(" \t");
         outputfile << gLibsNeeded.substr(0, endStr+1) << endl;
         // Add explicit delimiter
         outputfile << "# Now the list of classes\n";
// SELECTION LOOP
         G__ClassInfo clFile;
         clFile.Init();
         while (clFile.Next()) {
            if (clFile.Linkage() == G__CPPLINK && !(clFile.Property() & G__BIT_ISNAMESPACE) ) {
               outputfile << clFile.Fullname() << endl;
            }
         }
      }
   }

   G__setglobalcomp(-1);  // G__CPPLINK
   CleanupOnExit(0);

   G__exit(0);
   return 0;
}
