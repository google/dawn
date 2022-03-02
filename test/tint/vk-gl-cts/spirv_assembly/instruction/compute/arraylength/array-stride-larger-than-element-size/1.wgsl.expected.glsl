vk-gl-cts/spirv_assembly/instruction/compute/arraylength/array-stride-larger-than-element-size/1.wgsl:5:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type RTArr = @stride(4) array<u32>;
              ^^^^^^

#version 310 es

struct Buf1 {
  int result;
};

layout(binding = 1, std430) buffer Buf1_1 {
  int result;
} x_4;
layout(binding = 0, std430) buffer Buf0_1 {
  uint values[];
} x_7;
void main_1() {
  uint i = 0u;
  x_4.result = 1;
  i = 0u;
  {
    for(; (i < 512u); i = (i + uint(1))) {
      uint x_39 = x_7.values[(i * 2u)];
      if ((x_39 != i)) {
        x_4.result = 0;
      }
    }
  }
  return;
}

void tint_symbol() {
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
