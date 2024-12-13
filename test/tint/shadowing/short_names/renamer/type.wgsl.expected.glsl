#version 310 es


struct tint_struct {
  int member_0;
};

vec4 v(uint v_1) {
  tint_struct v_2 = tint_struct(1);
  float v_3 = float(v_2.member_0);
  bool v_4 = bool(v_3);
  return mix(vec4(0.0f), vec4(1.0f), bvec4(v_4));
}
void main() {
  gl_Position = v(uint(gl_VertexID));
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
