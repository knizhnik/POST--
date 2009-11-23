//-< ARRAY.CXX >-----------------------------------------------------*--------*
// POST++                     Version 1.0        (c) 1998  GARRET    *     ?  *
// (Persistent Object Storage)                                       *   /\|  *
//                                                                   *  /  \  *
//                          Created:     17-Mar-98    K.A. Knizhnik  * / [] \ *
//                          Last update: 17-Mar-98    K.A. Knizhnik  * GARRET *
//-------------------------------------------------------------------*--------*
// Registration of array template instantiations
//-------------------------------------------------------------------*--------*

#define INSIDE_POST

#include "array.h"

BEGIN_POST_NAMESPACE

REGISTER_TEMPLATE(ArrayOfChar);
REGISTER_TEMPLATE(ArrayOfByte);
REGISTER_TEMPLATE(ArrayOfWord);
REGISTER_TEMPLATE(ArrayOfInt);
REGISTER_TEMPLATE(ArrayOfNat);
REGISTER_TEMPLATE(ArrayOfFloat);
REGISTER_TEMPLATE(ArrayOfDouble);
REGISTER_TEMPLATE(ArrayOfObject);
REGISTER_TEMPLATE(MatrixOfChar);
REGISTER_TEMPLATE(MatrixOfByte);
REGISTER_TEMPLATE(MatrixOfWord);
REGISTER_TEMPLATE(MatrixOfInt);
REGISTER_TEMPLATE(MatrixOfNat);
REGISTER_TEMPLATE(MatrixOfFloat);
REGISTER_TEMPLATE(MatrixOfDouble);
REGISTER(String);
REGISTER(DynString);
REGISTER_TEMPLATE(DynArrayOfChar);
REGISTER_TEMPLATE(DynArrayOfByte);
REGISTER_TEMPLATE(DynArrayOfWord);
REGISTER_TEMPLATE(DynArrayOfInt);
REGISTER_TEMPLATE(DynArrayOfNat);
REGISTER_TEMPLATE(DynArrayOfFloat);
REGISTER_TEMPLATE(DynArrayOfDouble);
REGISTER_TEMPLATE(DynArrayOfObject);

END_POST_NAMESPACE
