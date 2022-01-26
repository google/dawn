SKIP: FAILED

#version 310 es
precision mediump float;

struct modf_result {
  float fract;
  float whole;
};

modf_result tint_modf(float param_0) {
  float whole;
  float fract = modf(param_0, whole);
  modf_result result = {fract, whole};
  return result;
}


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}

void main() {
  unused_entry_point();
}

void i() {
  float s = tint_modf(1.0f).whole;
}

Error parsing GLSL shader:
ERROR: 0:12: '{ } style initializers' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



