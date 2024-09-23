struct Mat4x3_ {
  float4 mx;
  float4 my;
  float4 mz;
};

struct Mat4x4_ {
  float4 mx;
  float4 my;
  float4 mz;
  float4 mw;
};

struct Mat4x2_ {
  float4 mx;
  float4 my;
};

struct VertexOutput {
  float4 v_Color;
  float2 v_TexCoord;
  float4 member;
};

struct main_outputs {
  float4 VertexOutput_v_Color : TEXCOORD0;
  float2 VertexOutput_v_TexCoord : TEXCOORD1;
  float4 VertexOutput_member : SV_Position;
};

struct main_inputs {
  float3 a_Position : TEXCOORD0;
  float2 a_UV : TEXCOORD1;
  float4 a_Color : TEXCOORD2;
  float3 a_Normal : TEXCOORD3;
  float a_PosMtxIdx : TEXCOORD4;
};


cbuffer cbuffer_global : register(b0) {
  uint4 global[4];
};
cbuffer cbuffer_global1 : register(b1) {
  uint4 global1[3];
};
cbuffer cbuffer_global2 : register(b2) {
  uint4 global2[96];
};
static float3 a_Position1 = (0.0f).xxx;
static float2 a_UV1 = (0.0f).xx;
static float4 a_Color1 = (0.0f).xxxx;
static float3 a_Normal1 = (0.0f).xxx;
static float a_PosMtxIdx1 = 0.0f;
static float4 v_Color = (0.0f).xxxx;
static float2 v_TexCoord = (0.0f).xx;
static float4 gl_Position = (0.0f).xxxx;
float3 Mat4x3GetCol0_(Mat4x3_ m) {
  Mat4x3_ m1 = (Mat4x3_)0;
  m1 = m;
  Mat4x3_ v_1 = m1;
  Mat4x3_ v_2 = m1;
  Mat4x3_ v_3 = m1;
  Mat4x3_ x_e2 = v_1;
  Mat4x3_ x_e5 = v_2;
  Mat4x3_ x_e8 = v_3;
  return float3(x_e2.mx[0u], x_e5.my[0u], x_e8.mz[0u]);
}

float3 Mat4x3GetCol1_(Mat4x3_ m2) {
  Mat4x3_ m3 = (Mat4x3_)0;
  m3 = m2;
  Mat4x3_ v_4 = m3;
  Mat4x3_ v_5 = m3;
  Mat4x3_ v_6 = m3;
  Mat4x3_ x_e2 = v_4;
  Mat4x3_ x_e5 = v_5;
  Mat4x3_ x_e8 = v_6;
  return float3(x_e2.mx[1u], x_e5.my[1u], x_e8.mz[1u]);
}

float3 Mat4x3GetCol2_(Mat4x3_ m4) {
  Mat4x3_ m5 = (Mat4x3_)0;
  m5 = m4;
  Mat4x3_ v_7 = m5;
  Mat4x3_ v_8 = m5;
  Mat4x3_ v_9 = m5;
  Mat4x3_ x_e2 = v_7;
  Mat4x3_ x_e5 = v_8;
  Mat4x3_ x_e8 = v_9;
  return float3(x_e2.mx[2u], x_e5.my[2u], x_e8.mz[2u]);
}

float3 Mat4x3GetCol3_(Mat4x3_ m6) {
  Mat4x3_ m7 = (Mat4x3_)0;
  m7 = m6;
  Mat4x3_ v_10 = m7;
  Mat4x3_ v_11 = m7;
  Mat4x3_ v_12 = m7;
  Mat4x3_ x_e2 = v_10;
  Mat4x3_ x_e5 = v_11;
  Mat4x3_ x_e8 = v_12;
  return float3(x_e2.mx[3u], x_e5.my[3u], x_e8.mz[3u]);
}

float4 Mul(Mat4x4_ m8, float4 v) {
  Mat4x4_ m9 = (Mat4x4_)0;
  float4 v1 = (0.0f).xxxx;
  m9 = m8;
  v1 = v;
  Mat4x4_ v_13 = m9;
  float4 x_e6 = v1;
  Mat4x4_ v_14 = m9;
  float4 x_e10 = v1;
  Mat4x4_ v_15 = m9;
  float4 x_e14 = v1;
  Mat4x4_ v_16 = m9;
  float4 x_e18 = v1;
  Mat4x4_ x_e4 = v_13;
  float v_17 = dot(x_e4.mx, x_e6);
  Mat4x4_ x_e8 = v_14;
  float v_18 = dot(x_e8.my, x_e10);
  Mat4x4_ x_e12 = v_15;
  float v_19 = dot(x_e12.mz, x_e14);
  Mat4x4_ x_e16 = v_16;
  return float4(v_17, v_18, v_19, dot(x_e16.mw, x_e18));
}

float3 Mul1(Mat4x3_ m10, float4 v2) {
  Mat4x3_ m11 = (Mat4x3_)0;
  float4 v3 = (0.0f).xxxx;
  m11 = m10;
  v3 = v2;
  Mat4x3_ v_20 = m11;
  float4 x_e6 = v3;
  Mat4x3_ v_21 = m11;
  float4 x_e10 = v3;
  Mat4x3_ v_22 = m11;
  float4 x_e14 = v3;
  Mat4x3_ x_e4 = v_20;
  float v_23 = dot(x_e4.mx, x_e6);
  Mat4x3_ x_e8 = v_21;
  float v_24 = dot(x_e8.my, x_e10);
  Mat4x3_ x_e12 = v_22;
  return float3(v_23, v_24, dot(x_e12.mz, x_e14));
}

float2 Mul2(Mat4x2_ m12, float4 v4) {
  Mat4x2_ m13 = (Mat4x2_)0;
  float4 v5 = (0.0f).xxxx;
  m13 = m12;
  v5 = v4;
  Mat4x2_ v_25 = m13;
  float4 x_e6 = v5;
  Mat4x2_ v_26 = m13;
  float4 x_e10 = v5;
  Mat4x2_ x_e4 = v_25;
  float v_27 = dot(x_e4.mx, x_e6);
  Mat4x2_ x_e8 = v_26;
  return float2(v_27, dot(x_e8.my, x_e10));
}

float4 Mul3(float3 v6, Mat4x3_ m14) {
  float3 v7 = (0.0f).xxx;
  Mat4x3_ m15 = (Mat4x3_)0;
  v7 = v6;
  m15 = m14;
  Mat4x3_ v_28 = m15;
  Mat4x3_ x_e5 = v_28;
  float3 x_e6 = Mat4x3GetCol0_(x_e5);
  float3 x_e7 = v7;
  Mat4x3_ v_29 = m15;
  Mat4x3_ x_e10 = v_29;
  float3 x_e11 = Mat4x3GetCol1_(x_e10);
  float3 x_e12 = v7;
  Mat4x3_ v_30 = m15;
  Mat4x3_ x_e15 = v_30;
  float3 x_e16 = Mat4x3GetCol2_(x_e15);
  float3 x_e17 = v7;
  Mat4x3_ v_31 = m15;
  Mat4x3_ x_e20 = v_31;
  float3 x_e21 = Mat4x3GetCol3_(x_e20);
  float3 x_e22 = v7;
  float v_32 = dot(x_e6, x_e7);
  float v_33 = dot(x_e11, x_e12);
  float v_34 = dot(x_e16, x_e17);
  return float4(v_32, v_33, v_34, dot(x_e21, x_e22));
}

Mat4x4_ x_Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = (Mat4x4_)0;
  n1 = n;
  float x_e4 = n1;
  o.mx = float4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n1;
  o.my = float4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n1;
  o.mz = float4(0.0f, 0.0f, x_e18, 0.0f);
  float x_e25 = n1;
  o.mw = float4(0.0f, 0.0f, 0.0f, x_e25);
  Mat4x4_ v_35 = o;
  Mat4x4_ x_e27 = v_35;
  return x_e27;
}

Mat4x4_ x_Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = (Mat4x3_)0;
  Mat4x4_ o1 = (Mat4x4_)0;
  m17 = m16;
  Mat4x4_ v_36 = x_Mat4x4_(1.0f);
  Mat4x4_ x_e4 = v_36;
  o1 = x_e4;
  Mat4x3_ v_37 = m17;
  Mat4x3_ x_e7 = v_37;
  o1.mx = x_e7.mx;
  Mat4x3_ v_38 = m17;
  Mat4x3_ x_e10 = v_38;
  o1.my = x_e10.my;
  Mat4x3_ v_39 = m17;
  Mat4x3_ x_e13 = v_39;
  o1.mz = x_e13.mz;
  Mat4x4_ v_40 = o1;
  Mat4x4_ x_e15 = v_40;
  return x_e15;
}

Mat4x4_ x_Mat4x4_2(Mat4x2_ m18) {
  Mat4x2_ m19 = (Mat4x2_)0;
  Mat4x4_ o2 = (Mat4x4_)0;
  m19 = m18;
  Mat4x4_ v_41 = x_Mat4x4_(1.0f);
  Mat4x4_ x_e4 = v_41;
  o2 = x_e4;
  Mat4x2_ v_42 = m19;
  Mat4x2_ x_e7 = v_42;
  o2.mx = x_e7.mx;
  Mat4x2_ v_43 = m19;
  Mat4x2_ x_e10 = v_43;
  o2.my = x_e10.my;
  Mat4x4_ v_44 = o2;
  Mat4x4_ x_e12 = v_44;
  return x_e12;
}

Mat4x3_ x_Mat4x3_(float n2) {
  float n3 = 0.0f;
  Mat4x3_ o3 = (Mat4x3_)0;
  n3 = n2;
  float x_e4 = n3;
  o3.mx = float4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n3;
  o3.my = float4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n3;
  o3.mz = float4(0.0f, 0.0f, x_e18, 0.0f);
  Mat4x3_ v_45 = o3;
  Mat4x3_ x_e21 = v_45;
  return x_e21;
}

Mat4x3_ x_Mat4x3_1(Mat4x4_ m20) {
  Mat4x4_ m21 = (Mat4x4_)0;
  Mat4x3_ o4 = (Mat4x3_)0;
  m21 = m20;
  Mat4x4_ v_46 = m21;
  Mat4x4_ x_e4 = v_46;
  o4.mx = x_e4.mx;
  Mat4x4_ v_47 = m21;
  Mat4x4_ x_e7 = v_47;
  o4.my = x_e7.my;
  Mat4x4_ v_48 = m21;
  Mat4x4_ x_e10 = v_48;
  o4.mz = x_e10.mz;
  Mat4x3_ v_49 = o4;
  Mat4x3_ x_e12 = v_49;
  return x_e12;
}

Mat4x2_ v_50(uint start_byte_offset) {
  float4 v_51 = asfloat(global1[(start_byte_offset / 16u)]);
  Mat4x2_ v_52 = {v_51, asfloat(global1[((16u + start_byte_offset) / 16u)])};
  return v_52;
}

Mat4x4_ v_53(uint start_byte_offset) {
  float4 v_54 = asfloat(global[(start_byte_offset / 16u)]);
  float4 v_55 = asfloat(global[((16u + start_byte_offset) / 16u)]);
  float4 v_56 = asfloat(global[((32u + start_byte_offset) / 16u)]);
  Mat4x4_ v_57 = {v_54, v_55, v_56, asfloat(global[((48u + start_byte_offset) / 16u)])};
  return v_57;
}

Mat4x3_ v_58(uint start_byte_offset) {
  float4 v_59 = asfloat(global2[(start_byte_offset / 16u)]);
  float4 v_60 = asfloat(global2[((16u + start_byte_offset) / 16u)]);
  Mat4x3_ v_61 = {v_59, v_60, asfloat(global2[((32u + start_byte_offset) / 16u)])};
  return v_61;
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void main1() {
  Mat4x3_ t_PosMtx = (Mat4x3_)0;
  float2 t_TexSpaceCoord = (0.0f).xx;
  float x_e15 = a_PosMtxIdx1;
  Mat4x3_ v_62 = v_58((48u * uint(tint_f32_to_i32(x_e15))));
  Mat4x3_ x_e18 = v_62;
  t_PosMtx = x_e18;
  Mat4x3_ v_63 = t_PosMtx;
  Mat4x3_ x_e23 = v_63;
  Mat4x4_ x_e24 = x_Mat4x4_1(x_e23);
  float3 x_e25 = a_Position1;
  Mat4x3_ v_64 = t_PosMtx;
  Mat4x3_ x_e29 = v_64;
  Mat4x4_ v_65 = x_Mat4x4_1(x_e29);
  float3 x_e31 = a_Position1;
  Mat4x4_ x_e30 = v_65;
  float4 x_e34 = Mul(x_e30, float4(x_e31, 1.0f));
  Mat4x4_ v_66 = v_53(0u);
  Mat4x3_ v_67 = t_PosMtx;
  Mat4x3_ x_e37 = v_67;
  Mat4x4_ x_e38 = x_Mat4x4_1(x_e37);
  float3 x_e39 = a_Position1;
  Mat4x3_ v_68 = t_PosMtx;
  Mat4x3_ x_e43 = v_68;
  Mat4x4_ v_69 = x_Mat4x4_1(x_e43);
  float3 x_e45 = a_Position1;
  Mat4x4_ x_e44 = v_69;
  float4 x_e48 = Mul(x_e44, float4(x_e45, 1.0f));
  Mat4x4_ x_e35 = v_66;
  float4 x_e49 = Mul(x_e35, x_e48);
  gl_Position = x_e49;
  float4 x_e50 = a_Color1;
  v_Color = x_e50;
  float4 x_e52 = asfloat(global1[2u]);
  if ((x_e52[0u] == 2.0f)) {
    float3 x_e59 = a_Normal1;
    Mat4x2_ v_70 = v_50(0u);
    float3 x_e65 = a_Normal1;
    Mat4x2_ x_e64 = v_70;
    float2 x_e68 = Mul2(x_e64, float4(x_e65, 1.0f));
    v_TexCoord = x_e68.xy;
    return;
  } else {
    float2 x_e73 = a_UV1;
    Mat4x2_ v_71 = v_50(0u);
    float2 x_e80 = a_UV1;
    Mat4x2_ x_e79 = v_71;
    float2 x_e84 = Mul2(x_e79, float4(x_e80, 1.0f, 1.0f));
    v_TexCoord = x_e84.xy;
    return;
  }
  /* unreachable */
}

VertexOutput main_inner(float3 a_Position, float2 a_UV, float4 a_Color, float3 a_Normal, float a_PosMtxIdx) {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  float4 x_e11 = v_Color;
  float2 x_e13 = v_TexCoord;
  float4 x_e15 = gl_Position;
  VertexOutput v_72 = {x_e11, x_e13, x_e15};
  return v_72;
}

main_outputs main(main_inputs inputs) {
  VertexOutput v_73 = main_inner(inputs.a_Position, inputs.a_UV, inputs.a_Color, inputs.a_Normal, inputs.a_PosMtxIdx);
  VertexOutput v_74 = v_73;
  VertexOutput v_75 = v_73;
  VertexOutput v_76 = v_73;
  main_outputs v_77 = {v_74.v_Color, v_75.v_TexCoord, v_76.member};
  return v_77;
}

