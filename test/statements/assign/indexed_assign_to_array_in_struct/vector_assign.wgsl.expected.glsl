#version 310 es
precision mediump float;

struct Uniforms {
  uint i;
};
struct OuterS {
  uint a1[8];
};

layout (binding = 4) uniform Uniforms_1 {
  uint i;
} uniforms;

uint f(uint i) {
  return (i + 1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  OuterS s1 = OuterS(uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  vec3 v = vec3(0.0f, 0.0f, 0.0f);
  v[s1.a1[uniforms.i]] = 1.0f;
  v[f(s1.a1[uniforms.i])] = 1.0f;
  return;
}
void main() {
  tint_symbol();
}


