SKIP: FAILED

#version 310 es

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleLevel_aab3b9() {
  vec4 res = textureLod(arg_0_arg_1, vec4(vec3(1.0f), float(1u)), 1.0f);
}

vec4 vertex_main() {
  textureSampleLevel_aab3b9();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleLevel_aab3b9() {
  vec4 res = textureLod(arg_0_arg_1, vec4(vec3(1.0f), float(1u)), 1.0f);
}

void fragment_main() {
  textureSampleLevel_aab3b9();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

uniform highp samplerCubeArray arg_0_arg_1;

void textureSampleLevel_aab3b9() {
  vec4 res = textureLod(arg_0_arg_1, vec4(vec3(1.0f), float(1u)), 1.0f);
}

void compute_main() {
  textureSampleLevel_aab3b9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'samplerCubeArray' : Reserved word. 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



