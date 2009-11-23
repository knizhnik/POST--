// -*- Mode: C++ -*-

//          BT.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/BTree/BT.h,v 1.1.1.1 1996/08/06 23:47:18 jmh Exp $

#ifndef BT_H
#define BT_H

#include "GiST.h"
#include "GiSTdb.h"
#include "BTentry.h"
#include "BTnode.h"
#include "BTpredicate.h"

class BT : public GiST
{
public:
  // optional, for debugging support
  GiSTobjid IsA() { return BT_CLASS; }
  BT(storage& sto) : db(sto) {} 


protected:
  // Required members
  GiSTnode  *CreateNode()  const { return new BTnode; }
  GiSTstore *CreateStore() const { return new GiSTdb(db); }

  // set special property
  int  IsOrdered()    const { return 1; }

  storage& db;
};

#endif










