SKIP: FAILED

#version 310 es

struct doesNotMatter {
  float x_compute_data[];
};

doesNotMatter x_9;
float nb_mod_() {
  float s = 0.0f;
  int i = 0;
  int GLF_live1i = 0;
  int GLF_live1_looplimiter2 = 0;
  float x_51 = 0.0f;
  float x_56 = 0.0f;
  s = 0.0f;
  i = 5;
  {
    while(true) {
      float x_50 = 0.0f;
      x_56 = 0.0f;
      if (true) {
      } else {
        break;
      }
      GLF_live1i = 0;
      {
        while(true) {
          x_51 = 0.0f;
          if (true) {
          } else {
            break;
          }
          if (false) {
            x_50 = 1.0f;
            s = x_50;
            x_51 = x_50;
            break;
          }
          return 42.0f;
        }
      }
      if ((5.0f <= x_51)) {
        x_56 = x_51;
        break;
      }
      return 42.0f;
    }
  }
  return x_56;
}
void main_1() {
  float x_32 = nb_mod_();
  x_9.x_compute_data[0] = x_32;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: '' : array size required 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
