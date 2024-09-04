SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0.40000000596046447754f, 0.599609375hf, vec4(0.0f));
}
void main(Interface colors) {
  float r = colors.col1;
  float16_t g = colors.col2;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'structure' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct Interface {
  float col1;
  float16_t col2;
  vec4 pos;
};
precision highp float;
precision highp int;


Interface main() {
  return Interface(0.40000000596046447754f, 0.599609375hf, vec4(0.0f));
}
void main(Interface colors) {
  float r = colors.col1;
  float16_t g = colors.col2;
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
