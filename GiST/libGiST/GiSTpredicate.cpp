// -*- Mode: C++ -*-

//          GiSTpredicate.cpp
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/libGiST/GiSTpredicate.cpp,v 1.1.1.1 1996/08/06 23:47:21 jmh Exp $

#include "GiST.h"

int PtrPredicate::Consistent(const GiSTentry& entry) const
{
    return !entry.IsLeaf() || entry.Ptr() == page;
}

GiSTobject* PtrPredicate::Copy() const
{
    return new PtrPredicate(page);
}

#ifdef PRINTING_OBJECTS
void PtrPredicate::Print(ostream& os) const
{
    os << "ptr = " << page;
}
#endif
