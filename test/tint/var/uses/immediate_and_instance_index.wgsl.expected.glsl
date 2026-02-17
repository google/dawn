#version 310 es

layout(location = 0) uniform uint tint_immediates[2];
vec4 main_inner(uint b) {
  float v = uintBitsToFloat(tint_immediates[0u]);
  return vec4((v + float(b)));
}
void main() {
  uint v_1 = uint(gl_InstanceID);
  vec4 v_2 = main_inner((v_1 + tint_immediates[1u]));
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
