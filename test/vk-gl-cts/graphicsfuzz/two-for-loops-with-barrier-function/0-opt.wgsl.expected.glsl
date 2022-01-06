SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  vec2 injectionSwitch;
};
struct doesNotMatter {
  uint x_compute_data[];
};

vec4 GLF_live2gl_FragCoord = vec4(0.0f, 0.0f, 0.0f, 0.0f);
layout (binding = 1) uniform buf0_1 {
  vec2 injectionSwitch;
} x_9;
layout (binding = 0) buffer doesNotMatter_1 {
  uint x_compute_data[];
} x_12;

void main_1() {
  int GLF_live2_looplimiter1 = 0;
  int i = 0;
  int j = 0;
  float GLF_dead3x = 0.0f;
  float x_51 = 0.0f;
  int GLF_dead3k = 0;
  GLF_live2_looplimiter1 = 0;
  i = 0;
  {
    for(; (i < 1); i = (i + 1)) {
      if ((GLF_live2_looplimiter1 >= 3)) {
        j = 0;
        {
          for(; (j < 1); j = (j + 1)) {
            float x_13 = GLF_live2gl_FragCoord.x;
            if ((int(x_13) < 120)) {
            } else {
              memoryBarrierShared();
            }
          }
        }
        break;
      }
    }
  }
  float x_81 = x_9.injectionSwitch.x;
  float x_83 = x_9.injectionSwitch.y;
  if ((x_81 > x_83)) {
    float x_14 = GLF_live2gl_FragCoord.x;
    x_51 = x_14;
  } else {
    x_51 = 0.0f;
  }
  GLF_dead3x = x_51;
  GLF_dead3k = 0;
  {
    for(; (GLF_dead3k < 2); GLF_dead3k = (GLF_dead3k + 1)) {
      if ((GLF_dead3x > 4.0f)) {
        break;
      }
      float x_16 = GLF_live2gl_FragCoord.x;
      GLF_dead3x = x_16;
      memoryBarrierShared();
    }
  }
  x_12.x_compute_data[0] = 42u;
  return;
}

layout(local_size_x = 1, local_size_y = 18, local_size_z = 6) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:8: '' : array size required 
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



