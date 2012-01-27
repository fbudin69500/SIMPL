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


#include <stdlib.h>

#include <iostream>
#include <string>

#include "MXA/Common/LogTime.h"
#include "MXA/Utilities/MXADir.h"
#include "MXA/Utilities/MXAFileInfo.h"

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/AbstractPipeline.h"
#include "DREAM3DLib/Common/Observer.h"
#include "DREAM3DLib/VTKUtils/VTKFileWriters.hpp"
#include "DREAM3DLib/HDF5/H5VoxelReader.h"
#include "DREAM3DLib/GenericFilters/DataContainerWriter.h"
#include "DREAM3DLib/GenericFilters/VtkRectilinearGridWriter.h"
#include "DREAM3DLib/PrivateFilters/FindNeighbors.h"
#include "DREAM3DLib/SyntheticBuilderFilters/MatchCrystallography.h"
#include "DREAM3DLib/SyntheticBuilderFilters/PlacePrecipitates.h"
#include "DREAM3DLib/SyntheticBuilderFilters/PackGrainsGen2.h"
#include "DREAM3DLib/SyntheticBuilderFilters/AdjustVolume.h"
#include "DREAM3DLib/GenericFilters/FieldDataCSVWriter.h"

#include "UnitTestSupport.hpp"

#include "TestFileLocations.h"

#define PACK_GRAINS_ERROR_TXT_OUT 1
#define PACK_GRAINS_VTK_FILE_OUT 1



std::string m_H5StatsFile("");
std::string m_OutputDirectory = UnitTest::SyntheticBuilderTest::TestDir;
std::string m_OutputFilePrefix("");
size_t m_XPoints = 128;
size_t m_YPoints = 128;
size_t m_ZPoints = 128;

typedef DataArray<unsigned int> ShapeTypeArrayType;


float m_XResolution = 0.25f;
float m_YResolution = 0.25f;
float m_ZResolution = 0.25f;
double m_NeighborhoodErrorWeight = 1.0f;
double m_FractionPrecipitates = 0.0f;

bool m_PeriodicBoundary = false;
std::string m_StructureFile("");
bool m_AlreadyFormed = false;

int m_Precipitates = 0;

bool m_WriteBinaryVTKFiles = true;
bool m_WriteVtkFile = true;
bool m_WriteSurfaceVoxel = true;
bool m_WritePhaseId = true;
bool m_WriteIPFColor = true;

bool m_WriteHDF5GrainFile = false;


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::string getH5StatsFile()
{
  std::string s = UnitTest::DataDir + MXADir::Separator + "2PhaseBulk.h5";
  return s;
}

void setCurrentFilter(AbstractFilter::Pointer f)
{

}

void pipelineFinished()
{

}

void setErrorCondition(int err)
{

}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveTestFiles()
{
  MXADir::remove(UnitTest::SyntheticBuilderTest::CsvFile);
  MXADir::remove(UnitTest::SyntheticBuilderTest::OutputFile);
  MXADir::remove(UnitTest::SyntheticBuilderTest::VtkOutputFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestSyntheticBuilder()
{

  MXADir::mkdir(m_OutputDirectory, true);

  Observer* observer = new Observer;
  ShapeTypeArrayType::Pointer m_ShapeTypes = ShapeTypeArrayType::CreateArray(3);
  m_ShapeTypes->SetName(DREAM3D::EnsembleData::ShapeTypes);
  m_ShapeTypes->SetValue(0, DREAM3D::ShapeType::UnknownShapeType);
  m_ShapeTypes->SetValue(1, DREAM3D::ShapeType::SuperEllipsoidShape);
  m_ShapeTypes->SetValue(2, DREAM3D::ShapeType::SuperEllipsoidShape);

  int err = 0;
  // Instantiate our DataContainer object
  DataContainer::Pointer m = DataContainer::New();
  // Create a Vector to hold all the filters. Later on we will execute all the filters
  std::vector<AbstractFilter::Pointer> pipeline;

#if PACK_GRAINS_ERROR_TXT_OUT
  MAKE_OUTPUT_FILE_PATH( errorFile, DREAM3D::SyntheticBuilder::ErrorFile)
#endif

#if PACK_GRAINS_VTK_FILE_OUT
  MAKE_OUTPUT_FILE_PATH( vtkFile, DREAM3D::SyntheticBuilder::VtkFile)
#endif

  if(m_AlreadyFormed == false)
  {
    m->setDimensions(m_XPoints, m_YPoints, m_ZPoints);
    m->setResolution(m_XResolution, m_YResolution, m_ZResolution);
    m->addEnsembleData(DREAM3D::EnsembleData::ShapeTypes, m_ShapeTypes);

    PackGrainsGen2::Pointer pack_grains = PackGrainsGen2::New();
    pack_grains->setH5StatsInputFile(getH5StatsFile());
    pack_grains->setPeriodicBoundaries(m_PeriodicBoundary);
    pack_grains->setNeighborhoodErrorWeight(m_NeighborhoodErrorWeight);
#if PACK_GRAINS_ERROR_TXT_OUT
    pack_grains->setErrorOutputFile(errorFile);
#endif
#if PACK_GRAINS_VTK_FILE_OUT
    pack_grains->setVtkOutputFile(vtkFile);
#endif
    pipeline.push_back(pack_grains);

    AdjustVolume::Pointer adjust_grains = AdjustVolume::New();
    pipeline.push_back(adjust_grains);
  }
  else if(m_AlreadyFormed == true)
  {
    assert(false);
  }

  if(m_AlreadyFormed == false)
  {
    PlacePrecipitates::Pointer place_precipitates = PlacePrecipitates::New();
    place_precipitates->setH5StatsInputFile(getH5StatsFile());
    place_precipitates->setPeriodicBoundaries(m_PeriodicBoundary);
    pipeline.push_back(place_precipitates);
  }

  MatchCrystallography::Pointer match_crystallography = MatchCrystallography::New();
  match_crystallography->setH5StatsInputFile(getH5StatsFile());
  pipeline.push_back(match_crystallography);

  FieldDataCSVWriter::Pointer write_fielddata = FieldDataCSVWriter::New();
  write_fielddata->setFieldDataFile(UnitTest::SyntheticBuilderTest::CsvFile);
  pipeline.push_back(write_fielddata);

  DataContainerWriter::Pointer writer = DataContainerWriter::New();
  writer->setOutputFile(UnitTest::SyntheticBuilderTest::OutputFile);
  pipeline.push_back(writer);

  bool m_WriteVtkFile(true);
  bool m_WriteBinaryVTKFiles(true);
  bool m_WritePhaseId(true);
  bool m_WriteIPFColor(true);
  bool m_WriteGoodVoxels(false);

  if(m_WriteVtkFile)
  {
    VtkRectilinearGridWriter::Pointer vtkWriter = VtkRectilinearGridWriter::New();
    vtkWriter->setOutputFile(UnitTest::SyntheticBuilderTest::VtkOutputFile);
    vtkWriter->setWriteGrainIds(true);
    vtkWriter->setWritePhaseIds(m_WritePhaseId);
   // vtkWriter->setWriteGoodVoxels(m_WriteGoodVoxels);
    vtkWriter->setWriteIPFColors(m_WriteIPFColor);
    vtkWriter->setWriteBinaryFile(m_WriteBinaryVTKFiles);
    pipeline.push_back(vtkWriter);
  }


  // Start a Benchmark Clock so we can keep track of each filter's execution time
  START_CLOCK()
  // Start looping through the Pipeline
  float progress = 0.0f;
  std::stringstream ss;
  for (std::vector<AbstractFilter::Pointer>::iterator filter = pipeline.begin(); filter != pipeline.end(); ++filter)
  {
    progress = progress + 1.0f;
    observer->pipelineProgress(progress / (pipeline.size() + 1) * 100.0f);
    ss.str("");
    ss << "Executing Filter [" << progress << "/" << pipeline.size() << "] - " << (*filter)->getNameOfClass();
    observer->pipelineProgressMessage(ss.str());
    (*filter)->addObserver(static_cast<Observer*>(observer));
    (*filter)->setDataContainer(m.get());
    setCurrentFilter(*filter);
    (*filter)->execute();
    (*filter)->removeObserver(static_cast<Observer*>(observer));
    err = (*filter)->getErrorCondition();
    if(err < 0)
    {
      setErrorCondition(err);
      observer->pipelineErrorMessage((*filter)->getErrorMessage().c_str());
      observer->pipelineProgress(100);
      pipelineFinished();
      DREAM3D_REQUIRE(err >= 0);
      return;
    }
    //CHECK_FOR_CANCELED(DataContainer, "Grain Generator was canceled", write_fielddata)

    if(DREAM3D_BENCHMARKS)
    {
      std::cout << (*filter)->getNameOfClass() << " Finish Time(ms): " << (MXA::getMilliSeconds() - millis) << std::endl;
      millis = MXA::getMilliSeconds();
    }
  }


  delete observer;
}



// -----------------------------------------------------------------------------
//  Use unit test framework
// -----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  int err = EXIT_SUCCESS;

#if !REMOVE_TEST_FILES
  DREAM3D_REGISTER_TEST( RemoveTestFiles() );
#endif

  DREAM3D_REGISTER_TEST( TestSyntheticBuilder() );

#if REMOVE_TEST_FILES
  DREAM3D_REGISTER_TEST( RemoveTestFiles() );
#endif


  PRINT_TEST_SUMMARY();
  return err;
}




