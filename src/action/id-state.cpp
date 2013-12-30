#include "action/id-state.hpp"
#include "id.hpp"

struct ActionIdState :: Impl {
  IdPrimitive state;

  Impl () : state (Id ().primitive ()) {}

  void run () { 
    this->state = IdObject::_currentIdPrimitive;
  }

  void toggle () {
    IdPrimitive currentIdPrimitive = IdObject::_currentIdPrimitive;
    IdObject::_currentIdPrimitive  = this->state;
    this->state                    = currentIdPrimitive;
  }

  void undo () { this->toggle (); }
  void redo () { this->toggle (); }
};


DELEGATE_ACTION_BIG6 (ActionIdState)
DELEGATE             (void, ActionIdState, run)
DELEGATE             (void, ActionIdState, undo)
DELEGATE             (void, ActionIdState, redo)