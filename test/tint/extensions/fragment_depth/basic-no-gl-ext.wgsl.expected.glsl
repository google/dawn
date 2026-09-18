//
// less
//
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
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
#version 310 es
precision highp float;
precision highp int;

layout(location = 0) uniform uint tint_immediates[2];
float greater_inner() {
  return 1.0f;
}
void main() {
  float v = greater_inner();
  gl_FragDepth = clamp(v, uintBitsToFloat(tint_immediates[0u]), uintBitsToFloat(tint_immediates[1u]));
}
