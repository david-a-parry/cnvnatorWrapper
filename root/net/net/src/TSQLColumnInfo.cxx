// @(#)root/net:$Id$
// Author: Sergey Linev   31/05/2006

/*************************************************************************
 * Copyright (C) 1995-2006, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                      
// TSQLColumnInfo
//
// Contains information about single column from SQL table
// Has following methods:
//   GetTypeName() - field type name in string form as it is reported by correspondent
//          database method. Some databases providing full type name like "numeric(20)",
//          other showing only "NUMERIC". As a result, one cannot use this string directly
//          to create new field of similar types in other table 
//   IsNullable() - says if field value can be NULL or not
//   GetSQLType() - returns kind of sql type. Possible values:
//      TSQLServer::kSQL_NONE        data type unknown
//      TSQLServer::kSQL_CHAR        CHAR(n) - string with fixed length n
//      TSQLServer::kSQL_VARCHAR     VARCHAR(n) - string with variable length upto n
//      TSQLServer::kSQL_INTEGER     INTEGER, INT, TINYINT - any integer types
//      TSQLServer::kSQL_FLOAT       FLOAT - float value
//      TSQLServer::kSQL_DOUBLE      DOUBLE - double precision value
//      TSQLServer::kSQL_NUMERIC     NUMERIC(n,s), NUMBER(n,s) - numeric values with length and precion
//      TSQLServer::kSQL_BINARY      BLOB, VARBINARY  - binary data (vriable or fixed size)
//      TSQLServer::kSQL_TIMESTAMP   TIMESTAMP - time and date stamp
//   GetSize() - size of field in database. -1 if not known.
//   GetLength() - length argument in type declaration like CHAR(len) or NUMERIC(len), -1 if not defined
//   GetScale() - second argument in declarations like NUMERIC(len, s), -1 if not defined
//   GetSigned() - is type signed(==1) or unsigned(==0), -1 if not defined
//                                                                      
////////////////////////////////////////////////////////////////////////////////

#include "TSQLColumnInfo.h"
#include "TSQLServer.h"
#include "TROOT.h"
#include "Riostream.h"

ClassImp(TSQLColumnInfo)

//______________________________________________________________________________
TSQLColumnInfo::TSQLColumnInfo() : 
   TNamed(),
   fTypeName(),
   fSQLType(-1),
   fSize(-1),
   fLength(-1),
   fScale(-1),
   fSigned(-1),
   fNullable(kFALSE)
{
   // default contructor
}

//______________________________________________________________________________
TSQLColumnInfo::TSQLColumnInfo(const char* columnname,
                               const char* sqltypename,
                               Bool_t nullable,
                               Int_t sqltype,
                               Int_t size,
                               Int_t length,
                               Int_t scale,
                               Int_t sign) :
   TNamed(columnname,"column information"),
   fTypeName(sqltypename),
   fSQLType(sqltype),
   fSize(size),
   fLength(length),
   fScale(scale),
   fSigned(sign),
   fNullable(nullable)
{
   // normal constructor
}

//______________________________________________________________________________
void TSQLColumnInfo::Print(Option_t*) const
{
   // Prints column information to standard output
   
   TROOT::IndentLevel();
   cout << "Column: " << GetName() 
        << " type:'" << fTypeName << "'"; 
   if (fSQLType>=0) {
      cout << " typeid:";
      switch (fSQLType) {
         case TSQLServer::kSQL_CHAR : cout << "kSQL_CHAR"; break;
         case TSQLServer::kSQL_VARCHAR : cout << "kSQL_VARCHAR"; break;
         case TSQLServer::kSQL_INTEGER : cout << "kSQL_INTEGER"; break;
         case TSQLServer::kSQL_FLOAT : cout << "kSQL_FLOAT"; break;
         case TSQLServer::kSQL_DOUBLE : cout << "kSQL_DOUBLE"; break;
         case TSQLServer::kSQL_NUMERIC : cout << "kSQL_NUMERIC"; break;
         case TSQLServer::kSQL_BINARY : cout << "kSQL_BINARY"; break;
         case TSQLServer::kSQL_TIMESTAMP : cout << "kSQL_TIMESTAMP"; break;
         default: cout << fSQLType;
      }
   }
   cout << " nullable:" << (fNullable ? "yes" : "no");
   if (fSize>=0) cout << " size:" << fSize;
   if (fLength>=0) cout << " len:" << fLength;
   if (fScale>=0) cout << " scale:" << fScale;
   if (fSigned>=0) { 
      if (fSigned==0)
         cout << " unsigned";
      else
         cout << " signed";
   }
   cout << endl;
}
