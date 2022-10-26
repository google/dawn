SKIP: FAILED

#version 310 es

layout(rgba32ui) uniform highp writeonly uimage1D arg_0;
void textureDimensions_09140b() {
  uint res = uint(imageSize(arg_0));
}

vec4 vertex_main() {
  textureDimensions_09140b();
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
ERROR: 0:3: 'uimage1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

layout(rgba32ui) uniform highp writeonly uimage1D arg_0;
void textureDimensions_09140b() {
  uint res = uint(imageSize(arg_0));
}

void fragment_main() {
  textureDimensions_09140b();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'uimage1D' : Reserved word. 
WARNING: 0:4: 'layout' : useless application of layout qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(rgba32ui) uniform highp writeonly uimage1D arg_0;
void textureDimensions_09140b() {
  uint res = uint(imageSize(arg_0));
}

void compute_main() {
  textureDimensions_09140b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:3: 'uimage1D' : Reserved word. 
WARNING: 0:3: 'layout' : useless application of layout qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



