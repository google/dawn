#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;
precision highp int;

struct PushConstants {
  float min_depth;
  float max_depth;
};

layout(location=0) uniform PushConstants push_constants;
float clamp_frag_depth(float v) {
  return clamp(v, push_constants.min_depth, push_constants.max_depth);
}

struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

FragmentOutputs clamp_frag_depth_FragmentOutputs(FragmentOutputs s) {
  FragmentOutputs tint_symbol_1 = FragmentOutputs(clamp_frag_depth(s.frag_depth), s.sample_mask);
  return tint_symbol_1;
}

FragmentOutputs tint_symbol() {
  FragmentOutputs tint_symbol_2 = FragmentOutputs(1.0f, 1u);
  return clamp_frag_depth_FragmentOutputs(tint_symbol_2);
}

void main() {
  FragmentOutputs inner_result = tint_symbol();
  gl_FragDepth = inner_result.frag_depth;
  gl_SampleMask[0] = int(inner_result.sample_mask);
  return;
}
