#version 310 es
precision mediump float;


layout (binding = 0) buffer ssbOut_1 {
  float result[];
} x_16;
layout (binding = 1) buffer ssbA_1 {
  float A[];
} x_20;
uvec3 tint_symbol = uvec3(0u, 0u, 0u);
layout (binding = 2) uniform Uniforms_1 {
  float NAN;
  int aShape;
  int outShape;
  int outShapeStrides;
  int size;
} x_24;

float getAAtOutCoords_() {
  uint x_42 = tint_symbol.x;
  float x_44 = x_20.A[x_42];
  return x_44;
}

float unaryOperation_f1_(inout float a) {
  float x_47 = a;
  if ((x_47 < 0.0f)) {
    return uintBitsToFloat(0x7f800000u);
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
  int x_70 = x_24.size;
  if ((x_63 < x_70)) {
    float x_75 = getAAtOutCoords_();
    a_1 = x_75;
    param = a_1;
    float x_78 = unaryOperation_f1_(param);
    param_1 = index;
    param_2 = x_78;
    setOutput_i1_f1_(param_1, param_2);
  }
  return;
}

struct tint_symbol_4 {
  uvec3 tint_symbol_2;
};

void tint_symbol_1_inner(uvec3 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void tint_symbol_1(tint_symbol_4 tint_symbol_3) {
  tint_symbol_1_inner(tint_symbol_3.tint_symbol_2);
  return;
}
void main() {
  tint_symbol_4 inputs;
  inputs.tint_symbol_2 = gl_GlobalInvocationID;
  tint_symbol_1(inputs);
}


