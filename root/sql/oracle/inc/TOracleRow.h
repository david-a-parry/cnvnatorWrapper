// @(#)root/physics:$Id$
// Author: Yan Liu and Shaowen Wang   23/11/04

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TOracleRow
#define ROOT_TOracleRow

#ifndef ROOT_TSQLRow
#include "TSQLRow.h"
#endif

#if !defined(__CINT__)
#ifndef R__WIN32
#include <sys/time.h>
#endif
#include <occi.h>
#ifdef CONST
#undef CONST
#endif
#else
namespace oracle { namespace occi {
class ResultSet;
class MetaData;
   }}
#endif

class TOracleRow : public TSQLRow {

private:
   oracle::occi::ResultSet *fResult;      // current result set
   std::vector<oracle::occi::MetaData> *fFieldInfo;   // metadata for columns
   Int_t                    fFieldCount;
   char                   **fFieldsBuffer;

   Bool_t  IsValid(Int_t field);

   TOracleRow(const TOracleRow&);            // Not implemented.
   TOracleRow &operator=(const TOracleRow&); // Not implemented.
   
protected:
   void        GetRowData();

public:
   TOracleRow(oracle::occi::ResultSet *rs,
              std::vector<oracle::occi::MetaData> *fieldMetaData);
   ~TOracleRow();

   void        Close(Option_t *opt="");
   ULong_t     GetFieldLength(Int_t field);
   const char *GetField(Int_t field);

   ClassDef(TOracleRow,0)  // One row of Oracle query result
};

#endif
