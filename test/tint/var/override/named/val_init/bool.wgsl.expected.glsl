#version 310 es

void tint_symbol() {
  bool tint_tmp = false;
  if (tint_tmp) {
    tint_tmp = true;
  }
  if ((tint_tmp)) {
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
