#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 2, r32i) uniform highp iimage2D tex;
void foo() {
  {
    int i = 0;
    while (true) {
      if (!((i < 3))) {
        break;
      }
      {
      }
      {
        imageStore(tex, ivec2(0), ivec4(0));
      }
    }
  }
}

