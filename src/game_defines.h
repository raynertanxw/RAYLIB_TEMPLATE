#ifndef GAME_DEFINES_H
#define GAME_DEFINES_H

#include "include/raylib.h"
#include "include/raymath.h"

// ::SYSTEM
#ifdef _WIN32
#include <limits.h> // For PATH_MAX
#define PATH_SEPARATOR '\\'
#else
#include <sys/syslimits.h> // For PATH_MAX
#define PATH_SEPARATOR '/'
#endif
#define MAX_PATH_LENGTH PATH_MAX

// ::TEXTURES & SPRITES
typedef enum TextureID {
  TEX_NIL = 0,

  TEX_RAYLIB_LOGO,
  TEX_MAIN,

  TEX_COUNT
} TextureID;
typedef enum SpriteID {
  SPRITE_NIL = 0,
  SPRITE_RAYLIB_LOGO,

  // MAIN TEXTURE
  SPRITE_MAIN_PLAYER_1,
  SPRITE_MAIN_PLAYER_2,

  SPRITE_COUNT
} SpriteID;
typedef struct SpriteData {
  TextureID sourceTexture;
  Rectangle sourceRect;
  Vector2   size;
} SpriteData;

// ::WORLD
typedef struct {
  Camera2D camera;

  Vector2 player1Pos;
  Vector2 player2Pos;
} World;

// ::GLOBALS
Texture2D  textures[TEX_COUNT];
SpriteData sprites[SPRITE_COUNT];

World* world = 0;

#endif // !GAME_DEFINES_H
