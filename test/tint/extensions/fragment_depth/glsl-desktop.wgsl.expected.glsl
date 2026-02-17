//
// any
//
#version 460
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
float any_inner() {
  return 1.0f;
}
void main() {
  float v = any_inner();
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
//
// less
//
#version 460
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
layout(depth_less) out float gl_FragDepth;
float less_inner() {
  return 1.0f;
}
void main() {
  float v = less_inner();
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
//
// greater
//
#version 460
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
layout(depth_greater) out float gl_FragDepth;
float greater_inner() {
  return 1.0f;
}
void main() {
  float v = greater_inner();
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
