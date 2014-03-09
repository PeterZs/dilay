#ifndef DILAY_WINGED_MESH
#define DILAY_WINGED_MESH

#include <unordered_set>
#include <glm/fwd.hpp>
#include "fwd-winged.hpp"
#include "macro.hpp"

class WingedFaceIntersection;
class Ray;
class Triangle;
class Octree;
class WingedVertex;
class WingedFace;
class WingedEdge;
class Id;
class Sphere;

class WingedMesh {
  public: WingedMesh             ();
          WingedMesh             (const Id&);
          WingedMesh             (const WingedMesh&) = delete;
          WingedMesh             (      WingedMesh&&);
    const WingedMesh& operator=  (const WingedMesh&) = delete;
    const WingedMesh& operator=  (      WingedMesh&&);
         ~WingedMesh             ();

    Id              id                () const;
    glm::vec3       vertex            (unsigned int) const;
    unsigned int    index             (unsigned int) const;
    glm::vec3       normal            (unsigned int) const;
    WingedVertex*   vertexSLOW        (unsigned int);
    WingedVertex&   lastVertex        ();
    WingedEdge*     edgeSLOW          (const Id&);
    WingedFace*     face              (const Id&);
    unsigned int    addIndex          (unsigned int);
    WingedVertex&   addVertex         (const glm::vec3&);
    WingedEdge&     addEdge           (const WingedEdge&);
    WingedFace&     addFace           (const WingedFace&, const Triangle&);
    void            setIndex          (unsigned int, unsigned int);
    void            setVertex         (unsigned int, const glm::vec3&);
    void            setNormal         (unsigned int, const glm::vec3&);

    const Vertices& vertices          () const;
    const Edges&    edges             () const;
    const Octree&   octree            () const;

    void            deleteEdge        (const WingedEdge&);
    void            deleteFace        (const WingedFace&);
    void            popVertex         ();

    WingedFace&     realignFace       (const WingedFace&, const Triangle&, bool* = nullptr);

    unsigned int    numVertices       () const;
    unsigned int    numEdges          () const;
    unsigned int    numFaces          () const;
    unsigned int    numIndices        () const;
    bool            isEmpty           () const;

    void            writeIndices      (); 
    void            writeNormals      (); 
    void            write             (); 
    void            bufferData        ();
    void            writeAndBuffer    ();
    void            render            ();
    void            reset             ();
    void            initOctreeRoot    (const glm::vec3&, float);
    void            toggleRenderMode  ();
    
    bool            intersects        (const Ray&, WingedFaceIntersection&);
    bool            intersects        (const Sphere&, std::unordered_set <Id>&);
    bool            intersects        (const Sphere&, std::unordered_set <WingedVertex*>&);
    
    SAFE_REF1 (WingedVertex, vertexSLOW, unsigned int)
    SAFE_REF1 (WingedEdge  , edgeSLOW  , const Id&)
    SAFE_REF1 (WingedFace  , face      , const Id&)
  private:
    class Impl;
    Impl* impl;
};

#endif
