#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared vec4 v;
vec4 tint_workgroupUniformLoad_v() {
  barrier();
  vec4 result = v;
  barrier();
  return result;
}

vec4 foo() {
  return tint_workgroupUniformLoad_v();
}

