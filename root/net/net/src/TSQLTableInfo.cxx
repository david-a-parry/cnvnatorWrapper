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
// TSQLTableInfo
//
// Contains information about table and table columns.
// For MySQL additional information like engine type, 
// creation and last update time is provided
//                                                                      
////////////////////////////////////////////////////////////////////////////////



#include "TSQLTableInfo.h"

#include "TSQLColumnInfo.h"
#include "TList.h"
#include "TROOT.h"
#include "Riostream.h"

ClassImp(TSQLTableInfo)

//______________________________________________________________________________
TSQLTableInfo::TSQLTableInfo() : 
   TNamed(),
   fColumns(0),
   fEngine(),
   fCreateTime(),
   fUpdateTime()
{
   // default constructor
}

//______________________________________________________________________________
TSQLTableInfo::TSQLTableInfo(const char* tablename, 
                             TList* columns,
                             const char* comment,
                             const char* engine,
                             const char* create_time,
                             const char* update_time) : 
   TNamed(tablename, comment),
   fColumns(columns),
   fEngine(engine),
   fCreateTime(create_time),
   fUpdateTime(update_time)
{
   // normal constructor
   
}

//______________________________________________________________________________
TSQLTableInfo::~TSQLTableInfo()
{
   // destructor
   
   if (fColumns!=0) {
      fColumns->Delete();
      delete fColumns;
      fColumns = 0;
   }
}

//______________________________________________________________________________
void TSQLTableInfo::Print(Option_t*) const
{
   // Prints table and table columns info
   
   TROOT::IndentLevel();
   cout << "Table:" << GetName();
   
   if ((GetTitle()!=0) && (strlen(GetTitle())!=0))
      cout << " comm:'" << GetTitle() << "'";
   
   if (fEngine.Length()>0)
      cout << " engine:" << fEngine;

   if (fCreateTime.Length()>0)
      cout << " create:" << fCreateTime;

   if (fUpdateTime.Length()>0)
      cout << " update:" << fUpdateTime;
   
   cout << endl;
    
   TROOT::IncreaseDirLevel();
   if (fColumns!=0)
      fColumns->Print("*");
   TROOT::DecreaseDirLevel();
}

//______________________________________________________________________________
TSQLColumnInfo* TSQLTableInfo::FindColumn(const char* columnname)
{
   // Return column info object of given name
   
   if ((columnname==0) || (fColumns==0)) return 0;
   
   return dynamic_cast<TSQLColumnInfo*> (fColumns->FindObject(columnname));
   
}
