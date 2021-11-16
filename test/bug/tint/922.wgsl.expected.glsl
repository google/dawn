SKIP: FAILED

#version 310 es
precision mediump float;

struct Mat4x4_ {
  vec4 mx;
  vec4 my;
  vec4 mz;
  vec4 mw;
};
struct Mat4x3_ {
  vec4 mx;
  vec4 my;
  vec4 mz;
};
struct Mat4x2_ {
  vec4 mx;
  vec4 my;
};
struct VertexOutput {
  vec4 v_Color;
  vec2 v_TexCoord;
  vec4 member;
};

layout (binding = 0) uniform ub_SceneParams_1 {
  Mat4x4_ u_Projection;
} global;
layout (binding = 1) uniform ub_MaterialParams_1 {
  Mat4x2_ u_TexMtx[1];
  vec4 u_Misc0_;
} global1;
layout (binding = 2) uniform ub_PacketParams_1 {
  Mat4x3_ u_PosMtx[32];
} global2;
vec3 a_Position1 = vec3(0.0f, 0.0f, 0.0f);
vec2 a_UV1 = vec2(0.0f, 0.0f);
vec4 a_Color1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec3 a_Normal1 = vec3(0.0f, 0.0f, 0.0f);
float a_PosMtxIdx1 = 0.0f;
vec4 v_Color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
vec2 v_TexCoord = vec2(0.0f, 0.0f);
vec4 tint_symbol = vec4(0.0f, 0.0f, 0.0f, 0.0f);

vec4 Mul(Mat4x4_ m8, vec4 v) {
  Mat4x4_ m9 = Mat4x4_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  vec4 v1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  m9 = m8;
  v1 = v;
  return vec4(dot(m9.mx, v1), dot(m9.my, v1), dot(m9.mz, v1), dot(m9.mw, v1));
}

vec2 Mul2(Mat4x2_ m12, vec4 v4) {
  Mat4x2_ m13 = Mat4x2_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  vec4 v5 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  m13 = m12;
  v5 = v4;
  return vec2(dot(m13.mx, v5), dot(m13.my, v5));
}

Mat4x4_ x_Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = Mat4x4_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  n1 = n;
  o.mx = vec4(n1, 0.0f, 0.0f, 0.0f);
  o.my = vec4(0.0f, n1, 0.0f, 0.0f);
  o.mz = vec4(0.0f, 0.0f, n1, 0.0f);
  o.mw = vec4(0.0f, 0.0f, 0.0f, n1);
  return o;
}

Mat4x4_ x_Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = Mat4x3_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  Mat4x4_ o1 = Mat4x4_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  m17 = m16;
  Mat4x4_ x_e4 = x_Mat4x4_(1.0f);
  o1 = x_e4;
  o1.mx = m17.mx;
  o1.my = m17.my;
  o1.mz = m17.mz;
  return o1;
}

void main1() {
  Mat4x3_ t_PosMtx = Mat4x3_(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  vec2 t_TexSpaceCoord = vec2(0.0f, 0.0f);
  Mat4x3_ x_e18 = global2.u_PosMtx[int(a_PosMtxIdx1)];
  t_PosMtx = x_e18;
  Mat4x4_ x_e24 = x_Mat4x4_1(t_PosMtx);
  vec3 x_e25 = a_Position1;
  Mat4x4_ x_e30 = x_Mat4x4_1(t_PosMtx);
  vec4 x_e34 = Mul(x_e30, vec4(a_Position1, 1.0f));
  Mat4x4_ x_e35 = global.u_Projection;
  Mat4x4_ x_e38 = x_Mat4x4_1(t_PosMtx);
  vec3 x_e39 = a_Position1;
  Mat4x4_ x_e44 = x_Mat4x4_1(t_PosMtx);
  vec4 x_e48 = Mul(x_e44, vec4(a_Position1, 1.0f));
  vec4 x_e49 = Mul(x_e35, x_e48);
  tint_symbol = x_e49;
  v_Color = a_Color1;
  vec4 x_e52 = global1.u_Misc0_;
  if ((x_e52.x == 2.0f)) {
    {
      vec3 x_e59 = a_Normal1;
      Mat4x2_ x_e64 = global1.u_TexMtx[0];
      vec2 x_e68 = Mul2(x_e64, vec4(a_Normal1, 1.0f));
      v_TexCoord = x_e68.xy;
      return;
    }
  } else {
    {
      vec2 x_e73 = a_UV1;
      Mat4x2_ x_e79 = global1.u_TexMtx[0];
      vec2 x_e84 = Mul2(x_e79, vec4(a_UV1, 1.0f, 1.0f));
      v_TexCoord = x_e84.xy;
      return;
    }
  }
}

struct tint_symbol_3 {
  vec3 a_Position;
  vec2 a_UV;
  vec4 a_Color;
  vec3 a_Normal;
  float a_PosMtxIdx;
};
struct tint_symbol_4 {
  vec4 v_Color;
  vec2 v_TexCoord;
  vec4 member;
};

VertexOutput tint_symbol_1_inner(vec3 a_Position, vec2 a_UV, vec4 a_Color, vec3 a_Normal, float a_PosMtxIdx) {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  VertexOutput tint_symbol_5 = VertexOutput(v_Color, v_TexCoord, tint_symbol);
  return tint_symbol_5;
}

tint_symbol_4 tint_symbol_1(tint_symbol_3 tint_symbol_2) {
  VertexOutput inner_result = tint_symbol_1_inner(tint_symbol_2.a_Position, tint_symbol_2.a_UV, tint_symbol_2.a_Color, tint_symbol_2.a_Normal, tint_symbol_2.a_PosMtxIdx);
  tint_symbol_4 wrapper_result = tint_symbol_4(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.v_Color = inner_result.v_Color;
  wrapper_result.v_TexCoord = inner_result.v_TexCoord;
  wrapper_result.member = inner_result.member;
  return wrapper_result;
}
in vec3 a_Position;
in vec2 a_UV;
in vec4 a_Color;
in vec3 a_Normal;
in float a_PosMtxIdx;
out vec4 v_Color;
out vec2 v_TexCoord;
void main() {
  tint_symbol_3 inputs;
  inputs.a_Position = a_Position;
  inputs.a_UV = a_UV;
  inputs.a_Color = a_Color;
  inputs.a_Normal = a_Normal;
  inputs.a_PosMtxIdx = a_PosMtxIdx;
  tint_symbol_4 outputs;
  outputs = tint_symbol_1(inputs);
  v_Color = outputs.v_Color;
  v_TexCoord = outputs.v_TexCoord;
  gl_Position = outputs.member;
  gl_Position.y = -gl_Position.y;
}


Error parsing GLSL shader:
ERROR: 0:157: 'v_Color' : redefinition 
ERROR: 1 compilation errors.  No code generated.



