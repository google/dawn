#version 310 es
precision mediump float;


layout (binding = 0) uniform S_1 {
  mat3x2 matrix;
  vec3 vector;
} data;

void tint_symbol() {
  vec2 x = (data.matrix * data.vector);
  return;
}
void main() {
  tint_symbol();
}


