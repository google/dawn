
[numthreads(1, 1, 1)]
void f() {
  float3 a = float3(1.0f, 2.0f, 3.0f);
  float b = 4.0f;
  float3 v = (a / b);
  float3 v_1 = floor(v);
  float3 r = ((a - (((v < (0.0f).xxx)) ? (ceil(v)) : (v_1))) * b);
}

