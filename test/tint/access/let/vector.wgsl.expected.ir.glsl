#version 310 es

vec3 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 v = vec3(1.0f, 2.0f, 3.0f);
  float scalar = v[1u];
  vec2 swizzle2 = v.xz;
  vec3 swizzle3 = v.xzy;
  vec3 v_1 = vec3(scalar);
  s = ((v_1 + vec3(swizzle2, 1.0f)) + swizzle3);
}
