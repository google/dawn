RWByteAddressBuffer x_16 : register(u0, space0);
ByteAddressBuffer x_20 : register(t1, space0);
static uint3 gl_GlobalInvocationID = uint3(0u, 0u, 0u);
cbuffer cbuffer_x_24 : register(b2, space0) {
  uint4 x_24[2];
};

float getAAtOutCoords_() {
  const uint x_42 = gl_GlobalInvocationID.x;
  const float x_44 = asfloat(x_20.Load((4u * x_42)));
  return x_44;
}

float unaryOperation_f1_(inout float a) {
  const float x_47 = a;
  if ((x_47 < 0.0f)) {
    return asfloat(0x7f800000u);
  }
  const float x_55 = a;
  return log(x_55);
}

void setOutput_i1_f1_(inout int flatIndex, inout float value) {
  const int x_27 = flatIndex;
  const float x_28 = value;
  x_16.Store((4u * uint(x_27)), asuint(x_28));
  return;
}

void main_1() {
  int index = 0;
  float a_1 = 0.0f;
  float param = 0.0f;
  int param_1 = 0;
  float param_2 = 0.0f;
  const uint x_61 = gl_GlobalInvocationID.x;
  index = asint(x_61);
  const int x_63 = index;
  const int x_70 = asint(x_24[1].x);
  if ((x_63 < x_70)) {
    const float x_75 = getAAtOutCoords_();
    a_1 = x_75;
    param = a_1;
    const float x_78 = unaryOperation_f1_(param);
    param_1 = index;
    param_2 = x_78;
    setOutput_i1_f1_(param_1, param_2);
  }
  return;
}

struct tint_symbol_1 {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(128, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.gl_GlobalInvocationID_param);
  return;
}
