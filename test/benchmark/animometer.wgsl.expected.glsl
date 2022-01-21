SKIP: FAILED

#version 310 es
precision mediump float;

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

layout (binding = 0) uniform Time_1 {
  float value;
} time;
layout (binding = 1) uniform Uniforms_1 {
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
struct tint_symbol_2 {
  vec4 position;
  vec4 color;
};
struct tint_symbol_3 {
  vec4 v_color;
  vec4 Position;
};

VertexOutput vert_main_inner(vec4 position, vec4 color) {
  float fade = ((uniforms.scalarOffset + ((time.value * uniforms.scalar) / 10.0f)) % 1.0f);
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

struct tint_symbol_5 {
  vec4 v_color;
};
struct tint_symbol_6 {
  vec4 value;
};

tint_symbol_3 vert_main(tint_symbol_2 tint_symbol_1) {
  VertexOutput inner_result = vert_main_inner(tint_symbol_1.position, tint_symbol_1.color);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.Position = inner_result.Position;
  wrapper_result.v_color = inner_result.v_color;
  return wrapper_result;
}
in vec4 position;
in vec4 color;
out vec4 v_color;
void main() {
  tint_symbol_2 inputs;
  inputs.position = position;
  inputs.color = color;
  tint_symbol_3 outputs;
  outputs = vert_main(inputs);
  v_color = outputs.v_color;
  gl_Position = outputs.Position;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:40: '%' :  wrong operand types: no operation '%' exists that takes a left-hand operand of type ' temp mediump float' and a right operand of type ' const float' (or there is no acceptable conversion)
ERROR: 0:40: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;

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
struct tint_symbol_2 {
  vec4 position;
  vec4 color;
};
struct tint_symbol_3 {
  vec4 v_color;
  vec4 Position;
};
struct tint_symbol_5 {
  vec4 v_color;
};
struct tint_symbol_6 {
  vec4 value;
};

vec4 frag_main_inner(vec4 v_color) {
  return v_color;
}

tint_symbol_6 frag_main(tint_symbol_5 tint_symbol_4) {
  vec4 inner_result_1 = frag_main_inner(tint_symbol_4.v_color);
  tint_symbol_6 wrapper_result_1 = tint_symbol_6(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
in vec4 v_color;
out vec4 value;
void main() {
  tint_symbol_5 inputs;
  inputs.v_color = v_color;
  tint_symbol_6 outputs;
  outputs = frag_main(inputs);
  value = outputs.value;
}


