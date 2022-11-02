#version 310 es

struct Uniforms {
  float NAN;
  int aShape;
  int outShape;
  int outShapeStrides;
  int size;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std430) buffer ssbOut_ssbo {
  float result[];
} x_16;

layout(binding = 1, std430) buffer ssbA_ssbo {
  float A[];
} x_20;

uvec3 tint_symbol = uvec3(0u, 0u, 0u);
layout(binding = 2, std140) uniform x_24_block_ubo {
  Uniforms inner;
} x_24;

float getAAtOutCoords_() {
  uint x_42 = tint_symbol.x;
  float x_44 = x_20.A[x_42];
  return x_44;
}

float unaryOperation_f1_(inout float a) {
  float x_47 = a;
  if ((x_47 < 0.0f)) {
    return 0.0f /* inf */;
  }
  float x_55 = a;
  return log(x_55);
}

void setOutput_i1_f1_(inout int flatIndex, inout float value) {
  int x_27 = flatIndex;
  float x_28 = value;
  x_16.result[x_27] = x_28;
  return;
}

void main_1() {
  int index = 0;
  float a_1 = 0.0f;
  float param = 0.0f;
  int param_1 = 0;
  float param_2 = 0.0f;
  uint x_61 = tint_symbol.x;
  index = int(x_61);
  int x_63 = index;
  int x_70 = x_24.inner.size;
  if ((x_63 < x_70)) {
    float x_75 = getAAtOutCoords_();
    a_1 = x_75;
    float x_77 = a_1;
    param = x_77;
    float x_78 = unaryOperation_f1_(param);
    int x_80 = index;
    param_1 = x_80;
    param_2 = x_78;
    setOutput_i1_f1_(param_1, param_2);
  }
  return;
}

void tint_symbol_1(uvec3 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_GlobalInvocationID);
  return;
}
