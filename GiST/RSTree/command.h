// -*- Mode: C++ -*- ----------------------------------------
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /usr/local/devel/GiST/libGiST/RSTree/command.h,v 1.1.1.1 1996/08/06 23:47:23 jmh Exp $
//
//  command.h
//  Test Program: Command Parser .h
//
// ----------------------------------------------------------

#ifndef COMMAND_H
#define COMMAND_H


class Record : public object { 
  public:
    int value;

    CLASSINFO(Record, NO_REFS);
};

void CommandCreate(const char *method,
		   const char *table);

void CommandDrop(const char *table);

void CommandOpen(const char *method, const char *table);

void CommandClose(const char *table);

void CommandSelect(const char *table,
		   const GiSTpredicate& pred);

void CommandDelete(const char *table,
		   const GiSTpredicate& pred);

void CommandInsert(const char *table,
		   const RTkey& key,
		   int ptr);

void CommandDump(const char *table, GiSTpage page);

void CommandQuit();

void CommandDebug();

void CommandHelp();

#endif
