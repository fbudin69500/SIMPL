/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#ifndef _AbstractDataParser_hpp_
#define _AbstractDataParser_hpp_

#include <QtCore/QString>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/DataArrays/StringDataArray.hpp"
#include "SIMPLib/SIMPLib.h"

#include "SIMPLib/CoreFilters/util/ParserFunctors.hpp"

class AbstractDataParser
{
public:
  SIMPL_SHARED_POINTERS(AbstractDataParser)
  SIMPL_TYPE_MACRO(AbstractDataParser)

  virtual ~AbstractDataParser()
  {
  }

  SIMPL_INSTANCE_STRING_PROPERTY(ColumnName)
  SIMPL_INSTANCE_PROPERTY(int, ColumnIndex)
  SIMPL_VIRTUAL_INSTANCE_PROPERTY(IDataArray::Pointer, DataArray)

  virtual IDataArray::Pointer initializeNewDataArray(size_t numTuples, const QString& name, bool allocate)
  {
    return IDataArray::NullPointer();
  }

  virtual ParserFunctor::ErrorObject parse(const QString& token, size_t index) = 0;

protected:
  AbstractDataParser()
  {
  }

private:
  AbstractDataParser(const AbstractDataParser&); // Copy Constructor Not Implemented
  void operator=(const AbstractDataParser&);     // Move assignment Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template <typename ArrayType, class F> class Parser : public AbstractDataParser
{
public:
  typedef Parser<ArrayType, F> SelfType;

  SIMPL_SHARED_POINTERS(SelfType)
  SIMPL_TYPE_MACRO(SelfType)

  static Pointer New(typename ArrayType::Pointer ptr, const QString& name, int colIndex)
  {
    Pointer sharedPtr(new Parser(ptr, name, colIndex));
    return sharedPtr;
  }

  virtual ~Parser()
  {
  }

  static IDataArray::Pointer InitializeNewDataArray(size_t numTuples, const QString& name, bool allocate)
  {
    typename ArrayType::Pointer array = ArrayType::CreateArray(numTuples, name, allocate);
    if(allocate)
    {
      array->initializeWithZeros();
    }
    return array;
  }

  void setDataArray(IDataArray::Pointer value)
  {
    AbstractDataParser::setDataArray(value);
    m_Ptr = std::dynamic_pointer_cast<ArrayType>(value);
  }

  virtual ParserFunctor::ErrorObject parse(const QString& token, size_t index)
  {
    ParserFunctor::ErrorObject obj;
    obj.ok = true;
    (*m_Ptr).setValue(index, F()(token, obj));
    return obj;
  }

protected:
  Parser(typename ArrayType::Pointer ptr, const QString& name, int index)
  {
    setColumnName(name);
    setColumnIndex(index);
    setDataArray(ptr);
    m_Ptr = ptr;
  }

private:
  typename ArrayType::Pointer m_Ptr;

  Parser(const Parser&);         // Copy Constructor Not Implemented
  void operator=(const Parser&); // Move assignment Not Implemented
};

typedef Parser<Int8ArrayType, Int8Functor> Int8ParserType;
typedef Parser<UInt8ArrayType, UInt8Functor> UInt8ParserType;

typedef Parser<Int16ArrayType, Int16Functor> Int16ParserType;
typedef Parser<UInt16ArrayType, UInt16Functor> UInt16ParserType;

typedef Parser<Int32ArrayType, Int32Functor> Int32ParserType;
typedef Parser<UInt32ArrayType, UInt32Functor> UInt32ParserType;

typedef Parser<Int64ArrayType, Int64Functor> Int64ParserType;
typedef Parser<UInt64ArrayType, UInt64Functor> UInt64ParserType;

typedef Parser<FloatArrayType, FloatFunctor> FloatParserType;
typedef Parser<DoubleArrayType, DoubleFunctor> DoubleParserType;

typedef Parser<StringDataArray, StringFunctor> StringParserType;

#endif /* DATAPARSER_HPP_ */
