#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  bool x = false;
  bool y = false;
  bool tint_tmp = x;
  if (tint_tmp) {
    bool tint_tmp_1 = true;
    if (!tint_tmp_1) {
      tint_tmp_1 = y;
    }
    tint_tmp = (tint_tmp_1);
  }
  if ((tint_tmp)) {
  }
}

