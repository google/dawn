#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer S_1 {
  float field0;
  float age[];
} myvar;
void main_1() {
  myvar.age[2u] = 42.0f;
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
