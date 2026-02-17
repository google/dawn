//
// any
//
#version 310 es
precision highp float;
precision highp int;


struct FragDepthAnyOutput {
  float frag_depth;
};

layout(location = 0) uniform uint tint_immediates[2];
FragDepthAnyOutput any_inner() {
  return FragDepthAnyOutput(1.0f);
}
void main() {
  float v = any_inner().frag_depth;
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
//
// less
//
#version 310 es
#extension GL_EXT_conservative_depth: require
precision highp float;
precision highp int;


struct FragDepthLessOutput {
  float frag_depth;
};

layout(location = 0) uniform uint tint_immediates[2];
layout(depth_less) out float gl_FragDepth;
FragDepthLessOutput less_inner() {
  return FragDepthLessOutput(1.0f);
}
void main() {
  float v = less_inner().frag_depth;
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
//
// greater
//
#version 310 es
#extension GL_EXT_conservative_depth: require
precision highp float;
precision highp int;


struct FragDepthGreaterOutput {
  float frag_depth;
};

layout(location = 0) uniform uint tint_immediates[2];
layout(depth_greater) out float gl_FragDepth;
FragDepthGreaterOutput greater_inner() {
  return FragDepthGreaterOutput(1.0f);
}
void main() {
  float v = greater_inner().frag_depth;
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
