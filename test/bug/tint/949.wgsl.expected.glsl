SKIP: FAILED

#version 310 es
precision mediump float;

struct lightingInfo {
  vec3 diffuse;
  vec3 specular;
};

float u_Float = 0.0f;
vec3 u_Color = vec3(0.0f, 0.0f, 0.0f);
uniform highp sampler2D TextureSamplerTexture;

vec2 vMainuv = vec2(0.0f, 0.0f);
layout (binding = 6) uniform LeftOver_1 {
  mat4 u_World;
  mat4 u_ViewProjection;
  float u_bumpStrength;
  uint padding;
  vec3 u_cameraPosition;
  float u_parallaxScale;
  float textureInfoName;
  uint padding_1;
  vec2 tangentSpaceParameter0;
} x_269;
vec4 v_output1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
bool tint_symbol = false;
vec2 v_uv = vec2(0.0f, 0.0f);
vec4 v_output2 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
uniform highp sampler2D TextureSampler1Texture;

layout (binding = 5) uniform Light0_1 {
  vec4 vLightData;
  vec4 vLightDiffuse;
  vec4 vLightSpecular;
  vec3 vLightGround;
  uint padding_2;
  vec4 shadowsInfo;
  vec2 depthValues;
} light0;
vec4 glFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

mat3 cotangent_frame_vf3_vf3_vf2_vf2_(inout vec3 normal_1, inout vec3 p, inout vec2 uv, inout vec2 tangentSpaceParams) {
  vec3 dp1 = vec3(0.0f, 0.0f, 0.0f);
  vec3 dp2 = vec3(0.0f, 0.0f, 0.0f);
  vec2 duv1 = vec2(0.0f, 0.0f);
  vec2 duv2 = vec2(0.0f, 0.0f);
  vec3 dp2perp = vec3(0.0f, 0.0f, 0.0f);
  vec3 dp1perp = vec3(0.0f, 0.0f, 0.0f);
  vec3 tangent = vec3(0.0f, 0.0f, 0.0f);
  vec3 bitangent = vec3(0.0f, 0.0f, 0.0f);
  float invmax = 0.0f;
  vec3 x_133 = p;
  dp1 = ddx(x_133);
  vec3 x_136 = p;
  dp2 = ddy(x_136);
  vec2 x_139 = uv;
  duv1 = ddx(x_139);
  vec2 x_142 = uv;
  duv2 = ddy(x_142);
  vec3 x_145 = dp2;
  vec3 x_146 = normal_1;
  dp2perp = cross(x_145, x_146);
  vec3 x_149 = normal_1;
  dp1perp = cross(x_149, dp1);
  vec3 x_153 = dp2perp;
  float x_155 = duv1.x;
  vec3 x_157 = dp1perp;
  float x_159 = duv2.x;
  tangent = ((x_153 * x_155) + (x_157 * x_159));
  vec3 x_163 = dp2perp;
  float x_165 = duv1.y;
  vec3 x_167 = dp1perp;
  float x_169 = duv2.y;
  bitangent = ((x_163 * x_165) + (x_167 * x_169));
  float x_173 = tangentSpaceParams.x;
  tangent = (tangent * x_173);
  float x_177 = tangentSpaceParams.y;
  bitangent = (bitangent * x_177);
  invmax = rsqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
  vec3 x_191 = (tangent * invmax);
  vec3 x_194 = (bitangent * invmax);
  vec3 x_195 = normal_1;
  return mat3(vec3(x_191.x, x_191.y, x_191.z), vec3(x_194.x, x_194.y, x_194.z), vec3(x_195.x, x_195.y, x_195.z));
}

mat3 transposeMat3_mf33_(inout mat3 inMatrix) {
  vec3 i0 = vec3(0.0f, 0.0f, 0.0f);
  vec3 i1 = vec3(0.0f, 0.0f, 0.0f);
  vec3 i2 = vec3(0.0f, 0.0f, 0.0f);
  mat3 outMatrix = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 x_60 = inMatrix[0];
  i0 = x_60;
  vec3 x_64 = inMatrix[1];
  i1 = x_64;
  vec3 x_68 = inMatrix[2];
  i2 = x_68;
  float x_73 = i0.x;
  float x_75 = i1.x;
  float x_77 = i2.x;
  vec3 x_78 = vec3(x_73, x_75, x_77);
  float x_81 = i0.y;
  float x_83 = i1.y;
  float x_85 = i2.y;
  vec3 x_86 = vec3(x_81, x_83, x_85);
  float x_89 = i0.z;
  float x_91 = i1.z;
  float x_93 = i2.z;
  vec3 x_94 = vec3(x_89, x_91, x_93);
  outMatrix = mat3(vec3(x_78.x, x_78.y, x_78.z), vec3(x_86.x, x_86.y, x_86.z), vec3(x_94.x, x_94.y, x_94.z));
  return outMatrix;
}

vec3 perturbNormalBase_mf33_vf3_f1_(inout mat3 cotangentFrame, inout vec3 normal, inout float scale) {
  mat3 x_113 = cotangentFrame;
  vec3 x_114 = normal;
  return normalize((x_113 * x_114));
}

vec3 perturbNormal_mf33_vf3_f1_(inout mat3 cotangentFrame_1, inout vec3 textureSample, inout float scale_1) {
  mat3 param = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 param_1 = vec3(0.0f, 0.0f, 0.0f);
  float param_2 = 0.0f;
  vec3 x_119 = textureSample;
  mat3 x_125 = cotangentFrame_1;
  param = x_125;
  param_1 = ((x_119 * 2.0f) - vec3(1.0f, 1.0f, 1.0f));
  float x_128 = scale_1;
  param_2 = x_128;
  vec3 x_129 = perturbNormalBase_mf33_vf3_f1_(param, param_1, param_2);
  return x_129;
}

lightingInfo computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(inout vec3 viewDirectionW, inout vec3 vNormal, inout vec4 lightData, inout vec3 diffuseColor, inout vec3 specularColor, inout vec3 groundColor, inout float glossiness) {
  float ndl = 0.0f;
  lightingInfo result = lightingInfo(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  vec3 angleW = vec3(0.0f, 0.0f, 0.0f);
  float specComp = 0.0f;
  vec3 x_212 = vNormal;
  vec4 x_213 = lightData;
  ndl = ((dot(x_212, vec3(x_213.x, x_213.y, x_213.z)) * 0.5f) + 0.5f);
  vec3 x_220 = groundColor;
  vec3 x_221 = diffuseColor;
  float x_222 = ndl;
  result.diffuse = mix(x_220, x_221, vec3(x_222, x_222, x_222));
  vec3 x_227 = viewDirectionW;
  vec4 x_228 = lightData;
  angleW = normalize((x_227 + vec3(x_228.x, x_228.y, x_228.z)));
  vec3 x_233 = vNormal;
  specComp = max(0.0f, dot(x_233, angleW));
  float x_237 = specComp;
  float x_238 = glossiness;
  specComp = pow(x_237, max(1.0f, x_238));
  float x_241 = specComp;
  vec3 x_242 = specularColor;
  result.specular = (x_242 * x_241);
  return result;
}

void main_1() {
  vec4 tempTextureRead = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec3 rgb = vec3(0.0f, 0.0f, 0.0f);
  vec3 output5 = vec3(0.0f, 0.0f, 0.0f);
  vec4 output4 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec2 uvOffset = vec2(0.0f, 0.0f);
  float normalScale = 0.0f;
  vec2 TBNUV = vec2(0.0f, 0.0f);
  vec2 x_299 = vec2(0.0f, 0.0f);
  mat3 TBN = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 param_3 = vec3(0.0f, 0.0f, 0.0f);
  vec3 param_4 = vec3(0.0f, 0.0f, 0.0f);
  vec2 param_5 = vec2(0.0f, 0.0f);
  vec2 param_6 = vec2(0.0f, 0.0f);
  mat3 invTBN = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  mat3 param_7 = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float parallaxLimit = 0.0f;
  vec2 vOffsetDir = vec2(0.0f, 0.0f);
  vec2 vMaxOffset = vec2(0.0f, 0.0f);
  float numSamples = 0.0f;
  float stepSize = 0.0f;
  float currRayHeight = 0.0f;
  vec2 vCurrOffset = vec2(0.0f, 0.0f);
  vec2 vLastOffset = vec2(0.0f, 0.0f);
  float lastSampledHeight = 0.0f;
  float currSampledHeight = 0.0f;
  int i = 0;
  float delta1 = 0.0f;
  float delta2 = 0.0f;
  float ratio = 0.0f;
  vec2 parallaxOcclusion_0 = vec2(0.0f, 0.0f);
  mat3 param_8 = mat3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vec3 param_9 = vec3(0.0f, 0.0f, 0.0f);
  float param_10 = 0.0f;
  vec2 output6 = vec2(0.0f, 0.0f);
  vec4 tempTextureRead1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec3 rgb1 = vec3(0.0f, 0.0f, 0.0f);
  vec3 viewDirectionW_1 = vec3(0.0f, 0.0f, 0.0f);
  float shadow = 0.0f;
  float glossiness_1 = 0.0f;
  vec3 diffuseBase = vec3(0.0f, 0.0f, 0.0f);
  vec3 specularBase = vec3(0.0f, 0.0f, 0.0f);
  vec3 normalW = vec3(0.0f, 0.0f, 0.0f);
  lightingInfo info = lightingInfo(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
  vec3 param_11 = vec3(0.0f, 0.0f, 0.0f);
  vec3 param_12 = vec3(0.0f, 0.0f, 0.0f);
  vec4 param_13 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec3 param_14 = vec3(0.0f, 0.0f, 0.0f);
  vec3 param_15 = vec3(0.0f, 0.0f, 0.0f);
  vec3 param_16 = vec3(0.0f, 0.0f, 0.0f);
  float param_17 = 0.0f;
  vec3 diffuseOutput = vec3(0.0f, 0.0f, 0.0f);
  vec3 specularOutput = vec3(0.0f, 0.0f, 0.0f);
  vec3 output3 = vec3(0.0f, 0.0f, 0.0f);
  u_Float = 100.0f;
  u_Color = vec3(0.5f, 0.5f, 0.5f);
  vec4 x_262 = texture(TextureSamplerTexture, vMainuv);
  tempTextureRead = x_262;
  vec4 x_264 = tempTextureRead;
  float x_273 = x_269.textureInfoName;
  rgb = (vec3(x_264.x, x_264.y, x_264.z) * x_273);
  vec3 x_279 = x_269.u_cameraPosition;
  vec4 x_282 = v_output1;
  output5 = normalize((x_279 - vec3(x_282.x, x_282.y, x_282.z)));
  output4 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  uvOffset = vec2(0.0f, 0.0f);
  float x_292 = x_269.u_bumpStrength;
  normalScale = (1.0f / x_292);
  if (tint_symbol) {
    x_299 = v_uv;
  } else {
    x_299 = -(v_uv);
  }
  TBNUV = x_299;
  vec4 x_310 = v_output2;
  param_3 = (vec3(x_310.x, x_310.y, x_310.z) * normalScale);
  vec4 x_317 = v_output1;
  param_4 = vec3(x_317.x, x_317.y, x_317.z);
  param_5 = TBNUV;
  vec2 x_324 = x_269.tangentSpaceParameter0;
  param_6 = x_324;
  mat3 x_325 = cotangent_frame_vf3_vf3_vf2_vf2_(param_3, param_4, param_5, param_6);
  TBN = x_325;
  param_7 = TBN;
  mat3 x_329 = transposeMat3_mf33_(param_7);
  invTBN = x_329;
  vec3 x_334 = (invTBN * -(output5));
  parallaxLimit = (length(vec2(x_334.x, x_334.y)) / (invTBN * -(output5)).z);
  float x_345 = x_269.u_parallaxScale;
  parallaxLimit = (parallaxLimit * x_345);
  vec3 x_352 = (invTBN * -(output5));
  vOffsetDir = normalize(vec2(x_352.x, x_352.y));
  vMaxOffset = (vOffsetDir * parallaxLimit);
  vec4 x_366 = v_output2;
  numSamples = (15.0f + (dot((invTBN * -(output5)), (invTBN * vec3(x_366.x, x_366.y, x_366.z))) * -11.0f));
  stepSize = (1.0f / numSamples);
  currRayHeight = 1.0f;
  vCurrOffset = vec2(0.0f, 0.0f);
  vLastOffset = vec2(0.0f, 0.0f);
  lastSampledHeight = 1.0f;
  currSampledHeight = 1.0f;
  i = 0;
  {
    for(; (i < 15); i = (i + 1)) {
      vec4 x_397 = texture(TextureSamplerTexture, (v_uv + vCurrOffset));
      currSampledHeight = x_397.w;
      if ((currSampledHeight > currRayHeight)) {
        delta1 = (currSampledHeight - currRayHeight);
        delta2 = ((currRayHeight + stepSize) - lastSampledHeight);
        ratio = (delta1 / (delta1 + delta2));
        vCurrOffset = ((vLastOffset * ratio) + (vCurrOffset * (1.0f - ratio)));
        break;
      } else {
        currRayHeight = (currRayHeight - stepSize);
        vLastOffset = vCurrOffset;
        vCurrOffset = (vCurrOffset + (vMaxOffset * stepSize));
        lastSampledHeight = currSampledHeight;
      }
    }
  }
  parallaxOcclusion_0 = vCurrOffset;
  uvOffset = parallaxOcclusion_0;
  vec4 x_452 = texture(TextureSamplerTexture, (v_uv + uvOffset));
  float x_454 = x_269.u_bumpStrength;
  param_8 = TBN;
  param_9 = vec3(x_452.x, x_452.y, x_452.z);
  param_10 = (1.0f / x_454);
  vec3 x_461 = perturbNormal_mf33_vf3_f1_(param_8, param_9, param_10);
  output4 = vec4(x_461.x, x_461.y, x_461.z, output4.w);
  output6 = (v_uv + uvOffset);
  vec4 x_475 = texture(TextureSampler1Texture, output6);
  tempTextureRead1 = x_475;
  vec4 x_477 = tempTextureRead1;
  rgb1 = vec3(x_477.x, x_477.y, x_477.z);
  vec3 x_481 = x_269.u_cameraPosition;
  vec4 x_482 = v_output1;
  viewDirectionW_1 = normalize((x_481 - vec3(x_482.x, x_482.y, x_482.z)));
  shadow = 1.0f;
  glossiness_1 = (1.0f * u_Float);
  diffuseBase = vec3(0.0f, 0.0f, 0.0f);
  specularBase = vec3(0.0f, 0.0f, 0.0f);
  vec4 x_494 = output4;
  normalW = vec3(x_494.x, x_494.y, x_494.z);
  param_11 = viewDirectionW_1;
  param_12 = normalW;
  vec4 x_507 = light0.vLightData;
  param_13 = x_507;
  vec4 x_510 = light0.vLightDiffuse;
  param_14 = vec3(x_510.x, x_510.y, x_510.z);
  vec4 x_514 = light0.vLightSpecular;
  param_15 = vec3(x_514.x, x_514.y, x_514.z);
  vec3 x_518 = light0.vLightGround;
  param_16 = x_518;
  param_17 = glossiness_1;
  lightingInfo x_521 = computeHemisphericLighting_vf3_vf3_vf4_vf3_vf3_vf3_f1_(param_11, param_12, param_13, param_14, param_15, param_16, param_17);
  info = x_521;
  shadow = 1.0f;
  vec3 x_523 = info.diffuse;
  diffuseBase = (diffuseBase + (x_523 * shadow));
  vec3 x_529 = info.specular;
  specularBase = (specularBase + (x_529 * shadow));
  diffuseOutput = (diffuseBase * rgb1);
  specularOutput = (specularBase * u_Color);
  output3 = (diffuseOutput + specularOutput);
  vec3 x_548 = output3;
  glFragColor = vec4(x_548.x, x_548.y, x_548.z, 1.0f);
  return;
}

struct main_out {
  vec4 glFragColor_1;
};
struct tint_symbol_4 {
  vec4 v_output1_param;
  vec2 vMainuv_param;
  vec4 v_output2_param;
  vec2 v_uv_param;
  bool tint_symbol_2;
};
struct tint_symbol_5 {
  vec4 glFragColor_1;
};

main_out tint_symbol_1_inner(vec2 vMainuv_param, vec4 v_output1_param, bool tint_symbol_2, vec2 v_uv_param, vec4 v_output2_param) {
  vMainuv = vMainuv_param;
  v_output1 = v_output1_param;
  tint_symbol = tint_symbol_2;
  v_uv = v_uv_param;
  v_output2 = v_output2_param;
  main_1();
  main_out tint_symbol_6 = main_out(glFragColor);
  return tint_symbol_6;
}

tint_symbol_5 tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  main_out inner_result = tint_symbol_1_inner(tint_symbol_3.vMainuv_param, tint_symbol_3.v_output1_param, tint_symbol_3.tint_symbol_2, tint_symbol_3.v_uv_param, tint_symbol_3.v_output2_param);
  tint_symbol_5 wrapper_result = tint_symbol_5(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.glFragColor_1 = inner_result.glFragColor_1;
  return wrapper_result;
}
in vec4 v_output1_param;
in vec2 vMainuv_param;
in vec4 v_output2_param;
in vec2 v_uv_param;
out vec4 glFragColor_1;
void main() {
  tint_symbol_4 inputs;
  inputs.v_output1_param = v_output1_param;
  inputs.vMainuv_param = vMainuv_param;
  inputs.v_output2_param = v_output2_param;
  inputs.v_uv_param = v_uv_param;
  inputs.tint_symbol_2 = gl_FrontFacing;
  tint_symbol_5 outputs;
  outputs = tint_symbol_1(inputs);
  glFragColor_1 = outputs.glFragColor_1;
}


Error parsing GLSL shader:
ERROR: 0:53: 'ddx' : no matching overloaded function found 
ERROR: 0:53: 'assign' :  cannot convert from ' const float' to ' temp mediump 3-component vector of float'
ERROR: 0:53: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



