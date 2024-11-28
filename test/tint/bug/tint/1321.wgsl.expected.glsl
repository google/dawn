#version 310 es
precision highp float;
precision highp int;

int foo() {
  return 1;
}
void main() {
  float arr[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  {
    uint v = min(uint(foo()), 3u);
    while(true) {
      float x = arr[v];
      break;
    }
  }
}
