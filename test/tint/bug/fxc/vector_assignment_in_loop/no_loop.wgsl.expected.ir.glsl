#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 v2f = vec2(0.0f);
  vec3 v3f = vec3(0.0f);
  vec4 v4f = vec4(0.0f);
  ivec2 v2i = ivec2(0);
  ivec3 v3i = ivec3(0);
  ivec4 v4i = ivec4(0);
  uvec2 v2u = uvec2(0u);
  uvec3 v3u = uvec3(0u);
  uvec4 v4u = uvec4(0u);
  bvec2 v2b = bvec2(false);
  bvec3 v3b = bvec3(false);
  bvec4 v4b = bvec4(false);
  int i = 0;
  v2f[i] = 1.0f;
  v3f[i] = 1.0f;
  v4f[i] = 1.0f;
  v2i[i] = 1;
  v3i[i] = 1;
  v4i[i] = 1;
  v2u[i] = 1u;
  v3u[i] = 1u;
  v4u[i] = 1u;
  v2b[i] = true;
  v3b[i] = true;
  v4b[i] = true;
}
