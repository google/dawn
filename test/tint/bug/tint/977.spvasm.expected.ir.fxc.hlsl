struct main_inputs {
  uint3 gl_GlobalInvocationID_param : SV_DispatchThreadID;
};


static uint3 gl_GlobalInvocationID = (0u).xxx;
RWByteAddressBuffer resultMatrix : register(u2);
ByteAddressBuffer firstMatrix : register(t0);
ByteAddressBuffer secondMatrix : register(t1);
cbuffer cbuffer_x_46 : register(b3) {
  uint4 x_46[1];
};
float binaryOperation_f1_f1_(inout float a, inout float b) {
  float x_26 = 0.0f;
  if ((b == 0.0f)) {
    return 1.0f;
  }
  float x_21 = b;
  if (!((round((x_21 - (2.0f * floor((x_21 / 2.0f))))) == 1.0f))) {
    float v = abs(a);
    x_26 = pow(v, b);
  } else {
    float v_1 = float(sign(a));
    float v_2 = abs(a);
    x_26 = (v_1 * pow(v_2, b));
  }
  float x_41 = x_26;
  return x_41;
}

void main_1() {
  int index = int(0);
  int a_1 = int(0);
  float param = 0.0f;
  float param_1 = 0.0f;
  index = asint(gl_GlobalInvocationID.x);
  a_1 = int(-10);
  int x_63 = index;
  param = -4.0f;
  param_1 = -3.0f;
  float x_68 = binaryOperation_f1_f1_(param, param_1);
  uint v_3 = (0u + (uint(x_63) * 4u));
  resultMatrix.Store(v_3, asuint(x_68));
}

void main_inner(uint3 gl_GlobalInvocationID_param) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.gl_GlobalInvocationID_param);
}

