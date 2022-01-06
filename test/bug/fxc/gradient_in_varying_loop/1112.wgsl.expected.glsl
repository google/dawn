#version 310 es
precision mediump float;


uniform highp sampler2D randomTexture;
uniform highp sampler2D depthTexture;

struct tint_symbol_2 {
  vec2 vUV;
};
struct tint_symbol_3 {
  vec4 value;
};

vec4 tint_symbol_inner(vec2 vUV) {
  vec3 random = texture(randomTexture, vUV).rgb;
  int i = 0;
  while (true) {
    if ((i < 1)) {
    } else {
      break;
    }
    vec3 offset = vec3(random.x);
    bool tint_tmp_2 = (offset.x < 0.0f);
    if (!tint_tmp_2) {
      tint_tmp_2 = (offset.y < 0.0f);
    }
    bool tint_tmp_1 = (tint_tmp_2);
    if (!tint_tmp_1) {
      tint_tmp_1 = (offset.x > 1.0f);
    }
    bool tint_tmp = (tint_tmp_1);
    if (!tint_tmp) {
      tint_tmp = (offset.y > 1.0f);
    }
    if ((tint_tmp)) {
      i = (i + 1);
      continue;
    }
    float sampleDepth = texture(depthTexture, offset.xy).r;
    i = (i + 1);
  }
  return vec4(1.0f);
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  vec4 inner_result = tint_symbol_inner(tint_symbol_1.vUV);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
in vec2 vUV;
out vec4 value;
void main() {
  tint_symbol_2 inputs;
  inputs.vUV = vUV;
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  value = outputs.value;
}


