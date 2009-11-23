// Mode: -*- C++ -*-

//          GiSTfile.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/libGiST/GiSTfile.h,v 1.1.1.1 1996/08/06 23:47:21 jmh Exp $

#ifndef GISTFILE_H
#define GISTFILE_H

#include "GiSTstore.h"


// GiSTfile is a simple storage class for GiSTs to work over 
// UNIX/NT files.

class GiSTfile : public GiSTstore {
public:
  GiSTfile() : GiSTstore() {}

  void Create(const char *filename);
  void Open(const char *filename);
  void Close();

  void Read(GiSTpage page, char *buf);
  void Write(GiSTpage page, const char *buf);
  GiSTpage Allocate();
  void Deallocate(GiSTpage page);
  void Sync() {}
  int PageSize() const { return 8192; }

private:
  int fileHandle;
};

#endif
