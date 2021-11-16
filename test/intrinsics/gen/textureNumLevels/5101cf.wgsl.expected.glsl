SKIP: FAILED

#version 310 es
precision mediump float;

uniform highp usampler2DArray arg_0;

void textureNumLevels_5101cf() {
  int res = textureQueryLevels(arg_0);;
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  textureNumLevels_5101cf();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  vec4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = tint_symbol(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
void main() {
  tint_symbol outputs;
  outputs = vertex_main();
  gl_Position = outputs.value;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:7: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:7: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp usampler2DArray arg_0;

void textureNumLevels_5101cf() {
  int res = textureQueryLevels(arg_0);;
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  textureNumLevels_5101cf();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:7: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:7: '=' :  cannot convert from ' const float' to ' temp mediump int'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp usampler2DArray arg_0;

void textureNumLevels_5101cf() {
  int res = textureQueryLevels(arg_0);;
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  textureNumLevels_5101cf();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:7: 'textureQueryLevels' : no matching overloaded function found 
ERROR: 0:7: '=' :  cannot convert from ' const float' to ' temp highp int'
ERROR: 0:7: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



