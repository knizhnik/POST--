// -*- Mode: C++ -*-

//          RTnode.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/RTree/RTnode.h,v 1.1.1.1 1996/08/06 23:47:26 jmh Exp $

#ifndef RTNODE_H
#define RTNODE_H

#include "RTentry.h"

class RTnode : public GiSTnode {
public:
    // constructors, destructors, etc.
    GiSTobjid IsA() const { return RTNODE_CLASS; }
    GiSTobject *Copy() const { return new RTnode(*this); }

    // two of the basic GiST methods 
    GiSTnode *PickSplit();
    GiSTentry* Union() const;

    // required support methods
    GiSTentry *CreateEntry() const { return new RTentry; }
    int FixedLength() const { return sizeof(RTkey); }
};

#endif






