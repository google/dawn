#version 310 es
precision highp float;
precision highp int;

int foo() {
  return 1;
}
void main() {
  float arr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    int v = foo();
    while(true) {
      float x = arr[v];
      break;
    }
  }
}
