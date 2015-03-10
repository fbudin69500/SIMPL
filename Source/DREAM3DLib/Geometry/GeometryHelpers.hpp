#ifndef _GeometryHelpers_H_
#define _GeometryHelpers_H_

#include <math.h>

#include "H5Support/QH5Utilities.h"
#include "H5Support/HDF5ScopedFileSentinel.h"
#include "H5Support/QH5Lite.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/DataArrays/DataArray.hpp"
#include "DREAM3DLib/Geometry/IGeometry.h"

/**
* @brief This file contains a namespace with classes for manipulating IGeometry objects
*/

namespace GeometryHelpers
{
  /**
   * @brief The GeomIO class
   */
  class GeomIO
  {
    public:
      GeomIO() {}
      virtual ~GeomIO() {}

      /**
       * @brief ReadMeshFromHDF5
       * @param listName
       * @param parentId
       * @param preflight
       * @return
       */
      template<typename ListType>
      static typename ListType::Pointer ReadMeshFromHDF5(const QString& listName, hid_t parentId, bool preflight)
      {
        herr_t err = 0;
        QVector<hsize_t> dims;
        H5T_class_t type_class;
        size_t type_size;
        typename ListType::Pointer mesh = ListType::CreateArray(0, listName);
        if (true == preflight)
        {
          err = QH5Lite::getDatasetInfo(parentId, listName, dims, type_class, type_size);
          if (err < 0)
          {
            mesh = ListType::NullPointer();
          }
        }
        else
        {
          err = mesh->readH5Data(parentId);
          if (err < 0)
          {
            mesh = ListType::NullPointer();
          }
        }

        return mesh;
      }

      /**
       * @brief ReadMetaDataFromHDF5
       * @param parentId
       * @param geometry
       * @return
       */
      static int ReadMetaDataFromHDF5(hid_t parentId, IGeometry::Pointer geometry)
      {
        herr_t err = 0;
        unsigned int spatialDims = 0;
        QString geomName = "";
        err = QH5Lite::readScalarAttribute(parentId, DREAM3D::Geometry::Geometry, DREAM3D::Geometry::SpatialDimensionality, spatialDims);
        if(err < 0)
        {
          return err;
        }
        err = QH5Lite::readStringAttribute(parentId, DREAM3D::Geometry::Geometry, DREAM3D::Geometry::GeometryName, geomName);
        if(err < 0)
        {
          return err;
        }
        geometry->setSpatialDimensionality(spatialDims);
        geometry->setName(geomName);

        return 1;
      }

      /**
       * @brief WriteListToHDF5
       * @param parentId
       * @param list
       * @return
       */
      static int WriteListToHDF5(hid_t parentId, IDataArray::Pointer list)
      {
        herr_t err = 0;
        QVector<size_t> tDims(1, list->getNumberOfTuples());
        err = list->writeH5Data(parentId, tDims);
        return err;
      }


      /**
       * @brief WriteDynamicListToHDF5
       * @param parentId
       * @param dynamicList
       * @param numElems
       * @param name
       * @return
       */
      template<typename T, typename K>
      static int WriteDynamicListToHDF5(hid_t parentId, typename DynamicListArray<T, K>::Pointer dynamicList, size_t numElems, QString name)
      {
        herr_t err = 0;
        int32_t rank = 0;
        hsize_t dims[2] = {0, 2ULL};
        size_t total = 0;
        for(size_t v = 0; v < numElems; ++v)
        {
          total += dynamicList->getNumberOfElements(v);
        }

        size_t totalBytes = numElems * sizeof(T) + total * sizeof(K);

        // Allocate a flat array to copy the data into
        QVector<uint8_t> buffer(totalBytes, 0);
        uint8_t* bufPtr = &(buffer.front());
        size_t offset = 0;

        for(size_t v = 0; v < numElems; ++v)
        {
          T nelems = dynamicList->getNumberOfElements(v);
          K* elems = dynamicList->getElementListPointer(v);
          ::memcpy(bufPtr + offset, &nelems, sizeof(T));
          offset += sizeof(T);
          ::memcpy(bufPtr + offset, elems, nelems * sizeof(K) );
          offset += nelems * sizeof(K);
        }

        rank = 1;
        dims[0] = totalBytes;

        err = QH5Lite::writePointerDataset(parentId, name, rank, dims, bufPtr);
        return err;
      }
  };

  /**
   * @brief The Connectivity class
   */
  class Connectivity
  {
    public:
      Connectivity() {}
      virtual ~Connectivity() {}


      /**
       * @brief FindElementsContainingVert
       * @param elemList
       * @param dynamicList
       * @param numVerts
       */
      template<typename T, typename K>
      static void FindElementsContainingVert(typename DataArray<K>::Pointer elemList, typename DynamicListArray<T, K>::Pointer dynamicList, size_t numVerts)
      {
        size_t numElems = elemList->getNumberOfTuples();
        size_t numVertsPerElem = elemList->getNumberOfComponents();

        // Allocate the basic structures
        QVector<size_t> linkCount(numVerts, 0);
        size_t elemId = 0;
        int64_t* linkLoc;

        // Fill out lists with number of references to cells
        typedef boost::shared_array<K> SharedArray_t;
        SharedArray_t linkLocPtr(new K[numVerts]);
        linkLoc = linkLocPtr.get();

        ::memset(linkLoc, 0, numVerts * sizeof(K));

        K* verts;

        //vtkPolyData *pdata = static_cast<vtkPolyData *>(data);
        // Traverse data to determine number of uses of each point
        for (elemId = 0; elemId < numElems; elemId++)
        {
          verts = elemList->getTuplePointer(elemId);
          for (size_t j = 0; j < numVertsPerElem; j++)
          {
            linkCount[verts[j]]++;
          }
        }

        // Now allocate storage for the links
        dynamicList->allocateLists(linkCount);

        for (elemId = 0; elemId < numElems; elemId++)
        {
          verts = elemList->getTuplePointer(elemId);
          for (size_t j = 0; j < numVertsPerElem; j++)
          {
            dynamicList->insertCellReference(verts[j], (linkLoc[verts[j]])++, elemId);
          }
        }
      }


      /**
       * @brief FindElementNeighbors
       * @param elemList
       * @param elemsContainingVert
       * @param dynamicList
       * @return
       */
      template<typename T, typename K>
      static int FindElementNeighbors(typename DataArray<K>::Pointer elemList, typename DynamicListArray<T, K>::Pointer elemsContainingVert,
                                   typename DynamicListArray<T, K>::Pointer dynamicList)
      {
        size_t numElems = elemList->getNumberOfTuples();
        size_t numVertsPerElem = elemList->getNumberOfComponents();
        size_t numSharedVerts = 0;
        QVector<T> linkCount(numElems, 0);
        int err = 0;

        switch(numVertsPerElem)
        {
        case 2: // edges
        {
          numSharedVerts = 1;
          break;
        }
        case 3: // triangles
        {
          numSharedVerts = 2;
          break;
        }
        case 4: // quadrilaterals
        {
          numSharedVerts = 2;
          break;
        }
        default:
          numSharedVerts = 0;
          break;
        }

        // Allocate an array of bools that we use each iteration so that we don't put duplicates into the array
        boost::shared_array<bool> visitedPtr(new bool[numElems]);
        bool* visited = visitedPtr.get();
        ::memset(visitedPtr.get(), 0, numElems);

        // Reuse this vector for each loop. Avoids re-allocating the memory each time through the loop
        QVector<K> loop_neighbors(32, 0);

        // Build up the element adjacency list now that we have the element links
        for (size_t t = 0; t < numElems; ++t)
        {
          //   qDebug() << "Analyzing Cell " << t << "\n";
          K* seedElem = elemList->getTuplePointer(t);
          for (size_t v = 0; v < numVertsPerElem; ++v)
          {
            //   qDebug() << " vert " << v << "\n";
            T nEs = elemsContainingVert->getNumberOfElements(seedElem[v]);
            K* vertIdxs = elemsContainingVert->getElementListPointer(seedElem[v]);

            for (T vt = 0; vt < nEs; ++vt)
            {
              if (vertIdxs[vt] == static_cast<K>(t) ) { continue; } // This is the same element as our "source"
              if (visited[vertIdxs[vt]] == true) { continue; } // We already added this element so loop again
              //      qDebug() << "   Comparing Element " << vertIdxs[vt] << "\n";
              K* vertCell = elemList->getTuplePointer(vertIdxs[vt]);
              size_t vCount = 0;
              // Loop over all the vertex indices of this element and try to match numSharedVerts of them to the current loop element
              // If there is numSharedVerts match then that element is a neighbor of the source. If there are more than numVertsPerElem
              // matches then there is a real problem with the mesh and the program is going to return an error.
              for (size_t i = 0; i < numVertsPerElem; i++)
              {
                for (size_t j = 0; j < numVertsPerElem; j++)
                {
                  if (seedElem[i] == vertCell[i])
                  {
                    vCount++;
                  }
                }
              }

              if (vCount >= numVertsPerElem) // No way 2 elements can share all vertices. Something is VERY wrong at this point
              {
                return -1;
              }

              // So if our vertex match count is numSharedVerts and we have not visited the element in question then add this element index
              // into the list of vertex indices as neighbors for the source element.
              if (vCount == numSharedVerts)
              {
                //qDebug() << "       Neighbor: " << vertIdxs[vt] << "\n";
                // Use the current count of neighbors as the index
                // into the loop_neighbors vector and place the value of the vertex element at that index
                loop_neighbors[linkCount[t]] = vertIdxs[vt];
                linkCount[t]++; // Increment the count for the next time through
                if (linkCount[t] >= loop_neighbors.size())
                {
                  loop_neighbors.resize(loop_neighbors.size() + 10);
                }
                visited[vertIdxs[vt]] = true; // Set this element as visited so we do NOT add it again
              }
            }
          }
          // Reset all the visited cell indexs back to false (zero)
          for(int64_t k = 0; k < linkCount[t]; ++k)
          {
            visited[loop_neighbors[k]] = false;
          }
          // Allocate the array storage for the current edge to hold its vertex list
          dynamicList->setElementList(t, linkCount[t], &(loop_neighbors[0]));
        }

        return err;
      }
  };

  /**
   * @brief The Topology class
   */
  class Topology
  {
    public:
      Topology() {}
      virtual ~Topology() {}

      /**
       * @brief FindElementCentroids
       * @param elemList
       * @param vertices
       * @param elementCentroids
       */
      template<typename T>
      static void FindElementCentroids(typename DataArray<T>::Pointer elemList, FloatArrayType::Pointer vertices, FloatArrayType::Pointer centroids)
      {
        size_t numElems = elemList->getNumberOfTuples();
        size_t numVertsPerElem = elemList->getNumberOfComponents();
        size_t numDims = 3;
        float* elementCentroids = centroids->getPointer(0);
        float* vertex = vertices->getPointer(0);

        for (size_t i = 0; i < numDims; i++)
        {
          for (size_t j = 0; j < numElems; j++)
          {
            T* Elem = elemList->getTuplePointer(j);
            float vertPos = 0.0;
            for (size_t k = 0; k < numVertsPerElem; k++)
            {
              vertPos += vertex[3*Elem[k]+i];
            }
            vertPos /= static_cast<float>(numVertsPerElem);
            elementCentroids[numDims*j+i] = vertPos;
          }
        }
      }
  };

  /**
   * @brief The Generic class
   */
  class Generic
  {
    public:
      Generic() {}
      virtual ~Generic() {}

      /**
       * @brief AverageVertexArrayValues
       * @param elemList
       * @param inVertexArray
       * @param outElemArray
       */
      template<typename T, typename K>
      static void AverageVertexArrayValues(typename DataArray<T>::Pointer elemList, typename DataArray<K>::Pointer inVertexArray, DataArray<float>::Pointer outElemArray)
      {
        BOOST_ASSERT(outElemArray->getComponentDimensions() == inVertexArray->getComponentDimensions());
        BOOST_ASSERT(elemList->getNumberOfTuples() == outElemArray->getNumberOfTuples());

        K* vertArray = inVertexArray->getPointer(0);
        float* elemArray = outElemArray->getPointer(0);

        size_t numElems = outElemArray->getNumberOfTuples();
        size_t numDims = inVertexArray->getNumberOfComponents();
        size_t numVertsPerElem = elemList->getNumberOfComponents();

        for (size_t i = 0; i < numDims; i++)
        {
          for (size_t j = 0; j < numElems; j++)
          {
            T* Elem = elemList->getTuplePointer(j);
            float vertValue = 0.0;
            for (size_t k = 0; k < numVertsPerElem; k++)
            {
              vertValue += vertArray[numDims*Elem[k]+i];
            }
            vertValue /= static_cast<float>(numVertsPerElem);
            elemArray[numDims*j+i] = vertValue;
          }
        }
      }

      /**
       * @brief WeightedAverageVertexArrayValues
       * @param elemList
       * @param vertices
       * @param centroids
       * @param inVertexArray
       * @param outElemArray
       */
      template<typename T, typename K>
      static void WeightedAverageVertexArrayValues(typename DataArray<T>::Pointer elemList, DataArray<float>::Pointer vertices,
                                                   DataArray<float>::Pointer centroids, typename DataArray<K>::Pointer inVertexArray,
                                                   DataArray<float>::Pointer outElemArray)
      {
        BOOST_ASSERT(outElemArray->getNumberOfTuples() == elemList->getNumberOfTuples());
        BOOST_ASSERT(outElemArray->getComponentDimensions() == inVertexArray->getComponentDimensions());

        K* vertArray = inVertexArray->getPointer(0);
        float* elemArray = outElemArray->getPointer(0);
        float* elementCentroids = centroids->getPointer(0);
        float* vertex = vertices->getPointer(0);

        size_t numElems = outElemArray->getNumberOfTuples();
        size_t cDims = inVertexArray->getNumberOfComponents();
        size_t numVertsPerElem = elemList->getNumberOfComponents();
        size_t numDims = 3;

        // Vector to hold vertex-centroid distances, 4 per cell
        std::vector<float> vertCentDist(numElems*numVertsPerElem);

        for (size_t i = 0; i < numElems; i++)
        {
          T* Elem = elemList->getTuplePointer(i);
          for (size_t j = 0; j < numVertsPerElem; j++)
          {
            for (size_t k = 0; k < numDims; k++)
            {
              vertCentDist[numVertsPerElem*i+j] += (vertex[numDims*Elem[j]+k] - elementCentroids[numDims*i+k]) * (vertex[numDims*Elem[j]+k] - elementCentroids[numDims*i+k]);
            }
            vertCentDist[numVertsPerElem*i+j] = sqrt(vertCentDist[numVertsPerElem*i+j]);
          }
        }

        for (size_t i = 0; i < cDims; i++)
        {
          for (size_t j = 0; j < numElems; j++)
          {
            T* Elem = elemList->getTuplePointer(j);
            float vertValue = 0.0;
            float sumDist = 0.0;
            for (size_t k = 0; k < numVertsPerElem; k++)
            {
              vertValue += vertArray[cDims*Elem[k]+i] * vertCentDist[numVertsPerElem*j+k];
              sumDist += vertCentDist[numVertsPerElem*j+k];
            }
            vertValue /= static_cast<float>(sumDist);
            elemArray[cDims*j+i] = vertValue;
          }
        }
      }

      template<typename T, typename K, typename L>
      static void AverageCellArrayValues(typename DynamicListArray<L, T>::Pointer elemsContainingVert,
                                         DataArray<float>::Pointer vertices, typename DataArray<K>::Pointer inElemArray,
                                         DataArray<float>::Pointer outVertexArray)
      {
        BOOST_ASSERT(outVertexArray->getNumberOfTuples() == vertices->getNumberOfTuples());
        BOOST_ASSERT(outVertexArray->getComponentDimensions() == inElemArray->getComponentDimensions());

        K* elemArray = inElemArray->getPointer(0);
        float* vertArray = outVertexArray->getPointer(0);

        size_t numVerts = vertices->getNumberOfTuples();
        size_t cDims = inElemArray->getNumberOfComponents();

        for (size_t i = 0; i < cDims; i++)
        {
          for (size_t j = 0; j < numVerts; j++)
          {
            L numElemsPerVert = elemsContainingVert->getNumberOfElements(j);
            T* elemIdxs = elemsContainingVert->getElementListPointer(j);
            float vertValue = 0.0;
            float weight = 1.0f / numElemsPerVert;
            for (size_t k = 0; k < numElemsPerVert; k++)
            {
              vertValue += elemArray[cDims*elemIdxs[k]+i] * weight;
            }
            vertArray[cDims*j+i] = vertValue;
          }
        }
      }
  };

}

#endif /* _GeometryHelpers_H_ */
