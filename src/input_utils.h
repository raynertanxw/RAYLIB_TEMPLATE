#ifndef TRACK_SHOOTER_INPUTUTILS_H
#define TRACK_SHOOTER_INPUTUTILS_H
#include "include/raylib.h"
#include "utils.h"
#include <stdio.h>

// ::CONSUMABLEINPUTFRAME
typedef enum ConsumableInputType {
  INPUT_NIL = 0,

  INPUT_ESC_PRESSED,
  INPUT_ENTER_PRESSED,
  INPUT_Y_PRESSED,
  INPUT_N_PRESSED,

  INPUT_UP_DOWN,
  INPUT_DOWN_DOWN,
  INPUT_LEFT_DOWN,
  INPUT_RIGHT_DOWN,

  INPUT_W_DOWN,
  INPUT_S_DOWN,
  INPUT_A_DOWN,
  INPUT_D_DOWN,

  INPUT_MOUSE_LEFT_PRESSED,
  INPUT_MOUSE_LEFT_DOWN,
  INPUT_MOUSE_LEFT_RELEASED,

  // ::GAMEPAD INPUTS
  INPUT_GPAD_FACE_A_PRESSED,
  INPUT_GPAD_FACE_B_PRESSED,
  INPUT_GPAD_START_PRESSED,

  // NOTE: Add more inputs as needed. Don't forget to regsiter the inputs in pollInputs.

  INPUT_NUM_TYPES
} ConsumableInputType;
typedef struct ConsumableInputFrame {
  float mouseWheelMove;
  bool  state[INPUT_NUM_TYPES];

  // NOTE: Gamepad values
  float gamepadLeftX;
  float gamepadLeftY;
  float gamepadRightX;
  float gamepadRightY;
} ConsumableInputFrame;

ConsumableInputFrame* consumableInputs = 0;

// ::FUNCTIONS
void registerInputState(ConsumableInputType type, bool state) { consumableInputs->state[type] = state; }
void pollInputs() {
  registerInputState(INPUT_ESC_PRESSED, IsKeyPressed(KEY_ESCAPE));
  registerInputState(INPUT_ENTER_PRESSED, IsKeyPressed(KEY_ENTER));
  registerInputState(INPUT_Y_PRESSED, IsKeyPressed(KEY_Y));
  registerInputState(INPUT_N_PRESSED, IsKeyPressed(KEY_N));

  registerInputState(INPUT_UP_DOWN, IsKeyDown(KEY_UP));
  registerInputState(INPUT_DOWN_DOWN, IsKeyDown(KEY_DOWN));
  registerInputState(INPUT_LEFT_DOWN, IsKeyDown(KEY_LEFT));
  registerInputState(INPUT_RIGHT_DOWN, IsKeyDown(KEY_RIGHT));

  registerInputState(INPUT_W_DOWN, IsKeyDown(KEY_W));
  registerInputState(INPUT_S_DOWN, IsKeyDown(KEY_S));
  registerInputState(INPUT_A_DOWN, IsKeyDown(KEY_A));
  registerInputState(INPUT_D_DOWN, IsKeyDown(KEY_D));

  registerInputState(INPUT_MOUSE_LEFT_PRESSED, IsMouseButtonPressed(0));
  registerInputState(INPUT_MOUSE_LEFT_DOWN, IsMouseButtonDown(0));
  registerInputState(INPUT_MOUSE_LEFT_RELEASED, IsMouseButtonReleased(0));

  consumableInputs->mouseWheelMove = GetMouseWheelMove();

  { // Register gamepad inputs
    // NOTE: Assume this is a single controller game, we only check first gamepad's input.
    const int GAMEPAD_ID = 0;
    if (IsGamepadAvailable(GAMEPAD_ID)) {
      registerInputState(INPUT_GPAD_FACE_A_PRESSED, IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
      registerInputState(INPUT_GPAD_FACE_B_PRESSED, IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
      registerInputState(INPUT_GPAD_START_PRESSED, IsGamepadButtonPressed(GAMEPAD_ID, GAMEPAD_BUTTON_MIDDLE_RIGHT));

      consumableInputs->gamepadLeftX = GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_X);
      consumableInputs->gamepadLeftY = GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_LEFT_Y);
      consumableInputs->gamepadRightX = GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_X);
      consumableInputs->gamepadRightY = GetGamepadAxisMovement(GAMEPAD_ID, GAMEPAD_AXIS_RIGHT_Y);

      const float leftStickDeadzoneX = 0.1f;
      const float leftStickDeadzoneY = 0.1f;
      const float rightStickDeadzoneX = 0.1f;
      const float rightStickDeadzoneY = 0.1f;
      if (absF(consumableInputs->gamepadLeftX) < leftStickDeadzoneX) consumableInputs->gamepadLeftX = 0.0f;
      if (absF(consumableInputs->gamepadLeftY) < leftStickDeadzoneY) consumableInputs->gamepadLeftY = 0.0f;
      if (absF(consumableInputs->gamepadRightX) < rightStickDeadzoneX) consumableInputs->gamepadRightX = 0.0f;
      if (absF(consumableInputs->gamepadRightY) < rightStickDeadzoneY) consumableInputs->gamepadRightY = 0.0f;
    }
  }
}
bool peekInput(ConsumableInputType type) { return consumableInputs->state[type]; }
void consumeInput(ConsumableInputType type) { consumableInputs->state[type] = false; }
bool tryConsumeInput(ConsumableInputType type) {
  bool value = consumableInputs->state[type];
  consumableInputs->state[type] = false;
  return value;
}
float peekMouseScroll() { return consumableInputs->mouseWheelMove; }
float tryConsumeMouseScroll() {
  float mouseWheelMoveValue = consumableInputs->mouseWheelMove;
  consumableInputs->mouseWheelMove = 0.0f;
  return mouseWheelMoveValue;
}

#endif // !TRACK_SHOOTER_INPUTUTILS_H
