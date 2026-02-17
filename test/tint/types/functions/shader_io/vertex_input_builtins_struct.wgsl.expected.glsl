#version 310 es


struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

layout(location = 0) uniform uint tint_immediates[1];
vec4 main_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return vec4(0.0f);
}
void main() {
  uint v = uint(gl_VertexID);
  uint v_1 = uint(gl_InstanceID);
  vec4 v_2 = main_inner(VertexInputs(v, (v_1 + tint_immediates[0u])));
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
