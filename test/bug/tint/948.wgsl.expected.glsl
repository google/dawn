SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 9) uniform LeftOver_1 {
  float time;
  uint padding;
  mat4 worldViewProjection;
  vec2 outputSize;
  vec2 stageSize;
  vec2 spriteMapSize;
  float stageScale;
  float spriteCount;
  vec3 colorMul;
} x_20;
uniform highp sampler2D frameMapTexture;

vec2 tUV = vec2(0.0f, 0.0f);
uniform highp sampler2D tileMapsTexture0;

uniform highp sampler2D tileMapsTexture1;
uniform highp sampler2D animationMapTexture;

float mt = 0.0f;
uniform highp sampler2D spriteSheetTexture;

vec4 glFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec2 tileID_1 = vec2(0.0f, 0.0f);
vec2 levelUnits = vec2(0.0f, 0.0f);
vec2 stageUnits_1 = vec2(0.0f, 0.0f);
vec3 vPosition = vec3(0.0f, 0.0f, 0.0f);
vec2 vUV = vec2(0.0f, 0.0f);

mat4 getFrameData_f1_(inout float frameID) {
  float fX = 0.0f;
  float x_15 = frameID;
  float x_25 = x_20.spriteCount;
  fX = (x_15 / x_25);
  vec4 x_40 = texture(frameMapTexture, vec2(fX, 0.0f), 0.0f);
  vec4 x_47 = texture(frameMapTexture, vec2(fX, 0.25f), 0.0f);
  vec4 x_54 = texture(frameMapTexture, vec2(fX, 0.5f), 0.0f);
  return mat4(vec4(x_40.x, x_40.y, x_40.z, x_40.w), vec4(x_47.x, x_47.y, x_47.z, x_47.w), vec4(x_54.x, x_54.y, x_54.z, x_54.w), vec4(vec4(0.0f, 0.0f, 0.0f, 0.0f).x, vec4(0.0f, 0.0f, 0.0f, 0.0f).y, vec4(0.0f, 0.0f, 0.0f, 0.0f).z, vec4(0.0f, 0.0f, 0.0f, 0.0f).w));
}

void main_1() {
  vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec2 tileUV = vec2(0.0f, 0.0f);
  vec2 tileID = vec2(0.0f, 0.0f);
  vec2 sheetUnits = vec2(0.0f, 0.0f);
  float spriteUnits = 0.0f;
  vec2 stageUnits = vec2(0.0f, 0.0f);
  int i = 0;
  float frameID_1 = 0.0f;
  vec4 animationData = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float f = 0.0f;
  mat4 frameData = mat4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float param = 0.0f;
  vec2 frameSize = vec2(0.0f, 0.0f);
  vec2 offset_1 = vec2(0.0f, 0.0f);
  vec2 ratio = vec2(0.0f, 0.0f);
  vec4 nc = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float alpha = 0.0f;
  vec3 mixed = vec3(0.0f, 0.0f, 0.0f);
  color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  tileUV = frac(tUV);
  float x_91 = tileUV.y;
  tileUV.y = (1.0f - x_91);
  tileID = floor(tUV);
  vec2 x_101 = x_20.spriteMapSize;
  sheetUnits = (vec2(1.0f, 1.0f) / x_101);
  float x_106 = x_20.spriteCount;
  spriteUnits = (1.0f / x_106);
  vec2 x_111 = x_20.stageSize;
  stageUnits = (vec2(1.0f, 1.0f) / x_111);
  i = 0;
  {
    for(; (i < 2); i = (i + 1)) {
      switch(i) {
        case 1: {
          vec2 x_150 = tileID;
          vec2 x_154 = x_20.stageSize;
          vec4 x_156 = texture(tileMapsTexture1, ((x_150 + vec2(0.5f, 0.5f)) / x_154), 0.0f);
          frameID_1 = x_156.x;
          break;
        }
        case 0: {
          vec2 x_136 = tileID;
          vec2 x_140 = x_20.stageSize;
          vec4 x_142 = texture(tileMapsTexture0, ((x_136 + vec2(0.5f, 0.5f)) / x_140), 0.0f);
          frameID_1 = x_142.x;
          break;
        }
        default: {
          break;
        }
      }
      float x_166 = frameID_1;
      float x_169 = x_20.spriteCount;
      vec4 x_172 = texture(animationMapTexture, vec2(((x_166 + 0.5f) / x_169), 0.0f), 0.0f);
      animationData = x_172;
      float x_174 = animationData.y;
      if ((x_174 > 0.0f)) {
        float x_181 = x_20.time;
        float x_184 = animationData.z;
        mt = ((x_181 * x_184) % 1.0f);
        f = 0.0f;
        {
          for(; (f < 8.0f); f = (f + 1.0f)) {
            float x_197 = animationData.y;
            if ((x_197 > mt)) {
              float x_203 = animationData.x;
              frameID_1 = x_203;
              break;
            }
            float x_208 = frameID_1;
            float x_211 = x_20.spriteCount;
            vec4 x_217 = texture(animationMapTexture, vec2(((x_208 + 0.5f) / x_211), (0.125f * f)), 0.0f);
            animationData = x_217;
          }
        }
      }
      param = (frameID_1 + 0.5f);
      mat4 x_225 = getFrameData_f1_(param);
      frameData = x_225;
      vec4 x_228 = frameData[0];
      vec2 x_231 = x_20.spriteMapSize;
      frameSize = (vec2(x_228.w, x_228.z) / x_231);
      vec4 x_235 = frameData[0];
      offset_1 = (vec2(x_235.x, x_235.y) * sheetUnits);
      vec4 x_241 = frameData[2];
      vec4 x_244 = frameData[0];
      ratio = (vec2(x_241.x, x_241.y) / vec2(x_244.w, x_244.z));
      float x_248 = frameData[2].z;
      if ((x_248 == 1.0f)) {
        vec2 x_252 = tileUV;
        tileUV = vec2(x_252.y, x_252.x);
      }
      if ((i == 0)) {
        vec4 x_268 = texture(spriteSheetTexture, ((tileUV * frameSize) + offset_1));
        color = x_268;
      } else {
        vec4 x_279 = texture(spriteSheetTexture, ((tileUV * frameSize) + offset_1));
        nc = x_279;
        float x_283 = color.w;
        float x_285 = nc.w;
        alpha = min((x_283 + x_285), 1.0f);
        vec4 x_290 = color;
        vec4 x_292 = nc;
        float x_295 = nc.w;
        mixed = mix(vec3(x_290.x, x_290.y, x_290.z), vec3(x_292.x, x_292.y, x_292.z), vec3(x_295, x_295, x_295));
        vec3 x_298 = mixed;
        color = vec4(x_298.x, x_298.y, x_298.z, alpha);
      }
    }
  }
  vec3 x_310 = x_20.colorMul;
  vec4 x_311 = color;
  vec3 x_313 = (vec3(x_311.x, x_311.y, x_311.z) * x_310);
  color = vec4(x_313.x, x_313.y, x_313.z, color.w);
  glFragColor = color;
  return;
}

struct main_out {
  vec4 glFragColor_1;
};
struct tint_symbol_2 {
  vec3 vPosition_param;
  vec2 vUV_param;
  vec2 tUV_param;
  vec2 stageUnits_1_param;
  vec2 levelUnits_param;
  vec2 tileID_1_param;
};
struct tint_symbol_3 {
  vec4 glFragColor_1;
};

main_out tint_symbol_inner(vec2 tUV_param, vec2 tileID_1_param, vec2 levelUnits_param, vec2 stageUnits_1_param, vec3 vPosition_param, vec2 vUV_param) {
  tUV = tUV_param;
  tileID_1 = tileID_1_param;
  levelUnits = levelUnits_param;
  stageUnits_1 = stageUnits_1_param;
  vPosition = vPosition_param;
  vUV = vUV_param;
  main_1();
  main_out tint_symbol_4 = main_out(glFragColor);
  return tint_symbol_4;
}

tint_symbol_3 tint_symbol(tint_symbol_2 tint_symbol_1) {
  main_out inner_result = tint_symbol_inner(tint_symbol_1.tUV_param, tint_symbol_1.tileID_1_param, tint_symbol_1.levelUnits_param, tint_symbol_1.stageUnits_1_param, tint_symbol_1.vPosition_param, tint_symbol_1.vUV_param);
  tint_symbol_3 wrapper_result = tint_symbol_3(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.glFragColor_1 = inner_result.glFragColor_1;
  return wrapper_result;
}
in vec3 vPosition_param;
in vec2 vUV_param;
in vec2 tUV_param;
in vec2 stageUnits_1_param;
in vec2 levelUnits_param;
in vec2 tileID_1_param;
out vec4 glFragColor_1;
void main() {
  tint_symbol_2 inputs;
  inputs.vPosition_param = vPosition_param;
  inputs.vUV_param = vUV_param;
  inputs.tUV_param = tUV_param;
  inputs.stageUnits_1_param = stageUnits_1_param;
  inputs.levelUnits_param = levelUnits_param;
  inputs.tileID_1_param = tileID_1_param;
  tint_symbol_3 outputs;
  outputs = tint_symbol(inputs);
  glFragColor_1 = outputs.glFragColor_1;
}


Error parsing GLSL shader:
ERROR: 0:65: 'frac' : no matching overloaded function found 
ERROR: 0:65: 'assign' :  cannot convert from ' const float' to ' temp mediump 2-component vector of float'
ERROR: 0:65: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



