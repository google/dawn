#version 310 es

struct tint_symbol_1 {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_2_block_ubo {
  tint_symbol_1 inner;
} tint_symbol_2;

void e() {
  {
    {
      for(uint level = tint_symbol_2.inner.texture_builtin_value_0; (level > 0u); ) {
      }
    }
  }
}

layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;
void main() {
  e();
  return;
}
