#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer S_1 {
  uint field0;
  uint field1[];
} myvar;
void main_1() {
  myvar.field0 = 0u;
  myvar.field1[1u] = 0u;
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
