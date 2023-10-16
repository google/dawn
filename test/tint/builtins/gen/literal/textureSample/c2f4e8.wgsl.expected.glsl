SKIP: FAILED

#version 310 es
precision highp float;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSample_c2f4e8() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1)), 0.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_c2f4e8();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArrayShadow' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



