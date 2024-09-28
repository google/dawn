#version 310 es


struct tint_symbol_2 {
  float tint_symbol_1;
};

layout(location = 0) uniform tint_symbol_2 v;
vec4 tint_symbol_inner(uint b) {
  float v_1 = v.tint_symbol_1;
  return vec4((v_1 + float(b)));
}
void main() {
  gl_Position = tint_symbol_inner(uint(gl_InstanceID));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
