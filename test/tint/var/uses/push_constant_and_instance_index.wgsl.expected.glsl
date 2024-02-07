#version 310 es

struct PushConstants {
  float inner;
  uint first_instance;
};

layout(location=0) uniform PushConstants a;
vec4 tint_symbol(uint b) {
  return vec4((a.inner + float((b + a.first_instance))));
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_InstanceID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
