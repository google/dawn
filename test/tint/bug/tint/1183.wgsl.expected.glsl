#version 310 es
precision highp float;

layout(location = 0) out vec4 value;
uniform highp sampler2D t_s;

vec4 f() {
  return textureOffset(t_s, vec2(0.0f), ivec2(4, 6));
}

void main() {
  vec4 inner_result = f();
  value = inner_result;
  return;
}
