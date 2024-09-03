#version 310 es

struct Uniforms {
  uint i;
};

struct OuterS {
  uint a1[8];
};

uniform Uniforms uniforms;
uint f(uint i) {
  return (i + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  vec3 v = vec3(0.0f);
  v[s1.a1[uniforms.i]] = 1.0f;
  v[f(s1.a1[uniforms.i])] = 1.0f;
}
