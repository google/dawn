#version 310 es
precision mediump float;


layout (binding = 0) uniform S_1 {
  mat3 matrix;
  vec3 vector;
} data;

void tint_symbol() {
  vec3 x = (data.matrix * data.vector);
  return;
}
void main() {
  tint_symbol();
}


