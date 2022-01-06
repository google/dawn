SKIP: FAILED

#version 310 es
precision mediump float;

struct doesNotMatter {
  int global_seed;
  int data[];
};
struct buf1 {
  vec2 injectionSwitch;
};

uvec3 tint_symbol = uvec3(0u, 0u, 0u);
layout (binding = 0) buffer doesNotMatter_1 {
  int global_seed;
  int data[];
} x_7;
layout (binding = 1) uniform buf1_1 {
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
      memoryBarrierShared();
    }
  }
  if ((lid == 0)) {
    x_7.data[0] = 42;
  }
  return;
}

struct tint_symbol_4 {
  uvec3 tint_symbol_2;
};

void tint_symbol_1_inner(uvec3 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
}

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  return;
}
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_LocalInvocationID;
  tint_symbol_1(inputs);
}


Error parsing GLSL shader:
ERROR: 0:6: '' : array size required 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



