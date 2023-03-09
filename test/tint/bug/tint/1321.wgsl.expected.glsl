#version 310 es
precision highp float;

int foo() {
  return 1;
}

void tint_symbol() {
  float arr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    int tint_symbol_1 = foo();
    int a_save = tint_symbol_1;
    while (true) {
      {
        float x = arr[a_save];
        break;
      }
    }
  }
}

void main() {
  tint_symbol();
  return;
}
