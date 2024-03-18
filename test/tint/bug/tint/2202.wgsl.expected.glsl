<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^

#version 310 es

void tint_symbol() {
  while (true) {
    while (true) {
      return;
    }
    bool _e9 = true;
    {
      bool tint_tmp = _e9;
      if (!tint_tmp) {
        tint_tmp = _e9;
      }
      if ((tint_tmp)) { break; }
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
