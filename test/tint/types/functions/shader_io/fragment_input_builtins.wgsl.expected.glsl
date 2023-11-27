#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;

void tint_symbol(vec4 position, bool front_facing, uint sample_index, uint sample_mask) {
  if (front_facing) {
    vec4 foo = position;
    uint bar = (sample_index + sample_mask);
  }
}

void main() {
  tint_symbol(gl_FragCoord, gl_FrontFacing, uint(gl_SampleID), uint(gl_SampleMaskIn[0]));
  return;
}
