#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  TintTextureUniformData inner;
} v;
layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uint level = v.inner.tint_builtin_value_0;
    while(true) {
      if ((level > 0u)) {
      } else {
        break;
      }
      {
      }
      continue;
    }
  }
}
