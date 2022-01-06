#version 310 es
precision mediump float;

int foo() {
  return 1;
}

void tint_symbol() {
  float arr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  int a_save = foo();
  {
    for(; ; ) {
      float x = arr[a_save];
      break;
    }
  }
  return;
}
void main() {
  tint_symbol();
}


