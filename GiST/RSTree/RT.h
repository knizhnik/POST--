// -*- Mode: C++ -*-

//          RT.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/RSTree/RT.h,v 1.1.1.1 1996/08/06 23:47:24 jmh Exp $

#ifndef RT_H
#define RT_H

#include "GiST.h"
#include "GiSTdb.h"
#include "RTentry.h"
#include "RTnode.h"
#include "RTpredicate.h"

class RT : public GiST
{
public:
  GiSTobjid IsA() { return RT_CLASS; }
  RT(storage& sto) : db(sto) {} 

protected:
  GiSTnode  *CreateNode()  const { return new RTnode; }
  GiSTstore *CreateStore() const { return new GiSTdb(db); }

  // R*-trees use forced reinsert, and the top nodes to remove are chosen
  // specially.
  int  ForcedReinsert()    const { return 1; }
  GiSTlist<GiSTentry*> RemoveTop(GiSTnode *node);

  storage& db;
};

#endif

