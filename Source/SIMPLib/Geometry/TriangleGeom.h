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

#ifndef _trianglegeom_h_
#define _trianglegeom_h_

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/Geometry/IGeometry2D.h"

/**
 * @brief The TriangleGeom class represents a collection of triangles
 */
class SIMPLib_EXPORT TriangleGeom : public IGeometry2D
{
  public:

    SIMPL_SHARED_POINTERS(TriangleGeom)
    SIMPL_STATIC_NEW_MACRO(TriangleGeom)
     SIMPL_TYPE_MACRO_SUPER_OVERRIDE(TriangleGeom, Observable)

    virtual ~TriangleGeom();

    /**
     * @brief CreateGeometry
     * @param numTriangles
     * @param vertices
     * @param name
     * @return
     */
    static Pointer CreateGeometry(int64_t numTriangles, SharedVertexList::Pointer vertices, const QString& name, bool allocate = true);

    /**
     * @brief CreateGeometry
     * @param triangles
     * @param vertices
     * @param name
     * @return
     */
    static Pointer CreateGeometry(SharedTriList::Pointer triangles, SharedVertexList::Pointer vertices, const QString& name);

// -----------------------------------------------------------------------------
// Inherited from SharedVertexOps
// -----------------------------------------------------------------------------

    /**
     * @brief CreateSharedVertexList
     * @param numVertices
     * @return
     */
    static SharedVertexList::Pointer CreateSharedVertexList(int64_t numVertices, bool allocate = true);

// -----------------------------------------------------------------------------
// Inherited from SharedEdgeOps
// -----------------------------------------------------------------------------

    /**
     * @brief CreateSharedEdgeList
     * @param numEdges
     * @return
     */
    static SharedEdgeList::Pointer CreateSharedEdgeList(int64_t numEdges, bool allocate = true);

// -----------------------------------------------------------------------------
// Inherited from SharedTriOps
// -----------------------------------------------------------------------------

    /**
     * @brief CreateSharedTriLost
     * @param numTris
     * @return
     */
    static SharedTriList::Pointer CreateSharedTriList(int64_t numTris, bool allocate = true);

    /**
     * @brief resizeTriList
     * @param newNumTris
     */
    void resizeTriList(int64_t newNumTris);

    /**
     * @brief setTriangles
     * @param triangles
     */
    void setTriangles(SharedTriList::Pointer triangles);

    /**
     * @brief getTriangles
     * @return
     */
    SharedTriList::Pointer getTriangles();

    /**
     * @brief setVertsAtTri
     * @param triId
     * @param verts
     */
    void setVertsAtTri(int64_t triId, int64_t verts[3]);

    /**
     * @brief getVertsAtTri
     * @param triId
     * @param verts
     */
    void getVertsAtTri(int64_t triId, int64_t verts[3]);

    /**
     * @brief getVertCoordsAtTri
     * @param triId
     * @param vert1
     * @param vert2
     * @param vert3
     */
    void getVertCoordsAtTri(int64_t triId, float vert1[3], float vert2[3], float vert3[3]);

    /**
     * @brief getPointer
     * @param i
     * @return
     */
    int64_t* getTriPointer(int64_t i);

    /**
     * @brief getNumberOfTris
     * @return
     */
    int64_t getNumberOfTris();

// -----------------------------------------------------------------------------
// Inherited from IGeometry
// -----------------------------------------------------------------------------

    /**
     * @brief initializeWithZeros
     */
    virtual void initializeWithZeros();

    /**
     * @brief getNumberOfElements
     * @return
     */
    virtual size_t getNumberOfElements();

    /**
     * @brief findElementSizes
     * @return
     */
    virtual int findElementSizes();

    /**
     * @brief getElementSizes
     * @return
     */
    virtual FloatArrayType::Pointer getElementSizes();

    /**
     * @brief deleteElementSizes
     */
    virtual void deleteElementSizes();

    /**
     * @brief findElementsContainingVert
     * @return
     */
    virtual int findElementsContainingVert();

    /**
     * @brief getElementsContainingVert
     * @return
     */
    virtual ElementDynamicList::Pointer getElementsContainingVert();

    /**
     * @brief deleteElementsContainingVert
     */
    virtual void deleteElementsContainingVert();

    /**
     * @brief findElementNeighbors
     * @return
     */
    virtual int findElementNeighbors();

    /**
     * @brief getElementNeighbors
     * @return
     */
    virtual ElementDynamicList::Pointer getElementNeighbors();

    /**
     * @brief deleteElementNeighbors
     */
    virtual void deleteElementNeighbors();

    /**
     * @brief findElementCentroids
     * @return
     */
    virtual int findElementCentroids();

    /**
     * @brief getElementCentroids
     * @return
     */
    virtual FloatArrayType::Pointer getElementCentroids();

    /**
     * @brief deleteElementCentroids
     */
    virtual void deleteElementCentroids();

    /**
     * @brief getParametricCenter
     * @param pCoords
     */
    virtual void getParametricCenter(double pCoords[3]);

    /**
     * @brief getShapeFunctions
     * @param pCoords
     * @param shape
     */
    virtual void getShapeFunctions(double pCoords[3], double* shape);

    /**
     * @brief findDerivatives
     * @param field
     * @param derivatives
     */
    virtual void findDerivatives(DoubleArrayType::Pointer field, DoubleArrayType::Pointer derivatives, Observable* observable = nullptr);

    /**
     * @brief getInfoString
     * @return Returns a formatted string that contains general infomation about
     * the instance of the object.
     */
    virtual QString getInfoString(SIMPL::InfoStringFormat format);

    /**
     * @brief writeGeometryToHDF5
     * @param parentId
     * @param writeXdmf
     * @return
     */
    virtual int writeGeometryToHDF5(hid_t parentId, bool writeXdmf);

    /**
     * @brief writeXdmf
     * @param out
     * @param dcName
     * @param hdfFileName
     * @return
     */
    virtual int writeXdmf(QTextStream& out, QString dcName, QString hdfFileName);

    /**
     * @brief readGeometryFromHDF5
     * @param parentId
     * @param preflight
     * @return
     */
    virtual int readGeometryFromHDF5(hid_t parentId, bool preflight);

    /**
     * @brief deepCopy
     * @return
     */
    virtual IGeometry::Pointer deepCopy(bool forceNoAllocate = false) override;

    /**
     * @brief addAttributeMatrix
     */
    virtual void addAttributeMatrix(const QString& name, AttributeMatrix::Pointer data);

// -----------------------------------------------------------------------------
// Inherited from IGeometry2D
// -----------------------------------------------------------------------------

    /**
     * @brief resizeVertexList
     * @param newNumVertices
     */
    virtual void resizeVertexList(int64_t newNumVertices);

    /**
     * @brief setVertices
     * @param vertices
     */
    virtual void setVertices(SharedVertexList::Pointer vertices);

    /**
     * @brief getVertices
     * @return
     */
    virtual SharedVertexList::Pointer getVertices();

    /**
     * @brief setCoords
     * @param vertId
     * @param coords
     */
    virtual void setCoords(int64_t vertId, float coords[3]);

    /**
     * @brief getCoords
     * @param vertId
     * @param coords
     */
    virtual void getCoords(int64_t vertId, float coords[3]);

    /**
     * @brief getVertexPointer
     * @param i
     * @return
     */
    virtual float* getVertexPointer(int64_t i);

    /**
     * @brief getNumberOfVertices
     * @return
     */
    virtual int64_t getNumberOfVertices();

    /**
     * @brief resizeEdgeList
     * @param newNumEdges
     */
    virtual void resizeEdgeList(int64_t newNumEdges);

    /**
     * @brief getEdges
     * @return
     */
    virtual SharedEdgeList::Pointer getEdges();

    /**
     * @brief setVerts
     * @param edgeId
     * @param verts
     */
    virtual void setVertsAtEdge(int64_t edgeId, int64_t verts[2]);

    /**
     * @brief getVerts
     * @param edgeId
     * @param verts
     */
    virtual void getVertsAtEdge(int64_t edgeId, int64_t verts[2]);

    /**
     * @brief getVertCoordsAtEdge
     * @param edgeId
     * @param vert1
     * @param vert2
     */
    virtual void getVertCoordsAtEdge(int64_t edgeId, float vert1[3], float vert2[3]);

    /**
     * @brief getEdgePointer
     * @param i
     * @return
     */
    virtual int64_t* getEdgePointer(int64_t i);

    /**
     * @brief getNumberOfEdges
     * @return
     */
    virtual int64_t getNumberOfEdges();

    /**
     * @brief findElementEdges
     * @return
     */
    virtual int findEdges();

    /**
     * @brief deleteElementEdges
     */
    virtual void deleteEdges();

    /**
     * @brief findUnsharedEdges
     */
    virtual int findUnsharedEdges();

    /**
     * @brief getUnsharedEdges
     * @return
     */
    virtual SharedEdgeList::Pointer getUnsharedEdges();

    /**
     * @brief deleteUnsharedEdges
     */
    virtual void deleteUnsharedEdges();

  protected:

    TriangleGeom();

    /**
     * @brief setElementsContainingVert
     * @param elementsContainingVert
     */
    virtual void setElementsContainingVert(ElementDynamicList::Pointer elementsContainingVert);

    /**
     * @brief setElementNeighbors
     * @param elementNeighbors
     */
    virtual void setElementNeighbors(ElementDynamicList::Pointer elementNeighbors);

    /**
     * @brief setElementCentroids
     * @param elementCentroids
     */
    virtual void setElementCentroids(FloatArrayType::Pointer elementCentroids);

    /**
     * @brief setElementSizes
     * @param elementSizes
     */
    virtual void setElementSizes(FloatArrayType::Pointer elementSizes);

    /**
     * @brief setEdges
     * @param edges
     */
    virtual void setEdges(SharedEdgeList::Pointer edges);

    /**
     * @brief setUnsharedEdges
     * @param bEdgeList
     */
    virtual void setUnsharedEdges(SharedEdgeList::Pointer bEdgeList);

  private:
    SharedVertexList::Pointer m_VertexList;
    SharedEdgeList::Pointer m_EdgeList;
    SharedEdgeList::Pointer m_UnsharedEdgeList;
    SharedTriList::Pointer m_TriList;
    ElementDynamicList::Pointer m_TrianglesContainingVert;
    ElementDynamicList::Pointer m_TriangleNeighbors;
    FloatArrayType::Pointer m_TriangleCentroids;
    FloatArrayType::Pointer m_TriangleSizes;

    friend class FindTriangleDerivativesImpl;

    TriangleGeom(const TriangleGeom&) = delete;   // Copy Constructor Not Implemented
    void operator=(const TriangleGeom&) = delete; // Move assignment Not Implemented
};


#endif /* _TriangleGeom_H_ */

