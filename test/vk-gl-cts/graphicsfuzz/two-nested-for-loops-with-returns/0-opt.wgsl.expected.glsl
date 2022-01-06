SKIP: FAILED

#version 310 es
precision mediump float;

struct doesNotMatter {
  float x_compute_data[];
};

layout (binding = 0) buffer doesNotMatter_1 {
  float x_compute_data[];
} x_9;

float nb_mod_() {
  float s = 0.0f;
  int i = 0;
  int GLF_live1i = 0;
  int GLF_live1_looplimiter2 = 0;
  float x_51 = 0.0f;
  float x_56_phi = 0.0f;
  s = 0.0f;
  i = 5;
  while (true) {
    float x_50 = 0.0f;
    float x_51_phi = 0.0f;
    x_56_phi = 0.0f;
    if ((5 < 800)) {
    } else {
      break;
    }
    GLF_live1i = 0;
    while (true) {
      x_51_phi = 0.0f;
      if ((0 < 20)) {
      } else {
        break;
      }
      if ((0 >= 5)) {
        x_50 = (0.0f + 1.0f);
        s = x_50;
        x_51_phi = x_50;
        break;
      }
      return 42.0f;
    }
    x_51 = x_51_phi;
    if ((float(5) <= x_51)) {
      x_56_phi = x_51;
      break;
    }
    return 42.0f;
  }
  return x_56_phi;
}

void main_1() {
  float x_32 = nb_mod_();
  x_9.x_compute_data[0] = x_32;
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  main_1();
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:5: '' : array size required 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



