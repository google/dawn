#version 310 es


struct FSUniformData {
  vec4 k[7];
  ivec2 size;
  uint tint_pad_0;
  uint tint_pad_1;
};

uint idx = 0u;
layout(binding = 2, std430)
buffer FSUniforms_1_ssbo {
  FSUniformData fsUniformData[];
} _storage;
void tint_symbol() {
  ivec2 vec = ivec2(0);
  {
    while(true) {
      int v = vec.y;
      uint v_1 = idx;
      if ((v >= _storage.fsUniformData[v_1].size.y)) {
        break;
      }
      {
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
