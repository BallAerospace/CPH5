////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Ball Aerospace & Technologies Corp. All Rights Reserved.
//
// This program is free software; you can modify and/or redistribute it under
// the terms found in the accompanying LICENSE.txt file.
////////////////////////////////////////////////////////////////////////////////

// This file should be used in conjunction with the cph5externs.cpp file, and
// should be included immediately after the inclusion of cph5.h.

// For a more specialized set of externs, copy this file and the analogous
// cpp file and customize (and rename).

#include "cph5.h"

// The cpp file just redefines CPH5_EXTERN to be nothing and includes
// this file. As such if externs need be changed it only affects this file.

// Note that using this is a double-edged sword. It will help reduce
// compile times if the below template specializations are used AND you
// are not performing a clean rebuild. A clean build requires all the
// specializations below to be compiled so it will likely be longer.

// The 'extern template ...' syntax is a feature as of C++11.
#ifndef CPH5_NO_EXTERN
#define CPH5_EXTERN extern
#else
#define CPH5_EXTERN
#endif


// Datasets. Up to three dimensions for each basic type
CPH5_EXTERN template class CPH5Dataset<uint8_t, 0>;
CPH5_EXTERN template class CPH5Dataset<uint8_t, 1>;
CPH5_EXTERN template class CPH5Dataset<uint8_t, 2>;
CPH5_EXTERN template class CPH5Dataset<uint8_t, 3>;

CPH5_EXTERN template class CPH5Dataset<uint16_t, 0>;
CPH5_EXTERN template class CPH5Dataset<uint16_t, 1>;
CPH5_EXTERN template class CPH5Dataset<uint16_t, 2>;
CPH5_EXTERN template class CPH5Dataset<uint16_t, 3>;

CPH5_EXTERN template class CPH5Dataset<uint32_t, 0>;
CPH5_EXTERN template class CPH5Dataset<uint32_t, 1>;
CPH5_EXTERN template class CPH5Dataset<uint32_t, 2>;
CPH5_EXTERN template class CPH5Dataset<uint32_t, 3>;

CPH5_EXTERN template class CPH5Dataset<uint64_t, 0>;
CPH5_EXTERN template class CPH5Dataset<uint64_t, 1>;
CPH5_EXTERN template class CPH5Dataset<uint64_t, 2>;
CPH5_EXTERN template class CPH5Dataset<uint64_t, 3>;

CPH5_EXTERN template class CPH5Dataset<int8_t, 0>;
CPH5_EXTERN template class CPH5Dataset<int8_t, 1>;
CPH5_EXTERN template class CPH5Dataset<int8_t, 2>;
CPH5_EXTERN template class CPH5Dataset<int8_t, 3>;

CPH5_EXTERN template class CPH5Dataset<int16_t, 0>;
CPH5_EXTERN template class CPH5Dataset<int16_t, 1>;
CPH5_EXTERN template class CPH5Dataset<int16_t, 2>;
CPH5_EXTERN template class CPH5Dataset<int16_t, 3>;

CPH5_EXTERN template class CPH5Dataset<int32_t, 0>;
CPH5_EXTERN template class CPH5Dataset<int32_t, 1>;
CPH5_EXTERN template class CPH5Dataset<int32_t, 2>;
CPH5_EXTERN template class CPH5Dataset<int32_t, 3>;

CPH5_EXTERN template class CPH5Dataset<int64_t, 0>;
CPH5_EXTERN template class CPH5Dataset<int64_t, 1>;
CPH5_EXTERN template class CPH5Dataset<int64_t, 2>;
CPH5_EXTERN template class CPH5Dataset<int64_t, 3>;

CPH5_EXTERN template class CPH5Dataset<float, 0>;
CPH5_EXTERN template class CPH5Dataset<float, 1>;
CPH5_EXTERN template class CPH5Dataset<float, 2>;
CPH5_EXTERN template class CPH5Dataset<float, 3>;

CPH5_EXTERN template class CPH5Dataset<double, 0>;
CPH5_EXTERN template class CPH5Dataset<double, 1>;
CPH5_EXTERN template class CPH5Dataset<double, 2>;
CPH5_EXTERN template class CPH5Dataset<double, 3>;

// Compound type and members.
CPH5_EXTERN class CPH5CompType;

CPH5_EXTERN template class CPH5CompMember<uint8_t>;
CPH5_EXTERN template class CPH5CompMember<uint16_t>;
CPH5_EXTERN template class CPH5CompMember<uint32_t>;
CPH5_EXTERN template class CPH5CompMember<uint64_t>;
CPH5_EXTERN template class CPH5CompMember<int8_t>;
CPH5_EXTERN template class CPH5CompMember<int16_t>;
CPH5_EXTERN template class CPH5CompMember<int32_t>;
CPH5_EXTERN template class CPH5CompMember<int64_t>;
CPH5_EXTERN template class CPH5CompMember<float>;
CPH5_EXTERN template class CPH5CompMember<double>;


// Not doing compound member arrays as the template
// parameter is the size.

