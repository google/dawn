SKIP: FAILED

struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  modf_result_vec2_f32 v = modf(tint_symbol);
  modf_result_vec2_f32 res = v;
  float2 fract = res.fract;
  modf_result_vec2_f32 v_1 = v;
  float2 whole = v_1.whole;
}

FXC validation failure:
<scrubbed_path>(10,28-44): error X3013: 'modf': no matching 1 parameter intrinsic function
<scrubbed_path>(10,28-44): error X3013: Possible intrinsic functions are:
<scrubbed_path>(10,28-44): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)


tint executable returned error: exit status 1
