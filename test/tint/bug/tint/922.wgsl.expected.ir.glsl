#version 310 es


struct Mat4x4_ {
  vec4 mx;
  vec4 my;
  vec4 mz;
  vec4 mw;
};

struct ub_SceneParams {
  Mat4x4_ u_Projection;
};

struct Mat4x2_ {
  vec4 mx;
  vec4 my;
};

struct ub_MaterialParams {
  Mat4x2_ u_TexMtx[1];
  vec4 u_Misc0_;
};

struct Mat4x3_ {
  vec4 mx;
  vec4 my;
  vec4 mz;
};

struct ub_PacketParams {
  Mat4x3_ u_PosMtx[32];
};

struct VertexOutput {
  vec4 v_Color;
  vec2 v_TexCoord;
  vec4 member;
};

layout(binding = 0, std140)
uniform global_block_1_ubo {
  ub_SceneParams inner;
} v_1;
layout(binding = 1, std140)
uniform global1_block_1_ubo {
  ub_MaterialParams inner;
} v_2;
layout(binding = 2, std140)
uniform global2_block_1_ubo {
  ub_PacketParams inner;
} v_3;
vec3 a_Position1 = vec3(0.0f);
vec2 a_UV1 = vec2(0.0f);
vec4 a_Color1 = vec4(0.0f);
vec3 a_Normal1 = vec3(0.0f);
float a_PosMtxIdx1 = 0.0f;
vec4 v_Color = vec4(0.0f);
vec2 v_TexCoord = vec2(0.0f);
vec4 tint_symbol = vec4(0.0f);
layout(location = 0) in vec3 tint_symbol_1_loc0_Input;
layout(location = 1) in vec2 tint_symbol_1_loc1_Input;
layout(location = 2) in vec4 tint_symbol_1_loc2_Input;
layout(location = 3) in vec3 tint_symbol_1_loc3_Input;
layout(location = 4) in float tint_symbol_1_loc4_Input;
layout(location = 0) out vec4 tint_symbol_1_loc0_Output;
layout(location = 1) out vec2 tint_symbol_1_loc1_Output;
vec4 Mul(Mat4x4_ m8, vec4 v) {
  Mat4x4_ m9 = Mat4x4_(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec4 v1 = vec4(0.0f);
  m9 = m8;
  v1 = v;
  Mat4x4_ x_e4 = m9;
  vec4 x_e6 = v1;
  Mat4x4_ x_e8 = m9;
  vec4 x_e10 = v1;
  Mat4x4_ x_e12 = m9;
  vec4 x_e14 = v1;
  Mat4x4_ x_e16 = m9;
  vec4 x_e18 = v1;
  float v_4 = dot(x_e4.mx, x_e6);
  float v_5 = dot(x_e8.my, x_e10);
  float v_6 = dot(x_e12.mz, x_e14);
  return vec4(v_4, v_5, v_6, dot(x_e16.mw, x_e18));
}
vec2 Mul2(Mat4x2_ m12, vec4 v4) {
  Mat4x2_ m13 = Mat4x2_(vec4(0.0f), vec4(0.0f));
  vec4 v5 = vec4(0.0f);
  m13 = m12;
  v5 = v4;
  Mat4x2_ x_e4 = m13;
  vec4 x_e6 = v5;
  Mat4x2_ x_e8 = m13;
  vec4 x_e10 = v5;
  float v_7 = dot(x_e4.mx, x_e6);
  return vec2(v_7, dot(x_e8.my, x_e10));
}
Mat4x4_ x_Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = Mat4x4_(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  n1 = n;
  float x_e4 = n1;
  o.mx = vec4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n1;
  o.my = vec4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n1;
  o.mz = vec4(0.0f, 0.0f, x_e18, 0.0f);
  float x_e25 = n1;
  o.mw = vec4(0.0f, 0.0f, 0.0f, x_e25);
  Mat4x4_ x_e27 = o;
  return x_e27;
}
Mat4x4_ x_Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = Mat4x3_(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  Mat4x4_ o1 = Mat4x4_(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f));
  m17 = m16;
  Mat4x4_ x_e4 = x_Mat4x4_(1.0f);
  o1 = x_e4;
  Mat4x3_ x_e7 = m17;
  o1.mx = x_e7.mx;
  Mat4x3_ x_e10 = m17;
  o1.my = x_e10.my;
  Mat4x3_ x_e13 = m17;
  o1.mz = x_e13.mz;
  Mat4x4_ x_e15 = o1;
  return x_e15;
}
int tint_f32_to_i32(float value) {
  return mix(2147483647, mix((-2147483647 - 1), int(value), (value >= -2147483648.0f)), (value <= 2147483520.0f));
}
void main1() {
  Mat4x3_ t_PosMtx = Mat4x3_(vec4(0.0f), vec4(0.0f), vec4(0.0f));
  vec2 t_TexSpaceCoord = vec2(0.0f);
  float x_e15 = a_PosMtxIdx1;
  int v_8 = tint_f32_to_i32(x_e15);
  Mat4x3_ x_e18 = v_3.inner.u_PosMtx[v_8];
  t_PosMtx = x_e18;
  Mat4x3_ x_e23 = t_PosMtx;
  Mat4x4_ x_e24 = x_Mat4x4_1(x_e23);
  vec3 x_e25 = a_Position1;
  Mat4x3_ x_e29 = t_PosMtx;
  Mat4x4_ x_e30 = x_Mat4x4_1(x_e29);
  vec3 x_e31 = a_Position1;
  vec4 x_e34 = Mul(x_e30, vec4(x_e31, 1.0f));
  Mat4x4_ x_e35 = v_1.inner.u_Projection;
  Mat4x3_ x_e37 = t_PosMtx;
  Mat4x4_ x_e38 = x_Mat4x4_1(x_e37);
  vec3 x_e39 = a_Position1;
  Mat4x3_ x_e43 = t_PosMtx;
  Mat4x4_ x_e44 = x_Mat4x4_1(x_e43);
  vec3 x_e45 = a_Position1;
  vec4 x_e48 = Mul(x_e44, vec4(x_e45, 1.0f));
  vec4 x_e49 = Mul(x_e35, x_e48);
  tint_symbol = x_e49;
  vec4 x_e50 = a_Color1;
  v_Color = x_e50;
  vec4 x_e52 = v_2.inner.u_Misc0_;
  if ((x_e52[0u] == 2.0f)) {
    vec3 x_e59 = a_Normal1;
    Mat4x2_ x_e64 = v_2.inner.u_TexMtx[0];
    vec3 x_e65 = a_Normal1;
    vec2 x_e68 = Mul2(x_e64, vec4(x_e65, 1.0f));
    v_TexCoord = x_e68.xy;
    return;
  } else {
    vec2 x_e73 = a_UV1;
    Mat4x2_ x_e79 = v_2.inner.u_TexMtx[0];
    vec2 x_e80 = a_UV1;
    vec2 x_e84 = Mul2(x_e79, vec4(x_e80, 1.0f, 1.0f));
    v_TexCoord = x_e84.xy;
    return;
  }
  /* unreachable */
}
VertexOutput tint_symbol_1_inner(vec3 a_Position, vec2 a_UV, vec4 a_Color, vec3 a_Normal, float a_PosMtxIdx) {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  vec4 x_e11 = v_Color;
  vec2 x_e13 = v_TexCoord;
  vec4 x_e15 = tint_symbol;
  return VertexOutput(x_e11, x_e13, x_e15);
}
void main() {
  VertexOutput v_9 = tint_symbol_1_inner(tint_symbol_1_loc0_Input, tint_symbol_1_loc1_Input, tint_symbol_1_loc2_Input, tint_symbol_1_loc3_Input, tint_symbol_1_loc4_Input);
  tint_symbol_1_loc0_Output = v_9.v_Color;
  tint_symbol_1_loc1_Output = v_9.v_TexCoord;
  gl_Position = v_9.member;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  gl_PointSize = 1.0f;
}
