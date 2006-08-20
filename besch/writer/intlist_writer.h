//@ADOC
/////////////////////////////////////////////////////////////////////////////
//
//  intlist_writer.h
//
//  (c) 2002 by Volker Meyer, Lohsack 1, D-23843 Lohsack
//
//---------------------------------------------------------------------------
//     Project: MakeObj                      Compiler: MS Visual C++ v6.00
//  SubProject: ...                              Type: C/C++ Header
//  $Workfile:: intlist_writer.h     $       $Author: hajo $
//  $Revision: 1.1 $         $Date: 2002/09/18 19:13:22 $
//---------------------------------------------------------------------------
//  Module Description:
//      ...
//
//---------------------------------------------------------------------------
//  Revision History:
//  $Log: intlist_writer.h,v $
//  Revision 1.1  2002/09/18 19:13:22  hajo
//  Volker: new config system
//
//
/////////////////////////////////////////////////////////////////////////////
//@EDOC
#ifndef __INTLIST_WRITER_H
#define __INTLIST_WRITER_H

/////////////////////////////////////////////////////////////////////////////
//
//  includes
//
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "obj_writer.h"
#include "../objversion.h"

/////////////////////////////////////////////////////////////////////////////
//
//  forward declarations
//
/////////////////////////////////////////////////////////////////////////////

template<class T> class slist_tpl;
class obj_node_t;

//@ADOC
/////////////////////////////////////////////////////////////////////////////
//  class:
//      intlist_writer_t()
//
//---------------------------------------------------------------------------
//  Description:
//      ...
/////////////////////////////////////////////////////////////////////////////
//@EDOC
class intlist_writer_t : public obj_writer_t {
    static intlist_writer_t the_instance;

    intlist_writer_t() { register_writer(false); }
public:
    static intlist_writer_t *instance() { return &the_instance; }

    virtual obj_type get_type() const { return obj_intlist; }
    virtual const char *get_type_name() const { return "intlist"; }

    void write_obj(FILE *fp, obj_node_t &parent, const slist_tpl <int> &values);
};

/////////////////////////////////////////////////////////////////////////////
//@EOF
/////////////////////////////////////////////////////////////////////////////
#endif // __INTLIST_WRITER_H
