#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared bool v;
bool tint_workgroupUniformLoad_v() {
  barrier();
  bool result = v;
  barrier();
  return result;
}

int foo() {
  if (tint_workgroupUniformLoad_v()) {
    return 42;
  }
  return 0;
}

