static uint3 gl_GlobalInvocationID = uint3(0u, 0u, 0u);
RWByteAddressBuffer resultMatrix : register(u2, space0);
ByteAddressBuffer firstMatrix : register(t0, space0);
ByteAddressBuffer secondMatrix : register(t1, space0);
cbuffer cbuffer_x_46 : register(b3, space0) {
  uint4 x_46[1];
};

float binaryOperation_f1_f1_(inout float a, inout float b) {
  float x_26 = 0.0f;
  const float x_13 = b;
  if ((x_13 == 0.0f)) {
    return 1.0f;
  }
  const float x_21 = b;
  if (!((round((x_21 - (2.0f * floor((x_21 / 2.0f))))) == 1.0f))) {
    const float x_29 = a;
    const float x_31 = b;
    x_26 = pow(abs(x_29), x_31);
  } else {
    const float x_34 = a;
    const float x_36 = a;
    const float x_38 = b;
    x_26 = (sign(x_34) * pow(abs(x_36), x_38));
  }
  return x_26;
}

void main_1() {
  int index = 0;
  int a_1 = 0;
  float param = 0.0f;
  float param_1 = 0.0f;
  const uint x_54 = gl_GlobalInvocationID.x;
  index = asint(x_54);
  a_1 = -10;
  const int x_63 = index;
  param = -4.0f;
  param_1 = -3.0f;
  const float x_68 = binaryOperation_f1_f1_(param, param_1);
  resultMatrix.Store((4u * uint(x_63)), asuint(x_68));
  return;
}

struct tint_symbol_1 {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_GlobalInvocationID_param);
  return;
}
