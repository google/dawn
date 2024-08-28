#version 310 es

struct S {
  mat2 m;
};

S SSBO = S(mat2(vec2(0.0f), vec2(0.0f)));
uniform S UBO = S(mat2(vec2(0.0f), vec2(0.0f)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
