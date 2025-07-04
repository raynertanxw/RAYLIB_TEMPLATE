#ifndef TEXTURE_PACKER_UTILS_H
#define TEXTURE_PACKER_UTILS_H

#include "include/raylib.h"
#include "game_defines.h"
#include "build_defines.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  const char* filename;
  Vector2     size;
  Rectangle   rect;
} SpriteMetadata;

Vector2 GetImageSize(const char* filePath) {
  Image   img = LoadImage(filePath);
  Vector2 size = (Vector2){(float)img.width, (float)img.height};
  UnloadImage(img);
  return size;
}

static inline void BuildResourcePath(const char* fileName, char* outPath) {
  snprintf(outPath, MAX_PATH_LENGTH, "%s%cresources%c%s", GetApplicationDirectory(), PATH_SEPARATOR, PATH_SEPARATOR, fileName);
}

// Load metadata from a file
int LoadSpriteMeta(const char* fileName, SpriteMetadata* sprites, int maxSprites) {
  char path[MAX_PATH_LENGTH];
  BuildResourcePath(fileName, path);

  FILE* file = fopen(path, "r");
  if (!file) {
    TraceLog(LOG_ERROR, "Failed to open metadata file: %s", path);
    return 0;
  }

  int count = 0;
  fscanf(file, "%d", &count);
  if (count > maxSprites)
    count = maxSprites;

  for (int i = 0; i < count; i++) {
    static char pathBuffer[512];
    int         tmp_x, tmp_y, tmp_width, tmp_height;
    fscanf(file, "%s %d %d %d %d", pathBuffer, &tmp_x, &tmp_y, &tmp_width, &tmp_height);
    sprites[i].rect = (Rectangle){(float)tmp_x, (float)tmp_y, (float)tmp_width, (float)tmp_height};

    sprites[i].filename = strdup(pathBuffer);
    sprites[i].size = (Vector2){sprites[i].rect.width, sprites[i].rect.height};
  }
  fclose(file);
  return count;
}

SpriteMetadata* FindSpriteByFilename(SpriteMetadata* sprites, int count, const char* filename) {
  for (int i = 0; i < count; i++) {
    if (strcmp(sprites[i].filename, filename) == 0) {
      return &sprites[i];
    }
  }
  TraceLog(LOG_WARNING, "Failed to find sprite: %s", filename);
  return NULL; // Not found
}

// ::TEXTURES
void setupSprite(SpriteID spriteID, TextureID texID, int startX, int startY, int width, int height) {
  sprites[spriteID] = (SpriteData){
      .sourceTexture = texID,
      .sourceRect = (Rectangle){(float)startX, (float)startY, (float)width, (float)height},
      .size = (Vector2){(float)width, (float)height}
  };
}
void setupSpriteFromMetadata(SpriteID spriteID, TextureID texID, SpriteMetadata* metadata) {
  if (metadata) {
    sprites[spriteID] = (SpriteData){
        .sourceTexture = texID,
        .sourceRect = metadata->rect,
        .size = metadata->size};
  } else {
    sprites[spriteID] = sprites[SPRITE_NIL];
  }
}
void setupSpriteAsWholeTexture(SpriteID spriteID, TextureID texID) {
  Texture2D tex = textures[texID];
  setupSprite(spriteID, texID, 0, 0, tex.width, tex.height);
}

void LoadAllTexturesAndSprites() {
  { // Single sprite textures
    textures[TEX_NIL] = LoadTexture("resources/missing.png");
    setupSpriteAsWholeTexture(SPRITE_NIL, TEX_NIL);
    textures[TEX_RAYLIB_LOGO] = LoadTexture("resources/Raylib_logo.png");
    setupSpriteAsWholeTexture(SPRITE_RAYLIB_LOGO, TEX_RAYLIB_LOGO);
  }

  { // Main Texture
    SpriteMetadata spritesMetadata[MAIN_ATLAS_MAX_SPRITES] = {0};
    int            spriteCount = LoadSpriteMeta(MAIN_ATLAS_META_FILE, spritesMetadata, MAIN_ATLAS_MAX_SPRITES);
    if (spriteCount == 0) {
      TraceLog(LOG_ERROR, "No valid metadata found!");
      exit(1);
    }

    char atlasPath[MAX_PATH_LENGTH];
    BuildResourcePath(MAIN_ATLAS_IMAGE_FILE, atlasPath);
    textures[TEX_MAIN] = LoadTexture(atlasPath);
    setupSpriteFromMetadata(SPRITE_MAIN_PLAYER_1, TEX_MAIN, FindSpriteByFilename(spritesMetadata, spriteCount, "player_1"));
    setupSpriteFromMetadata(SPRITE_MAIN_PLAYER_2, TEX_MAIN, FindSpriteByFilename(spritesMetadata, spriteCount, "player_2"));
  }
}

void UnloadAllTextures() {
  for (int iTex = 0; iTex < TEX_COUNT; iTex++) {
    UnloadTexture(textures[iTex]);
  }
}

#endif // !TEXTURE_PACKER_UTILS_H
