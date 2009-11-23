// -*- Mode: C++ -*-

//          GiSTcursor.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/libGiST/GiSTcursor.h,v 1.1.1.1 1996/08/06 23:47:21 jmh Exp $

#ifndef GISTCURSOR_H
#define GISTCURSOR_H

#include "GiSTdefs.h"
#include "GiSTentry.h"
#include "GiSTpredicate.h"
#include "GiSTlist.h"
#include "GiSTpath.h"

class GiST;

class GiSTcursor : public GiSTobject
{
  public:
    GiSTcursor(const GiST& gist, const GiSTpredicate& query);
    GiSTentry *Next();
    GiSTobjid IsA() const { return GISTCURSOR_CLASS; }
    ~GiSTcursor();
    const GiSTpath& Path() const;
  private:
    const GiST& gist;
    GiSTpath path;
    GiSTlist<GiSTentry*> stack;
    int first;
    GiSTpredicate *query;
    int lastlevel;
};

#endif
