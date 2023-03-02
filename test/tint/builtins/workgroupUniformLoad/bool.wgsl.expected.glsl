#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bool tint_workgroupUniformLoad(inout bool p) {
  barrier();
  bool result = p;
  barrier();
  return result;
}

shared bool v;
bool foo() {
  return tint_workgroupUniformLoad(v);
}

