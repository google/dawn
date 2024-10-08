#version 310 es


struct tint_push_constant_struct {
  uint tint_first_instance;
};

struct Output {
  vec4 Position;
  vec4 color;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
Output tint_symbol_inner(uint VertexIndex, uint InstanceIndex) {
  vec2 zv[4] = vec2[4](vec2(0.20000000298023223877f), vec2(0.30000001192092895508f), vec2(-0.10000000149011611938f), vec2(1.10000002384185791016f));
  float z = zv[InstanceIndex][0u];
  Output tint_symbol_1 = Output(vec4(0.0f), vec4(0.0f));
  tint_symbol_1.Position = vec4(0.5f, 0.5f, z, 1.0f);
  vec4 colors[4] = vec4[4](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f));
  tint_symbol_1.color = colors[InstanceIndex];
  return tint_symbol_1;
}
void main() {
  uint v = uint(gl_VertexID);
  uint v_1 = uint(gl_InstanceID);
  Output v_2 = tint_symbol_inner(v, (v_1 + tint_push_constants.tint_first_instance));
  gl_Position = v_2.Position;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_symbol_loc0_Output = v_2.color;
  gl_PointSize = 1.0f;
}
