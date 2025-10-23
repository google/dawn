#version 310 es


struct FSUniformData {
  vec4 k[7];
  ivec2 size;
  uint tint_pad_0;
  uint tint_pad_1;
};

uint idx = 0u;
layout(binding = 0, std430)
buffer FSUniforms_1_ssbo {
  FSUniformData fsUniformData[];
} _storage;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec2 vec = ivec2(0);
  {
    uvec2 tint_loop_idx = uvec2(4294967295u);
    while(true) {
      if (all(equal(tint_loop_idx, uvec2(0u)))) {
        break;
      }
      int v = vec.y;
      uint v_1 = idx;
      uint v_2 = min(v_1, (uint(_storage.fsUniformData.length()) - 1u));
      if ((v >= _storage.fsUniformData[v_2].size.y)) {
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
