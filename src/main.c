#include "include/raylib.h"
#include <stdio.h>

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;
  InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
  SetTargetFPS(60);
  SetExitKey(KEY_NULL);
  bool exitWindowRequested = false;
  bool exitWindow = false;

  Texture2D logo = LoadTexture("./resources/Raylib_logo.png");

  while (!exitWindow) {
    float deltaTime = GetFrameTime();

    if (exitWindowRequested) {
      if (IsKeyPressed(KEY_Y) || IsKeyPressed(KEY_ENTER))
        exitWindow = true;
      else if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE))
        exitWindowRequested = false;
    } else if (WindowShouldClose() || IsKeyPressed(KEY_ESCAPE))
      exitWindowRequested = true;

    BeginDrawing();
    {
      ClearBackground(RAYWHITE);
      DrawTexture(logo, (screenWidth - logo.width) / 2.0f, (screenHeight - logo.height) / 2.0f, WHITE);
      DrawText("raylib is the best thing since sliced bread!", 190, 20, 20, LIGHTGRAY);

#if DEBUG
      DrawText("This is a debug build", 190, 50, 20, LIGHTGRAY);
#endif

#if !DEBUG
      DrawText("This is a release build", 190, 50, 20, LIGHTGRAY);
#endif
    }

    // EXIT WINDOW
    if (exitWindowRequested) {
      DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 128});
      DrawRectangle(0, 100, screenWidth, 200, RAYWHITE);
      DrawText("Are you sure you want to exit program? [Y/N]", 40, 180, 30, BLACK);
    }
    EndDrawing();
  }

  UnloadTexture(logo);
  CloseWindow();
  return 0;
}
