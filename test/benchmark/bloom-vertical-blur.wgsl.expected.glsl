#version 310 es
precision mediump float;

const vec2 bloomDir = vec2(0.0f, 1.0f);
float weights[3] = float[3](0.227027029f, 0.31621623f, 0.07027027f);

struct BloomUniforms {
  float radius;
  float dim;
};

layout (binding = 0) uniform BloomUniforms_1 {
  float radius;
  float dim;
} bloom;
uniform highp sampler2D bloomTexture;


struct FragmentInput {
  vec2 texCoord;
};

vec4 getGaussianBlur(vec2 texCoord) {
  vec2 texelRadius = (vec2(bloom.radius) / vec2(textureSize(bloomTexture, 0)));
  vec2 tint_symbol = (bloomDir * texelRadius);
  vec4 sum = vec4(0.0f);
  sum = (sum + (texture(bloomTexture, texCoord) * weights[0]));
  sum = (sum + (texture(bloomTexture, (texCoord + (tint_symbol * 1.0f))) * weights[1]));
  sum = (sum + (texture(bloomTexture, (texCoord - (tint_symbol * 1.0f))) * weights[1]));
  sum = (sum + (texture(bloomTexture, (texCoord + (tint_symbol * 2.0f))) * weights[2]));
  sum = (sum + (texture(bloomTexture, (texCoord - (tint_symbol * 2.0f))) * weights[2]));
  return vec4(sum.rgb, 1.0f);
}

uniform highp sampler2D prevTexture;

struct tint_symbol_3 {
  vec2 texCoord;
};
struct tint_symbol_4 {
  vec4 value;
};

vec4 fragmentMain_inner(FragmentInput tint_symbol_1) {
  vec4 blurColor = getGaussianBlur(tint_symbol_1.texCoord);
  vec4 dimColor = (texture(prevTexture, tint_symbol_1.texCoord) * bloom.dim);
  return (blurColor + dimColor);
}

tint_symbol_4 fragmentMain(tint_symbol_3 tint_symbol_2) {
  FragmentInput tint_symbol_5 = FragmentInput(tint_symbol_2.texCoord);
  vec4 inner_result = fragmentMain_inner(tint_symbol_5);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
in vec2 texCoord;
out vec4 value;
void main() {
  tint_symbol_3 inputs;
  inputs.texCoord = texCoord;
  tint_symbol_4 outputs;
  outputs = fragmentMain(inputs);
  value = outputs.value;
}


