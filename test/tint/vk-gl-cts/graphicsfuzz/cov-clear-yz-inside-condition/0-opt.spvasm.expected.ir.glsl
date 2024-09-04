SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct buf1 {
  float zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_9;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  vec2 v0 = vec2(0.0f);
  vec4 v1 = vec4(0.0f);
  vec4 x_57 = vec4(0.0f);
  v0 = vec2(x_6.x_GLF_uniform_float_values[0].el);
  vec4 x_36 = vec4(v0.x);
  v1 = x_36;
  if (!((x_9.zero == x_6.x_GLF_uniform_float_values[0].el))) {
    if ((x_9.zero == x_6.x_GLF_uniform_float_values[1].el)) {
      return;
    }
    x_57 = vec4(x_36[0u], (x_36.yz - vec2(x_6.x_GLF_uniform_float_values[0].el)).xy, x_36[3u]);
    v1 = x_57;
  } else {
    continue_execution = false;
  }
  x_GLF_color = x_57;
}
main_out main() {
  main_1();
  main_out v = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
