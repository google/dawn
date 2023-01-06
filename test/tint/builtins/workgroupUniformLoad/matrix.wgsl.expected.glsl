#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared mat3 v;
mat3 tint_workgroupUniformLoad_v() {
  barrier();
  mat3 result = v;
  barrier();
  return result;
}

mat3 foo() {
  return tint_workgroupUniformLoad_v();
}

