#include <glm/glm.hpp>
#include "adjacent-iterator.hpp"
#include "primitive/triangle.hpp"
#include "winged/edge.hpp"
#include "winged/face.hpp"
#include "winged/mesh.hpp"
#include "winged/vertex.hpp"

WingedFace :: WingedFace (unsigned int i)
  : _index      (i)
  , _edge       (nullptr)
  , _octreeNode (nullptr) 
  {}

void WingedFace :: writeIndices (WingedMesh& mesh) {
  assert (this->isTriangle ());

  this->vertex (0)->writeIndex (mesh, (3 * this->_index) + 0);
  this->vertex (1)->writeIndex (mesh, (3 * this->_index) + 1);
  this->vertex (2)->writeIndex (mesh, (3 * this->_index) + 2);
}

PrimTriangle WingedFace :: triangle (const WingedMesh& mesh) const {
  return PrimTriangle ( mesh.vector (mesh.index ((3 * this->_index) + 0))
                      , mesh.vector (mesh.index ((3 * this->_index) + 1))
                      , mesh.vector (mesh.index ((3 * this->_index) + 2)) );
}

WingedVertex* WingedFace :: vertex (unsigned int index) const { 
  return this->_edge->vertex (*this, index);
}

unsigned int WingedFace :: numEdges () const {
  unsigned int i             = 0;
  AdjEdges     adjacentEdges = this->adjacentEdges ();

  for (auto it = adjacentEdges.begin (); it != adjacentEdges.end (); ++it) {
    i++;
  }
  return i;
}

glm::vec3 WingedFace :: normal (const WingedMesh& mesh) const {
  assert (this->isTriangle ());
  return PrimTriangle::normal ( this->vertex (0)->position (mesh)
                              , this->vertex (1)->position (mesh)
                              , this->vertex (2)->position (mesh) );
}

bool WingedFace :: isDegenerated (const WingedMesh& mesh) const {
  return PrimTriangle::isDegenerated ( this->vertex (0)->position (mesh)
                                     , this->vertex (1)->position (mesh)
                                     , this->vertex (2)->position (mesh) );
}

WingedEdge* WingedFace :: longestEdge (const WingedMesh& mesh, float *maxLengthSqr) const {
  WingedEdge* tmpLongest = this->edge ();
  float       tmpLength  = 0.0f;

  for (WingedEdge& edge : this->adjacentEdges ()) {
    float lengthSqr = edge.lengthSqr (mesh);
    if (lengthSqr > tmpLength) {
      tmpLength  = lengthSqr;
      tmpLongest = &edge;
    }
  }
  if (maxLengthSqr) {
    *maxLengthSqr = tmpLength;
  }
  return tmpLongest;
}

float WingedFace :: longestEdgeLengthSqr (const WingedMesh& mesh) const {
  float length;
  this->longestEdge (mesh, &length);
  return length;
}

bool WingedFace :: isTriangle () const { return this->numEdges () == 3; }

float WingedFace :: incircleRadiusSqr (const WingedMesh& mesh) const {
  assert (this->isTriangle ());
  return PrimTriangle::incircleRadiusSqr ( this->vertex (0)->position (mesh)
                                         , this->vertex (1)->position (mesh)
                                         , this->vertex (2)->position (mesh) );
}

AdjEdges WingedFace :: adjacentEdges (WingedEdge& e) const {
  return AdjEdges (*this, e);
}
AdjEdges WingedFace :: adjacentEdges () const {
  return this->adjacentEdges (this->edgeRef ());
}
AdjVertices WingedFace :: adjacentVertices (WingedEdge& e) const {
  return AdjVertices (*this, e);
}
AdjVertices WingedFace :: adjacentVertices () const {
  return this->adjacentVertices (this->edgeRef ());
}
AdjFaces WingedFace :: adjacentFaces (WingedEdge& e) const {
  return AdjFaces (*this, e);
}
AdjFaces WingedFace :: adjacentFaces () const {
  return this->adjacentFaces (this->edgeRef ());
}
