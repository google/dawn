SKIP: FAILED

intrinsics/gen/ignore/ad88be.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0;

void ignore_ad88be() {
  arg_0;
}

struct tint_symbol {
  vec4 value;
};

vec4 vertex_main_inner() {
  ignore_ad88be();
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
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0;

void ignore_ad88be() {
  arg_0;
}

struct tint_symbol {
  vec4 value;
};

void fragment_main() {
  ignore_ad88be();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0;

void ignore_ad88be() {
  arg_0;
}

struct tint_symbol {
  vec4 value;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  ignore_ad88be();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



