#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  bool tint_tmp = true;
  if (!tint_tmp) {
    tint_tmp = false;
  }
  bvec2 v2 = bvec2((tint_tmp));
  bool tint_tmp_1 = true;
  if (!tint_tmp_1) {
    tint_tmp_1 = false;
  }
  bvec3 v3 = bvec3((tint_tmp_1));
  bool tint_tmp_2 = true;
  if (!tint_tmp_2) {
    tint_tmp_2 = false;
  }
  bvec4 v4 = bvec4((tint_tmp_2));
}

