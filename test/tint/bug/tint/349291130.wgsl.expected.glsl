#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    uint level = v.metadata[(0u / 4u)][(0u % 4u)];
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      if ((level > 0u)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
      }
      continue;
    }
  }
}
