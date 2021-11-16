#version 310 es
precision mediump float;


layout (binding = 0) uniform S_1 {
  mat4x3 matrix;
  vec3 vector;
} data;

void tint_symbol() {
  vec4 x = (data.vector * data.matrix);
  return;
}
void main() {
  tint_symbol();
}


