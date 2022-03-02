#version 310 es

struct buf1 {
  vec2 injectionSwitch;
};

struct buf2 {
  vec2 resolution;
};

uvec3 tint_symbol = uvec3(0u, 0u, 0u);
layout(binding = 1) uniform buf1_1 {
  vec2 injectionSwitch;
} x_10;

layout(binding = 2) uniform buf2_1 {
  vec2 resolution;
} x_13;

layout(binding = 0, std430) buffer doesNotMatter_1 {
  int x_compute_data[];
} x_15;
void main_1() {
  float A[1] = float[1](0.0f);
  int i = 0;
  vec4 value = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  int m = 0;
  int l = 0;
  int n = 0;
  A[0] = 0.0f;
  i = 0;
  {
    for(; (i < 50); i = (i + 1)) {
      if ((i > 0)) {
        float x_68 = A[0];
        float x_70 = A[0];
        A[0] = (x_70 + x_68);
      }
    }
  }
  while (true) {
    uint x_80 = tint_symbol.x;
    if ((x_80 < 100u)) {
      value = vec4(0.0f, 0.0f, 0.0f, 1.0f);
      m = 0;
      {
        for(; (m < 1); m = (m + 1)) {
          l = 0;
          {
            for(; (l < 1); l = (l + 1)) {
              float x_100 = x_10.injectionSwitch.x;
              float x_102 = x_10.injectionSwitch.y;
              if ((x_100 > x_102)) {
                return;
              }
            }
          }
        }
      }
      n = 0;
      {
        for(; (n < 1); n = (n + 1)) {
          float x_118 = x_10.injectionSwitch.x;
          float x_120 = x_10.injectionSwitch.y;
          if ((x_118 > x_120)) {
            barrier();
          }
        }
      }
    } else {
      uint x_127 = tint_symbol.x;
      if ((x_127 < 120u)) {
        float x_133 = A[0];
        float x_135 = x_13.resolution.x;
        float x_138 = A[0];
        float x_140 = x_13.resolution.y;
        value = vec4((x_133 / x_135), (x_138 / x_140), 0.0f, 1.0f);
      } else {
        float x_144 = x_10.injectionSwitch.x;
        float x_146 = x_10.injectionSwitch.y;
        if ((x_144 > x_146)) {
          {
            if (false) {
            } else {
              break;
            }
          }
          continue;
        }
      }
    }
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  float x_151 = value.x;
  x_15.x_compute_data[0] = int(x_151);
  float x_155 = value.y;
  x_15.x_compute_data[1] = int(x_155);
  float x_159 = value.z;
  x_15.x_compute_data[2] = int(x_159);
  float x_163 = value.w;
  x_15.x_compute_data[3] = int(x_163);
  return;
}

void tint_symbol_1(uvec3 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_GlobalInvocationID);
  return;
}
