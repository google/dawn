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


void modf_180fed() {
  modf_result res = tint_modf(1.0f);
}

vec4 vertex_main() {
  modf_180fed();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:12: '{ } style initializers' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



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


void modf_180fed() {
  modf_result res = tint_modf(1.0f);
}

void fragment_main() {
  modf_180fed();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:12: '{ } style initializers' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



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


void modf_180fed() {
  modf_result res = tint_modf(1.0f);
}

void compute_main() {
  modf_180fed();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:12: '{ } style initializers' : not supported with this profile: es
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



