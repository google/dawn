#version 310 es

uniform mat4x3 u;
mat4x3 s;
void tint_store_and_preserve_padding(inout mat4x3 target, mat4x3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
  target[3u] = value_param[3u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(s, u);
  s[1] = u[0];
  s[1] = u[0].zxy;
  s[0][1] = u[1].x;
}
