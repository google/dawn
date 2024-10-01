#version 310 es


struct TintTextureUniformData {
  uint tint_builtin_value_0;
};

uniform highp sampler2D tint_symbol;
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  TintTextureUniformData tint_symbol_1;
} v;
layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;
void main() {
  {
    uint level = v.tint_symbol_1.tint_builtin_value_0;
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
