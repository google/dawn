SKIP: FAILED

#version 310 es
precision mediump float;

layout(location = 0) in vec3 shadowPos_1;
layout(location = 1) in vec3 fragPos_1;
layout(location = 2) in vec3 fragNorm_1;
layout(location = 0) out vec4 value;
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
uniform highp sampler2D shadowMap_shadowSampler;

vec4 tint_symbol(FragmentInput tint_symbol_1) {
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

void main() {
  FragmentInput tint_symbol_2 = FragmentInput(shadowPos_1, fragPos_1, fragNorm_1);
  vec4 inner_result = tint_symbol(tint_symbol_2);
  value = inner_result;
  return;
}
Error parsing GLSL shader:
ERROR: 0:39: 'assign' :  cannot convert from ' temp highp 4-component vector of float' to ' temp mediump float'
ERROR: 0:39: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



