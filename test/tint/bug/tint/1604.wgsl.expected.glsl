#version 310 es

layout(binding = 0, std140)
uniform x_block_1_ubo {
  uvec4 inner[1];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec4 v_1 = v.inner[0u];
  switch(int(v_1.x)) {
    case 0:
    {
      {
        uvec2 tint_loop_idx = uvec2(4294967295u);
        while(true) {
          if (all(equal(tint_loop_idx, uvec2(0u)))) {
            break;
          }
          return;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
}
