// -*- Mode: C++ -*-

//          RT.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/RTree/RT.h,v 1.1.1.1 1996/08/06 23:47:25 jmh Exp $

#define RT_H

#include "GiST.h"
#include "GiSTdb.h"
#include "RTentry.h"
#include "RTnode.h"
#include "RTpredicate.h"

class RT : public GiST
{
public:
  // optional, for debugging support
  GiSTobjid IsA() { return RT_CLASS; }
  RT(storage& sto) : db(sto) {} 

protected:
  // Required members
  GiSTnode  *CreateNode()  const { return new RTnode; }
  GiSTstore *CreateStore() const { return new GiSTdb(db); }

  storage& db;
};







