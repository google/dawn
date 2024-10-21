#version 310 es


struct tint_push_constant_struct {
  uint tint_first_instance;
};

struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

layout(location = 0) uniform tint_push_constant_struct tint_push_constants;
vec4 tint_symbol_inner(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return vec4(0.0f);
}
void main() {
  uint v = uint(gl_VertexID);
  uint v_1 = uint(gl_InstanceID);
  gl_Position = tint_symbol_inner(VertexInputs(v, (v_1 + tint_push_constants.tint_first_instance)));
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
