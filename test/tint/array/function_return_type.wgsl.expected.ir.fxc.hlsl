
RWByteAddressBuffer s : register(u0);
typedef float ary_ret[4];
ary_ret f1() {
  float v[4] = (float[4])0;
  return v;
}

typedef float ary_ret_1[3][4];
ary_ret_1 f2() {
  float v_1[4] = f1();
  float v_2[4] = f1();
  float v_3[4] = v_1;
  float v_4[4] = v_2;
  float v_5[4] = f1();
  float v_6[3][4] = {v_3, v_4, v_5};
  return v_6;
}

typedef float ary_ret_2[2][3][4];
ary_ret_2 f3() {
  float v_7[3][4] = f2();
  float v_8[3][4] = v_7;
  float v_9[3][4] = f2();
  float v_10[2][3][4] = {v_8, v_9};
  return v_10;
}

[numthreads(1, 1, 1)]
void main() {
  float v_11[4] = f1();
  float v_12[3][4] = f2();
  float v_13[2][3][4] = f3();
  float a1[4] = v_11;
  float a2[3][4] = v_12;
  float a3[2][3][4] = v_13;
  s.Store(0u, asuint(((a1[int(0)] + a2[int(0)][int(0)]) + a3[int(0)][int(0)][int(0)])));
}

