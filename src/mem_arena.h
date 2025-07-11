#ifndef MEM_ARENA_H
#define MEM_ARENA_H

#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct MemoryArena {
  size_t              size;
  uint8_t*            base;
  size_t              used;
  struct MemoryArena* parent;
} MemoryArena;

typedef struct TempMemory {
  MemoryArena* arena;
  size_t       arenaUsedAtStartOfTemp;
} TempMemory;

static MARK_IGNORE_UNUSED_FUNC void InitArena(MemoryArena* arena, size_t size, void* base) {
  arena->size = size;
  arena->base = (uint8_t*)base;
  arena->used = 0;
}
static MARK_IGNORE_UNUSED_FUNC MemoryArena* CreateMemoryArena(size_t size) {
  MemoryArena* arena = (MemoryArena*)malloc(sizeof(MemoryArena));
  InitArena(arena, size, calloc(1, size));
  return arena;
}
static MARK_IGNORE_UNUSED_FUNC void ResetArena(MemoryArena* arena) {
  arena->used = 0;
}
static MARK_IGNORE_UNUSED_FUNC void DestroyMemoryArena(MemoryArena* arena) {
  if (!arena->parent) {
    free(arena->base); // Only free if root, i.e. no parent.
    free(arena);
    // NOTE: We only free arena itself when it's parent
    // This is becasue the sub memory arenas are pushed onto parent arena,
    // So freeing them frees part of the parent's base, which then throws an error when parent is freed.
  }
}

static MARK_IGNORE_UNUSED_FUNC TempMemory BeginTempMemory(MemoryArena* arena) {
  TempMemory result;
  result.arena = arena;
  result.arenaUsedAtStartOfTemp = arena->used;

  return result;
}

static MARK_IGNORE_UNUSED_FUNC void EndTempMemory(TempMemory tempMem) {
  MemoryArena* arena = tempMem.arena;
  assert(arena->used >= tempMem.arenaUsedAtStartOfTemp);
  arena->used = tempMem.arenaUsedAtStartOfTemp;
}

// NOTE: Need to handle alignment when pushing. May cause issues for double and other datatypes
// because chip architecture might expect strict alignment (e.g. doubles always start in mem addr in mutiples of 8)
// Read here: https://hero.handmade.network/forums/code-discussion/t/423-memory_arenas_and_alignment
static size_t MARK_IGNORE_UNUSED_FUNC AlignUp(size_t value, size_t alignment) {
  return (value + (alignment - 1)) & ~(alignment - 1);
}
static size_t MARK_IGNORE_UNUSED_FUNC AlignUpMaxAlignment(size_t value) {
  size_t alignment = _Alignof(max_align_t); // Use max alignment to play safe
  return AlignUp(value, alignment);
}

#define PushType(arena, type) (type*)PushSize_(arena, sizeof(type))
#define PushArray(arena, count, type) (type*)PushSize_(arena, sizeof(type) * (count))
#define PushSize(arena, size) PushSize_(arena, size)
static MARK_IGNORE_UNUSED_FUNC void* PushSize_(MemoryArena* arena, size_t size) {
  size_t alignedUsed = AlignUpMaxAlignment(arena->used);
  size_t newUsed = alignedUsed + size;
  assert(newUsed <= arena->size);
  void* result = arena->base + alignedUsed;
  arena->used = newUsed;
  return result;
}

// NOTE: Undecided if I want to keep this or just have seperate arenas instead.
// Creates potential bugs should I forget to destroy root arena before sub arena and use stuff from sub arena...
static MARK_IGNORE_UNUSED_FUNC MemoryArena* CreateSubMemArena(MemoryArena* sourceArena, size_t subArenaSize) {
  MemoryArena* subArena = PushType(sourceArena, MemoryArena);    // So no stray mallocs, easier for hotreload
  subArenaSize = AlignUpMaxAlignment(subArenaSize);              // Saftey alignment
  assert(sourceArena->used + subArenaSize <= sourceArena->size); // Boundary check

  void* subArenaBase = PushSize(sourceArena, subArenaSize);
  InitArena(subArena, subArenaSize, subArenaBase);
  subArena->parent = sourceArena;
  return subArena;
}

typedef struct MemArenaSnapshot {
  size_t   usedSnapshot;
  uint8_t* dataSnapshot;
} MemArenaSnapshot;

static MARK_IGNORE_UNUSED_FUNC MemArenaSnapshot TakeMemArenaSnapshot(MemoryArena* arena) {
  assert(arena->used > 0 && "Trying to take snapshot of unused MemoryArena is not allowed");
  uint8_t* dataCopy = (uint8_t*)malloc(arena->used);
  memcpy(dataCopy, arena->base, arena->used);

  MemArenaSnapshot snapshot = {
      .usedSnapshot = arena->used,
      .dataSnapshot = dataCopy};
  return snapshot;
}

static MARK_IGNORE_UNUSED_FUNC void RestoreArenaFromSnapshot(MemoryArena* arena, const MemArenaSnapshot* snapshot) {
  assert(snapshot->usedSnapshot > 0 && "Cannot restore snapshot that of unused MemoryArena");
  assert(snapshot->usedSnapshot <= arena->size && "Mismatch in snapshot size and arena size");
  arena->used = snapshot->usedSnapshot;
  memcpy(arena->base, snapshot->dataSnapshot, snapshot->usedSnapshot);
}

static MARK_IGNORE_UNUSED_FUNC void FreeArenaSnapshot(MemArenaSnapshot* snapshot) {
  if (snapshot->dataSnapshot) {
    free(snapshot->dataSnapshot);
    snapshot->dataSnapshot = 0;
  }
  snapshot->usedSnapshot = 0;
}
#endif
