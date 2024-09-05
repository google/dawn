#version 310 es


struct S {
  int before;
  mat4 m;
  int after;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  S tint_symbol[4];
} v;
S p[4] = S[4](S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0), S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)), 0));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v.tint_symbol;
  p[1] = v.tint_symbol[2];
  p[3].m = v.tint_symbol[2].m;
  p[1].m[0] = v.tint_symbol[0].m[1].ywxz;
}
