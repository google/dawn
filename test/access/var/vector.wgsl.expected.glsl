#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  vec3 v = vec3(0.0f, 0.0f, 0.0f);
  float scalar = v.y;
  vec2 swizzle2 = v.xz;
  vec3 swizzle3 = v.xzy;
  return;
}
void main() {
  tint_symbol();
}


