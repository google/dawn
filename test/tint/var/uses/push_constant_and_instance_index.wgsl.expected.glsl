#version 310 es


struct tint_push_constant_struct {
  float user_constant;
  uint tint_first_instance;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
vec4 tint_symbol_inner(uint b) {
  float v = tint_push_constants.user_constant;
  return vec4((v + float(b)));
}
void main() {
  uint v_1 = uint(gl_InstanceID);
  gl_Position = tint_symbol_inner((v_1 + tint_push_constants.tint_first_instance));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
