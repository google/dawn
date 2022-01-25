SKIP: FAILED

#version 310 es
precision mediump float;

const float shadowDepthTextureSize = 1024.0f;

struct Scene {
  mat4 lightViewProjMatrix;
  mat4 cameraViewProjMatrix;
  vec3 lightPos;
};

layout(binding = 0) uniform Scene_1 {
  mat4 lightViewProjMatrix;
  mat4 cameraViewProjMatrix;
  vec3 lightPos;
} scene;

struct FragmentInput {
  vec3 shadowPos;
  vec3 fragPos;
  vec3 fragNorm;
};

const vec3 albedo = vec3(0.899999976f, 0.899999976f, 0.899999976f);
const float ambientFactor = 0.200000003f;

struct tint_symbol_3 {
  vec3 shadowPos;
  vec3 fragPos;
  vec3 fragNorm;
};
struct tint_symbol_4 {
  vec4 value;
};

uniform highp sampler2D shadowMap_shadowSampler;


vec4 tint_symbol_inner(FragmentInput tint_symbol_1) {
  float visibility = 0.0f;
  float oneOverShadowDepthTextureSize = (1.0f / shadowDepthTextureSize);
  {
    for(int y = -1; (y <= 1); y = (y + 1)) {
      {
        for(int x = -1; (x <= 1); x = (x + 1)) {
          vec2 offset = vec2((float(x) * oneOverShadowDepthTextureSize), (float(y) * oneOverShadowDepthTextureSize));
          visibility = (visibility + texture(shadowMap_shadowSampler, (tint_symbol_1.shadowPos.xy + offset), (tint_symbol_1.shadowPos.z - 0.007f)));
        }
      }
    }
  }
  visibility = (visibility / 9.0f);
  float lambertFactor = max(dot(normalize((scene.lightPos - tint_symbol_1.fragPos)), tint_symbol_1.fragNorm), 0.0f);
  float lightingFactor = min((ambientFactor + (visibility * lambertFactor)), 1.0f);
  return vec4((lightingFactor * albedo), 1.0f);
}

tint_symbol_4 tint_symbol(tint_symbol_3 tint_symbol_2) {
  FragmentInput tint_symbol_5 = FragmentInput(tint_symbol_2.shadowPos, tint_symbol_2.fragPos, tint_symbol_2.fragNorm);
  vec4 inner_result = tint_symbol_inner(tint_symbol_5);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.value = inner_result;
  return wrapper_result;
}
layout(location = 0) in vec3 shadowPos;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 fragNorm;
layout(location = 0) out vec4 value;
void main() {
  tint_symbol_3 inputs;
  inputs.shadowPos = shadowPos;
  inputs.fragPos = fragPos;
  inputs.fragNorm = fragNorm;
  tint_symbol_4 outputs;
  outputs = tint_symbol(inputs);
  value = outputs.value;
}


Error parsing GLSL shader:
ERROR: 0:47: 'assign' :  cannot convert from ' temp highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:47: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



