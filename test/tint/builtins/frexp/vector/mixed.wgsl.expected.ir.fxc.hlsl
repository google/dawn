SKIP: FAILED

struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  frexp_result_vec2_f32 res = {float2(0.625f, 0.9375f), int2(1, 2)};
  frexp_result_vec2_f32 v = frexp(runtime_in);
  res = v;
  frexp_result_vec2_f32 v_1 = {float2(0.625f, 0.9375f), int2(1, 2)};
  res = v_1;
  float2 fract = res.fract;
  int2 exp = res.exp;
}

FXC validation failure:
<scrubbed_path>(11,29-45): error X3013: 'frexp': no matching 1 parameter intrinsic function
<scrubbed_path>(11,29-45): error X3013: Possible intrinsic functions are:
<scrubbed_path>(11,29-45): error X3013:     frexp(float|half, out float|half exp)


tint executable returned error: exit status 1
