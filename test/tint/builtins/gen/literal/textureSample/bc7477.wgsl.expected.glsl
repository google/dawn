SKIP: FAILED

#version 310 es
precision highp float;

uniform highp samplerCubeArray arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_bc7477() {
  vec4 res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1u)));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_bc7477();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArray' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



