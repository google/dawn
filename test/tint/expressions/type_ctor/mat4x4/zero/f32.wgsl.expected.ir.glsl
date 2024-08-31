#version 310 es

mat4 m = mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
mat4 tint_symbol;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol = m;
}
