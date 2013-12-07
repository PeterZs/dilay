#include <unordered_set>
#include <functional>
#include <glm/glm.hpp>
#include "action/subdivide.hpp"
#include "macro.hpp"
#include "winged-face.hpp"
#include "winged-edge.hpp"
#include "winged-vertex.hpp"
#include "action/unit.hpp"
#include "partial-action/triangulate-6-gon.hpp"
#include "partial-action/triangulate-quad.hpp"
#include "partial-action/delete-t-edges.hpp"
#include "partial-action/insert-edge-vertex.hpp"
#include "adjacent-iterator.hpp"
#include "subdivision-butterfly.hpp"

typedef std::unordered_set <WingedFace*> FaceSet;

struct SubdivideData {
  WingedMesh&     mesh;
  WingedFace&     selection;
  unsigned int    selectionLevel;
  std::list <Id>* affectedFaces;

  SubdivideData (WingedMesh& m, WingedFace& f, std::list <Id>* n) 
    : mesh           (m)
    , selection      (refineSelection (f))
    , selectionLevel (f.level ())
    , affectedFaces  (n)
  {}

  static WingedFace& refineSelection (WingedFace& selection) {
    WingedEdge* tEdge = selection.tEdge ();
    if (tEdge && tEdge->isRightFace (selection))
      return tEdge->leftFaceRef ();
    else 
      return selection;
  }
};

struct ActionSubdivide::Impl {
  ActionUnit actions;

  void undo () { this->actions.undo (); }
  void redo () { this->actions.redo (); }

  WingedFace& run (WingedMesh& mesh, WingedFace& face, std::list <Id>* affectedFaces) { 
    this->actions.reset ();
    return this->subdivide (SubdivideData (mesh, face, affectedFaces)); 
  }

  WingedFace& subdivide (const SubdivideData& data, WingedFace& face) {
    return this->subdivide (SubdivideData (data.mesh, face, data.affectedFaces));
  }

  WingedFace& subdivide (const SubdivideData& data) {
    if (data.selection.level () <= data.selectionLevel) {
      FaceSet neighbourhood, border;

      this->insertNeighbour (neighbourhood, data.selection);

      if (! this->extendNeighbourhood (data, neighbourhood)) {
        return this->subdivide (data);
      }

      if (! this->oneRingBorder (data, neighbourhood, border)) {
        return this->subdivide (data);
      }

      this->subdivideFaces (data, neighbourhood);
      this->refineBorder   (data, border);
    }
    else
      assert (false);
    return data.selection;
  }

  bool extendNeighbourhood ( const SubdivideData& data, FaceSet& neighbourhood) {
    FaceSet extendedNeighbourhood;

    // checks whether a face is already a neighbour
    std::function < bool (WingedFace&) > isNeighbour =
      [&neighbourhood,&extendedNeighbourhood] (WingedFace& face) {

        return (neighbourhood        .count (&face) > 0) 
            || (extendedNeighbourhood.count (&face) > 0);
      };

    // checks whether a face has two adjacent faces that are neighbours
    auto hasAtLeast2Neighbours = [&neighbourhood] (WingedFace& face) -> bool {
      unsigned int numNeighbours = 0;

      for (ADJACENT_FACE_ITERATOR (it, face)) {
        if (neighbourhood.count (&it.element ()) > 0) {
          numNeighbours++;
        }
      }
      return numNeighbours >= 2;
    };

    // adds adjacent faces of a neighbour to the neighbourhood if they have
    // a t-edge or are adjacent to (at least) to neighbours
    std::function < bool (WingedFace&) > checkAdjacents =
      [&] (WingedFace& neighbour) -> bool {
        for (auto it = neighbour.adjacentFaceIterator (true); it.isValid (); it.next ()) {
          WingedFace& face = it.element ();
          if (neighbourhood.count (&face) == 0) {
            if (face.tEdge () || hasAtLeast2Neighbours (face)) {
              unsigned int faceLevel = face.level ();
              if (it.edge ()->gradientAlong (*it.face ())) {
                assert (faceLevel < data.selectionLevel);
                this->subdivide (data,face);
                return false;
              }
              else if (it.edge ()->gradient () == WEGradient::None) {
                assert (faceLevel == data.selectionLevel);
                this->insertNeighbour (neighbourhood,face);
                if (checkAdjacents (face) == false)
                  return false;
              }
            }
          }
        }
        return true;
      };

    neighbourhood.clear   ();
    this->insertNeighbour (neighbourhood, data.selection);

    return checkAdjacents (data.selection);
  }

  bool oneRingBorder ( const SubdivideData& data
                     , const FaceSet& neighbourhood, FaceSet& border) {

    auto ccwFace = [] (const WingedVertex& center, const WingedEdge& edge) -> WingedFace& {
      if (edge.isVertex1 (center))
        return edge.leftFaceRef ();
      else 
        return edge.rightFaceRef ();
    };

    auto deltaGradient = [] (WEGradient gradient) -> int {
      switch (gradient) {
        case WEGradient::Left:  return  1;
        case WEGradient::None:  return  0;
        case WEGradient::Right: return -1;
        default: assert (false);
      }
    };

    auto accountGradient = [&deltaGradient] (WingedVertex& center, WingedEdge& edge) -> int {
      if (edge.isVertex1 (center))
        return deltaGradient (edge.gradient ());
      else 
        return - deltaGradient (edge.gradient ());
    };

    // check levels in one-ring environment
    for (WingedFace* n : neighbourhood) {
      for (auto vIt = n->adjacentVertexIterator (true); vIt.isValid (); vIt.next ()) {
        WingedVertex& vertex   = vIt.element ();
        WingedEdge&   start    = n->adjacentRef (vertex,true);
        int           gradient = 0;

        for (auto eIt = vIt.element ().adjacentEdgeIterator (start); eIt.isValid (); eIt.next ()) {
          WingedEdge& edge = eIt.element ();
          WingedFace& ccw  = ccwFace (vertex, edge);

          if (ccw.id () != n->id ()) {
            gradient = gradient + accountGradient (vertex, edge);

            if (gradient < 0) {
              this->subdivide (data, ccw);
              return false;
            }
          }
        }
      }
    }

    // build border
    border.clear ();
    for (WingedFace* n : neighbourhood) {
      for (auto it = n->adjacentFaceIterator (true); it.isValid (); it.next ()) {
        WingedFace& face = it.element ();

        if (neighbourhood.count (&face) == 0) {
          unsigned int faceLevel = face.level ();
          
          assert (face.tEdge () == nullptr);
          assert (faceLevel >= data.selectionLevel);

          if (it.edge ()->gradient () == WEGradient::None) {
            assert (faceLevel == data.selectionLevel);
            border.insert (&face);
          }
        }
      }
    }
    return true;
  }

  void insertNeighbour (FaceSet& neighbourhood, WingedFace& neighbour) {
    neighbourhood.insert (&neighbour);

    WingedEdge* tEdge = neighbour.tEdge ();
    if (tEdge) {
      neighbourhood.insert (tEdge->otherFace (neighbour));
    }
  }

  void subdivideFaces (const SubdivideData& data, FaceSet& faces) {
    this->actions.add <PADeleteTEdges> ()->run (data.mesh,faces);

    std::unordered_set<Id> subdividedEdges;

    for (WingedFace* face : faces) {
      for (auto it = face->adjacentEdgeIterator (); it.isValid (); ) {
        WingedEdge& edge = it.element ();
        assert (! edge.isTEdge ());
        it.next ();

        if (subdividedEdges.count (edge.id ()) == 0 && edge.gradient () == WEGradient::None) {
          assert (   edge.vertex1Ref ().level () <= data.selectionLevel
                  && edge.vertex2Ref ().level () <= data.selectionLevel);

          WingedEdge& newEdge = this->actions.add <PAInsertEdgeVertex> ()->run 
            (data.mesh, edge, SubdivisionButterfly::subdivideEdge (data.mesh, edge));

          subdividedEdges.insert (edge   .id ());
          subdividedEdges.insert (newEdge.id ());
        }
      }
      this->actions.add <PATriangulate6Gon> ()->run (data.mesh, *face, data.affectedFaces);
    }
  }

  void refineBorder (const SubdivideData& data, FaceSet& border) {
    this->actions.add <PADeleteTEdges> ()->run (data.mesh, border);

    for (WingedFace* face : border) {
      assert (face->level () == data.selectionLevel);
      assert (face->tEdge () == nullptr);
      this->actions.add <PATriangulateQuad> ()->run (data.mesh, *face, data.affectedFaces);
    }
  }
};

DELEGATE_CONSTRUCTOR (ActionSubdivide)
DELEGATE_DESTRUCTOR  (ActionSubdivide)
DELEGATE3            (WingedFace&, ActionSubdivide, run, WingedMesh&, WingedFace&, std::list <Id>*)
DELEGATE             (void, ActionSubdivide, undo)
DELEGATE             (void, ActionSubdivide, redo)
