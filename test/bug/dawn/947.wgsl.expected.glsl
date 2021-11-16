#version 310 es
precision mediump float;


layout (binding = 0) uniform Uniforms_1 {
  vec2 u_scale;
  vec2 u_offset;
} uniforms;

struct VertexOutputs {
  vec2 texcoords;
  vec4 position;
};
struct tint_symbol_2 {
  uint VertexIndex;
};
struct tint_symbol_3 {
  vec2 texcoords;
  vec4 position;
};

VertexOutputs vs_main_inner(uint VertexIndex) {
  vec2 texcoord[3] = vec2[3](vec2(-0.5f, 0.0f), vec2(1.5f, 0.0f), vec2(0.5f, 2.0f));
  VertexOutputs tint_symbol = VertexOutputs(vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.position = vec4(((texcoord[VertexIndex] * 2.0f) - vec2(1.0f, 1.0f)), 0.0f, 1.0f);
  bool flipY = (uniforms.u_scale.y < 0.0f);
  if (flipY) {
    tint_symbol.texcoords = ((((texcoord[VertexIndex] * uniforms.u_scale) + uniforms.u_offset) * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f));
  } else {
    tint_symbol.texcoords = ((((texcoord[VertexIndex] * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f)) * uniforms.u_scale) + uniforms.u_offset);
  }
  return tint_symbol;
}

struct tint_symbol_5 {
  vec2 texcoord;
};
struct tint_symbol_6 {
  vec4 value;
};

tint_symbol_3 vs_main(tint_symbol_2 tint_symbol_1) {
  VertexOutputs inner_result = vs_main_inner(tint_symbol_1.VertexIndex);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.texcoords = inner_result.texcoords;
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
out vec2 texcoords;
void main() {
  tint_symbol_2 inputs;
  inputs.VertexIndex = uint(gl_VertexID);
  tint_symbol_3 outputs;
  outputs = vs_main(inputs);
  texcoords = outputs.texcoords;
  gl_Position = outputs.position;
  gl_Position.y = -gl_Position.y;
}


#version 310 es
precision mediump float;

struct VertexOutputs {
  vec2 texcoords;
  vec4 position;
};
struct tint_symbol_2 {
  uint VertexIndex;
};
struct tint_symbol_3 {
  vec2 texcoords;
  vec4 position;
};


uniform highp sampler2D myTexture;

struct tint_symbol_5 {
  vec2 texcoord;
};
struct tint_symbol_6 {
  vec4 value;
};

vec4 fs_main_inner(vec2 texcoord) {
  vec2 clampedTexcoord = clamp(texcoord, vec2(0.0f, 0.0f), vec2(1.0f, 1.0f));
  if (!(all(equal(clampedTexcoord, texcoord)))) {
    discard;
  }
  vec4 srcColor = texture(myTexture, texcoord);
  return srcColor;
}

tint_symbol_6 fs_main(tint_symbol_5 tint_symbol_4) {
  vec4 inner_result_1 = fs_main_inner(tint_symbol_4.texcoord);
  tint_symbol_6 wrapper_result_1 = tint_symbol_6(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
in vec2 texcoord;
out vec4 value;
void main() {
  tint_symbol_5 inputs;
  inputs.texcoord = texcoord;
  tint_symbol_6 outputs;
  outputs = fs_main(inputs);
  value = outputs.value;
}


