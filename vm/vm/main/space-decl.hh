// Copyright © 2011, Université catholique de Louvain
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// *  Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// *  Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef __SPACE_DECL_H
#define __SPACE_DECL_H

#include "core-forward-decl.hh"

#include "store-decl.hh"
#include "vm-decl.hh"

#include <iostream>

namespace mozart {

struct ScriptEntry {
public:
  UnstableNode left;
  UnstableNode right;
};

class SpaceScript : public VMAllocatedList<ScriptEntry> {
private:
  typedef VMAllocatedList<ScriptEntry> Super;
public:
  ScriptEntry& append(VM vm) {
    Super::push_back_new(vm);
    return back();
  }
};

struct TrailEntry {
public:
  TrailEntry(StableNode* node, const Node& saved) : node(node), saved(saved) {}

  StableNode* node;
  Node saved;
};

class SpaceTrail : public VMAllocatedList<TrailEntry> {
};

/**
 * Computation space
 */
class Space {
public:
  enum Status {
    ssNormal, ssReference, ssFailed, ssGCed
  };
public:
  /** Construct the top-level space */
  Space(VM vm) : vm(vm), _parent(nullptr), _isTopLevel(true), _status(ssNormal),
    _mark(false) {}

  /** Construct a subspace */
  Space(VM vm, Space* parent) : vm(vm), _parent(parent), _isTopLevel(false),
    _status(ssNormal), _mark(false) {}

  /** GC constructor */
  inline
  Space(GC gc, Space* from);

  Space* getParent() {
    if (_isTopLevel)
      return nullptr;
    else
      return _parent;
  }

// Status

public:
  bool isTopLevel() {
    return _isTopLevel;
  }

  Status status() {
    return _status;
  }

  bool isFailed() {
    return _status == ssFailed;
  }

  bool isAlive() {
    for (Space* s = this; !s->isTopLevel(); s = s->getParent())
      if (s->isFailed())
        return false;
    return true;
  }

  StableNode* getRootVar() {
    return &_rootVar;
  }

// Relations between spaces

  bool isAncestor(Space* potentialAncestor) {
    for (Space* s = this; s != nullptr; s = s->getParent()) {
      if (s == potentialAncestor)
        return true;
    }

    return false;
  }

// Speculative bindings

  void makeBackupForSpeculativeBinding(StableNode* node) {
    trail.push_back_new(vm, node, node->node);
  }

// Garbage collection

public:
  Space* gCollect(GC gc) {
    Space* result = new (gc->vm) Space(gc, this);
    _status = ssGCed;
    _gced = result;
    return result;
  }
private:
  friend class GarbageCollector;

  Space* getGCed() {
    assert(status() == ssGCed);
    return _gced;
  }

// Maintenance

public:
  void incRunnableCount() {
    if (!isTopLevel())
      runnableCount++;
  }

  void decRunnableCount() {
    if (!isTopLevel())
      runnableCount--;
  }
private:
  void setReference(Space* ref) {
    _status = ssReference;
    _reference = ref;
  }

// Installation and deinstallation

public:
  /**
   * Try to install this space. Returns true on success and false on failure.
   * Upon failure, the highest space that could be installed is installed.
   */
  bool install();
private:
  inline
  Space* findCommonAncestor(Space* other);

  inline
  void deinstallTo(Space* ancestor);

  inline
  bool installFrom(Space* ancestor);

  inline
  void deinstallThis();

  inline
  bool installThis();

// The mark

private:
  bool hasMark() {
    return _mark;
  }

  void setMark() {
    _mark = true;
  }

  void unsetMark() {
    _mark = false;
  }

// Fields

private:
  friend struct SpaceRef;

  VM vm;

  union {
    SpaceRef _parent;  // status not in [ssReference, ssGCed] && !isTopLevel
    Space* _reference; // status == ssReference
    Space* _gced;      // status == ssGCed
  };

  bool _isTopLevel;
  Status _status;

  bool _mark;

  StableNode _rootVar;

  SpaceTrail trail;
  SpaceScript script;

  int runnableCount;
};

}

#endif // __SPACE_DECL_H
