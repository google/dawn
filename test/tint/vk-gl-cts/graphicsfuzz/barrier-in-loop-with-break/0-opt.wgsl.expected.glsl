vk-gl-cts/graphicsfuzz/barrier-in-loop-with-break/0-opt.wgsl:1:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type RTArr = @stride(4) array<i32>;
              ^^^^^^

#version 310 es

struct buf1 {
  vec2 injectionSwitch;
};

uvec3 tint_symbol = uvec3(0u, 0u, 0u);
layout(binding = 0, std430) buffer doesNotMatter_1 {
  int global_seed;
  int data[];
} x_7;
layout(binding = 1) uniform buf1_1 {
  vec2 injectionSwitch;
} x_10;

void main_1() {
  int lid = 0;
  int val = 0;
  int i = 0;
  uint x_40 = tint_symbol.x;
  lid = int(x_40);
  int x_43 = x_7.global_seed;
  val = x_43;
  i = 0;
  {
    for(; (i < 2); i = (i + 1)) {
      if ((lid > 0)) {
        int x_58 = x_7.data[(lid - 1)];
        val = (val + x_58);
        float x_62 = x_10.injectionSwitch.x;
        if ((x_62 > 100.0f)) {
          break;
        }
      }
      barrier();
    }
  }
  if ((lid == 0)) {
    x_7.data[0] = 42;
  }
  return;
}

void tint_symbol_1(uvec3 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
}

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_LocalInvocationID);
  return;
}
