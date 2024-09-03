<dawn>/test/tint/bug/tint/2202.wgsl:7:9 warning: code is unreachable
        let _e9 = (vec3<i32>().y >= vec3<i32>().y);
        ^^^^^^^

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    while(true) {
      {
        while(true) {
          return;
        }
      }
      /* unreachable */
    }
  }
}
