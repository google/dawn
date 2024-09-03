<dawn>/test/tint/bug/tint/2201.wgsl:9:9 warning: code is unreachable
        let _e16_ = vec2(false, false);
        ^^^^^^^^^

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    while(true) {
      if (true) {
        break;
      } else {
        break;
      }
      /* unreachable */
    }
  }
}
