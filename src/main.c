#include "game_defines.h"
#include "include/raylib.h"
#include "input_utils.h"
#include "texture_packer_utils.h"
#include "mem_arena.h"

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;
  InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);
  bool exitWindowRequested = false;
  bool exitWindow = false;

  MemoryArena* arenaMain = CreateMemoryArena(1024 * 1024);

  LoadAllTexturesAndSprites();

  // ::INIT
  world = PushType(arenaMain, World);
  consumableInputs = PushType(arenaMain, ConsumableInputFrame);

  world->camera = (Camera2D){0};
  world->camera.target = (Vector2){0, 0};
  world->camera.offset = (Vector2){
      GetScreenWidth() / 2.0,
      GetScreenHeight() / 2.0};
  world->camera.rotation = 0;
  world->camera.zoom = 1.0f;

  world->player1Pos = (Vector2){-100, 0};
  world->player2Pos = (Vector2){100, 0};

  while (!exitWindow) {
    float deltaTime = GetFrameTime();

    { // ::INPUT
      pollInputs();

      if (exitWindowRequested) {
        if (tryConsumeInput(INPUT_Y_PRESSED) || tryConsumeInput(INPUT_ENTER_PRESSED) || tryConsumeInput(INPUT_GPAD_FACE_A_PRESSED))
          exitWindow = true;
        else if (tryConsumeInput(INPUT_N_PRESSED) || tryConsumeInput(INPUT_ESC_PRESSED) || tryConsumeInput(INPUT_GPAD_FACE_B_PRESSED))
          exitWindowRequested = false;
      } else if (WindowShouldClose() || tryConsumeInput(INPUT_ESC_PRESSED) || tryConsumeInput(INPUT_GPAD_START_PRESSED))
        exitWindowRequested = true;

      const float MOVEMENT_SPEED = 50.0f;
      if (tryConsumeInput(INPUT_A_DOWN)) world->player1Pos.x -= MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_D_DOWN)) world->player1Pos.x += MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_W_DOWN)) world->player1Pos.y -= MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_S_DOWN)) world->player1Pos.y += MOVEMENT_SPEED * deltaTime;

      if (tryConsumeInput(INPUT_LEFT_DOWN)) world->player2Pos.x -= MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_RIGHT_DOWN)) world->player2Pos.x += MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_UP_DOWN)) world->player2Pos.y -= MOVEMENT_SPEED * deltaTime;
      if (tryConsumeInput(INPUT_DOWN_DOWN)) world->player2Pos.y += MOVEMENT_SPEED * deltaTime;

      world->player1Pos.x += consumableInputs->gamepadLeftX * MOVEMENT_SPEED * deltaTime;
      world->player1Pos.y += consumableInputs->gamepadLeftY * MOVEMENT_SPEED * deltaTime;
      world->player2Pos.x += consumableInputs->gamepadRightX * MOVEMENT_SPEED * deltaTime;
      world->player2Pos.y += consumableInputs->gamepadRightY * MOVEMENT_SPEED * deltaTime;
    }

    { // ::RENDER
      BeginDrawing();
      {
        ClearBackground(RAYWHITE);
        Texture2D logo = textures[TEX_RAYLIB_LOGO];
        DrawTexture(logo, (screenWidth - logo.width) / 2.0f, (screenHeight - logo.height) / 2.0f, (Color){255, 255, 255, 16});
        DrawText("raylib is the best thing since sliced bread!", 190, 20, 20, LIGHTGRAY);

        BeginMode2D(world->camera);

        {
          SpriteData player1Sprite = sprites[SPRITE_MAIN_PLAYER_1];
          Rectangle  destRect = (Rectangle){
              world->player1Pos.x - (player1Sprite.size.x / 2),
              world->player1Pos.y - (player1Sprite.size.y / 2),
              player1Sprite.size.x,
              player1Sprite.size.y,
          };
          DrawTexturePro(textures[player1Sprite.sourceTexture],
              player1Sprite.sourceRect,
              destRect,
              (Vector2){0, 0},
              0,
              WHITE);
        }

        {
          SpriteData player2Sprite = sprites[SPRITE_MAIN_PLAYER_2];
          Rectangle  destRect = (Rectangle){
              world->player2Pos.x - (player2Sprite.size.x / 2),
              world->player2Pos.y - (player2Sprite.size.y / 2),
              player2Sprite.size.x,
              player2Sprite.size.y,
          };
          DrawTexturePro(textures[player2Sprite.sourceTexture],
              player2Sprite.sourceRect,
              destRect,
              (Vector2){0, 0},
              0,
              WHITE);
        }

        EndMode2D();

#if DEBUG
        DrawText("This is a debug build", 190, 50, 20, LIGHTGRAY);
#endif

#if !DEBUG
        DrawText("This is a release build", 190, 50, 20, LIGHTGRAY);
#endif
      }
    }

    // EXIT WINDOW
    if (exitWindowRequested) {
      DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 128});
      DrawRectangle(0, 100, screenWidth, 200, RAYWHITE);
      DrawText("Are you sure you want to exit program? [Y/N]", 40, 180, 30, BLACK);
    }

    EndDrawing();
  }

  UnloadAllTextures();
  DestroyMemoryArena(arenaMain);
  CloseWindow();
  return 0;
}
