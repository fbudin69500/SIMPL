/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <stdlib.h>

#include <iostream>
#include <cmath>

#include <QtCore/QJsonValue>

#include "SIMPLib/Utilities/ParameterSettings.h"
#include "SIMPLib/Utilities/UnitTestSupport.hpp"

#include "SIMPLTestFileLocations.h"

class ParameterSettingsTest
{

public:
  ParameterSettingsTest()
  {
  }

  virtual ~ParameterSettingsTest()
  {
  }


  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QJsonValue GenerateJSonValue(double value)
  {
    return QJsonValue(value);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QJsonValue GenerateStringJSonValue(double value)
  {
    return ParameterSettings::ToJson(value, false);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QJsonValue GenerateConstrainedJSonValue(double value, double min, double max)
  {
    return ParameterSettings::ToJson(value, min, max);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  QJsonValue GenerateConstrainedJSonValue(double value, double min, double max, bool locked)
  {
    return ParameterSettings::ToJson(value, min, max, locked);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void IntValueTest()
  {
    int originalValue = 5;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueInt(json), originalValue)

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueInt(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueInt(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueInt(json), originalValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void FloatValueTest()
  {
    float originalValue = 5.0f;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueFloat(json), originalValue)

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueFloat(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueFloat(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueFloat(json), originalValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void DoubleValueTest()
  {
    double originalValue = 5.0;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueDouble(json), originalValue)

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueDouble(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueDouble(json), originalValue)

    json = GenerateConstrainedJSonValue(originalValue, 0, 10, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetValueDouble(json), originalValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void IntMinTest()
  {
    int originalValue = 5;
    int minValue = 0;
    int maxValue = 10;
    
    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinInt(json), std::numeric_limits<int>::min())

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinInt(json), std::numeric_limits<int>::min())

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinInt(json), minValue)

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinInt(json), minValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void FloatMinTest()
  {
    float originalValue = 5.0f;
    float minValue = 0.0f;
    float maxValue = 10.0f;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinFloat(json), std::numeric_limits<float>::min())

      json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinFloat(json), std::numeric_limits<float>::min())

      json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinFloat(json), minValue)

      json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinFloat(json), minValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void DoubleMinTest()
  {
    double originalValue = 5.0;
    double minValue = 0.0;
    double maxValue = 10.0;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinDouble(json), std::numeric_limits<double>::min())

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinDouble(json), std::numeric_limits<double>::min())

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinDouble(json), minValue)

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMinDouble(json), minValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void IntMaxTest()
  {
    int originalValue = 5;
    int minValue = 0;
    int maxValue = 10;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxInt(json), std::numeric_limits<int>::max())

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxInt(json), std::numeric_limits<int>::max())

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxInt(json), maxValue)

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxInt(json), maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void FloatMaxTest()
  {
    float originalValue = 5.0f;
    float minValue = 0.0f;
    float maxValue = 10.0f;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxFloat(json), std::numeric_limits<float>::max())

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxFloat(json), std::numeric_limits<float>::max())

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxFloat(json), maxValue)

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxFloat(json), maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void DoubleMaxTest()
  {
    double originalValue = 5.0;
    double minValue = 0.0;
    double maxValue = 10.0;

    QJsonValue json = GenerateJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxDouble(json), std::numeric_limits<double>::max())

    json = GenerateStringJSonValue(originalValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxDouble(json), std::numeric_limits<double>::max())

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxDouble(json), maxValue)

    json = GenerateConstrainedJSonValue(originalValue, minValue, maxValue, false);
    DREAM3D_REQUIRE_EQUAL(ParameterSettings::GetMaxDouble(json), maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void ReadIntTest()
  {
    int value = 5;
    int minValue = 0;
    int maxValue = 10;

    int valueRead;
    int minValueRead;
    int maxValueRead;

    QJsonValue json = GenerateJSonValue(value);
    ParameterSettings::ReadJsonInt(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<int>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<int>::max())

    json = GenerateStringJSonValue(value);
    ParameterSettings::ReadJsonInt(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<int>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<int>::max())

    json = GenerateConstrainedJSonValue(value, minValue, maxValue);
    ParameterSettings::ReadJsonInt(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, minValue)
    DREAM3D_REQUIRE_EQUAL(maxValueRead, maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void ReadFloatTest()
  {
    float value = 5.0f;
    float minValue = 0.0f;
    float maxValue = 10.0f;

    float valueRead;
    float minValueRead;
    float maxValueRead;

    QJsonValue json = GenerateJSonValue(value);
    ParameterSettings::ReadJsonFloat(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<float>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<float>::max())

    json = GenerateStringJSonValue(value);
    ParameterSettings::ReadJsonFloat(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<float>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<float>::max())

    json = GenerateConstrainedJSonValue(value, minValue, maxValue);
    ParameterSettings::ReadJsonFloat(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, minValue)
    DREAM3D_REQUIRE_EQUAL(maxValueRead, maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void ReadDoubleTest()
  {
    double value = 5.0;
    double minValue = 0.0;
    double maxValue = 10.0;

    double valueRead;
    double minValueRead;
    double maxValueRead;

    QJsonValue json = GenerateJSonValue(value);
    ParameterSettings::ReadJsonDouble(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<double>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<double>::max())

    json = GenerateStringJSonValue(value);
    ParameterSettings::ReadJsonDouble(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, std::numeric_limits<double>::min())
    DREAM3D_REQUIRE_EQUAL(maxValueRead, std::numeric_limits<double>::max())

    json = GenerateConstrainedJSonValue(value, minValue, maxValue);
    ParameterSettings::ReadJsonDouble(json, valueRead, minValueRead, maxValueRead);
    DREAM3D_REQUIRE_EQUAL(valueRead, value)
    DREAM3D_REQUIRE_EQUAL(minValueRead, minValue)
    DREAM3D_REQUIRE_EQUAL(maxValueRead, maxValue)
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void operator()()
  {
    std::cout << "#### ParameterSettingsTest Starting ####" << std::endl;

    int err = EXIT_SUCCESS;
    DREAM3D_REGISTER_TEST(IntValueTest())
    DREAM3D_REGISTER_TEST(FloatValueTest())
    DREAM3D_REGISTER_TEST(DoubleValueTest())

    DREAM3D_REGISTER_TEST(IntMinTest())
    DREAM3D_REGISTER_TEST(FloatMinTest())
    DREAM3D_REGISTER_TEST(DoubleMinTest())

    DREAM3D_REGISTER_TEST(IntMaxTest())
    DREAM3D_REGISTER_TEST(FloatMaxTest())
    DREAM3D_REGISTER_TEST(DoubleMaxTest())

    DREAM3D_REGISTER_TEST(ReadIntTest())
    DREAM3D_REGISTER_TEST(ReadFloatTest())
    DREAM3D_REGISTER_TEST(ReadDoubleTest())
  }

private:
  ParameterSettingsTest(const ParameterSettingsTest&); // Copy Constructor Not Implemented
  void operator=(const ParameterSettingsTest&); // Operator '=' Not Implemented
};