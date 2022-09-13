bug/dawn/947.wgsl:59:20 warning: 'textureSample' must only be called from uniform control flow
    var srcColor = textureSample(myTexture, mySampler, texcoord);
                   ^^^^^^^^^^^^^

bug/dawn/947.wgsl:55:5 note: control flow depends on non-uniform value
    if (!all(clampedTexcoord == texcoord)) {
    ^^

bug/dawn/947.wgsl:55:33 note: reading from user-defined input 'texcoord' may result in a non-uniform value
    if (!all(clampedTexcoord == texcoord)) {
                                ^^^^^^^^

#version 310 es

layout(location = 0) out vec2 texcoords_1;
layout(binding = 0, std140) uniform Uniforms_ubo {
  vec2 u_scale;
  vec2 u_offset;
} uniforms;

struct VertexOutputs {
  vec2 texcoords;
  vec4 position;
};

VertexOutputs vs_main(uint VertexIndex) {
  vec2 texcoord[3] = vec2[3](vec2(-0.5f, 0.0f), vec2(1.5f, 0.0f), vec2(0.5f, 2.0f));
  VertexOutputs tint_symbol = VertexOutputs(vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.position = vec4(((texcoord[VertexIndex] * 2.0f) - vec2(1.0f)), 0.0f, 1.0f);
  bool flipY = (uniforms.u_scale.y < 0.0f);
  if (flipY) {
    tint_symbol.texcoords = ((((texcoord[VertexIndex] * uniforms.u_scale) + uniforms.u_offset) * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f));
  } else {
    tint_symbol.texcoords = ((((texcoord[VertexIndex] * vec2(1.0f, -1.0f)) + vec2(0.0f, 1.0f)) * uniforms.u_scale) + uniforms.u_offset);
  }
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutputs inner_result = vs_main(uint(gl_VertexID));
  texcoords_1 = inner_result.texcoords;
  gl_Position = inner_result.position;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

layout(location = 0) in vec2 texcoord_1;
layout(location = 0) out vec4 value;
struct Uniforms {
  vec2 u_scale;
  vec2 u_offset;
};

struct VertexOutputs {
  vec2 texcoords;
  vec4 position;
};

bool tint_discard = false;
uniform highp sampler2D myTexture_mySampler;

vec4 fs_main(vec2 texcoord) {
  vec2 clampedTexcoord = clamp(texcoord, vec2(0.0f), vec2(1.0f));
  if (!(all(equal(clampedTexcoord, texcoord)))) {
    tint_discard = true;
    return vec4(0.0f);
  }
  vec4 srcColor = texture(myTexture_mySampler, texcoord);
  return srcColor;
}

void tint_discard_func() {
  discard;
}

void main() {
  vec4 inner_result = fs_main(texcoord_1);
  if (tint_discard) {
    tint_discard_func();
    return;
  }
  value = inner_result;
  return;
}
