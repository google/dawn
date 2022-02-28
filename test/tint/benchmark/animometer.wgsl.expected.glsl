#version 310 es

float tint_float_modulo(float lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


layout(location = 0) in vec4 position_1;
layout(location = 1) in vec4 color_1;
layout(location = 0) out vec4 v_color_1;
struct Time {
  float value;
};

struct Uniforms {
  float scale;
  float offsetX;
  float offsetY;
  float scalar;
  float scalarOffset;
};

layout(binding = 0) uniform Time_1 {
  float value;
} time;

layout(binding = 1) uniform Uniforms_1 {
  float scale;
  float offsetX;
  float offsetY;
  float scalar;
  float scalarOffset;
} uniforms;

struct VertexOutput {
  vec4 Position;
  vec4 v_color;
};

VertexOutput vert_main(vec4 position, vec4 color) {
  float fade = tint_float_modulo((uniforms.scalarOffset + ((time.value * uniforms.scalar) / 10.0f)), 1.0f);
  if ((fade < 0.5f)) {
    fade = (fade * 2.0f);
  } else {
    fade = ((1.0f - fade) * 2.0f);
  }
  float xpos = (position.x * uniforms.scale);
  float ypos = (position.y * uniforms.scale);
  float angle = ((3.141590118f * 2.0f) * fade);
  float xrot = ((xpos * cos(angle)) - (ypos * sin(angle)));
  float yrot = ((xpos * sin(angle)) + (ypos * cos(angle)));
  xpos = (xrot + uniforms.offsetX);
  ypos = (yrot + uniforms.offsetY);
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.v_color = (vec4(fade, (1.0f - fade), 0.0f, 1.0f) + color);
  tint_symbol.Position = vec4(xpos, ypos, 0.0f, 1.0f);
  return tint_symbol;
}

void main() {
  VertexOutput inner_result = vert_main(position_1, color_1);
  gl_Position = inner_result.Position;
  v_color_1 = inner_result.v_color;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

layout(location = 0) in vec4 v_color_1;
layout(location = 0) out vec4 value_1;
struct Time {
  float value;
};

struct Uniforms {
  float scale;
  float offsetX;
  float offsetY;
  float scalar;
  float scalarOffset;
};

struct VertexOutput {
  vec4 Position;
  vec4 v_color;
};

vec4 frag_main(vec4 v_color) {
  return v_color;
}

void main() {
  vec4 inner_result = frag_main(v_color_1);
  value_1 = inner_result;
  return;
}
