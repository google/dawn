#version 310 es


struct VSOutputs {
  int result;
  vec4 position;
};

struct VSInput {
  vec4 val;
};

layout(location = 0) in vec4 vsMain_loc0_Input;
layout(location = 0) flat out int tint_interstage_location0;
VSOutputs vsMain_inner(VSInput vtxIn) {
  return VSOutputs(1, vtxIn.val);
}
void main() {
  VSOutputs v = vsMain_inner(VSInput(vsMain_loc0_Input.zyxw));
  tint_interstage_location0 = v.result;
  gl_Position = vec4(v.position.x, -(v.position.y), ((2.0f * v.position.z) - v.position.w), v.position.w);
  gl_PointSize = 1.0f;
}
