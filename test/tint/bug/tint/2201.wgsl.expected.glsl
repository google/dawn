<dawn>/test/tint/bug/tint/2201.wgsl:9:9 warning: code is unreachable
        let _e16_ = vec2(false, false);
        ^^^^^^^^^

#version 310 es

void tint_symbol() {
  while (true) {
    if (true) {
      break;
    } else {
      break;
    }
    bvec2 _e16_ = bvec2(false);
    {
      if (_e16_.x) { break; }
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
