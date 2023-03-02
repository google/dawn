#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
mat3 tint_workgroupUniformLoad(inout mat3 p) {
  barrier();
  mat3 result = p;
  barrier();
  return result;
}

shared mat3 v;
mat3 foo() {
  return tint_workgroupUniformLoad(v);
}

