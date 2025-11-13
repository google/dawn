//
// any
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

struct FragDepthAnyOutput {
  float frag_depth;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
FragDepthAnyOutput any_inner() {
  return FragDepthAnyOutput(1.0f);
}
void main() {
  float v = any_inner().frag_depth;
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
//
// less
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

struct FragDepthLessOutput {
  float frag_depth;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
FragDepthLessOutput less_inner() {
  return FragDepthLessOutput(1.0f);
}
void main() {
  float v = less_inner().frag_depth;
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
//
// greater
//
#version 310 es
precision highp float;
precision highp int;


struct tint_immediate_struct {
  float tint_frag_depth_min;
  float tint_frag_depth_max;
};

struct FragDepthGreaterOutput {
  float frag_depth;
};

layout(location = 0) uniform tint_immediate_struct tint_immediates;
FragDepthGreaterOutput greater_inner() {
  return FragDepthGreaterOutput(1.0f);
}
void main() {
  float v = greater_inner().frag_depth;
  gl_FragDepth = clamp(v, tint_immediates.tint_frag_depth_min, tint_immediates.tint_frag_depth_max);
}
