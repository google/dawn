SKIP: FAILED

struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  modf_result_f32 res = {0.25f, 1.0f};
  modf_result_f32 v = modf(runtime_in);
  res = v;
  modf_result_f32 v_1 = {0.25f, 1.0f};
  res = v_1;
  float fract = res.fract;
  float whole = res.whole;
}

FXC validation failure:
<scrubbed_path>(11,23-38): error X3013: 'modf': no matching 1 parameter intrinsic function
<scrubbed_path>(11,23-38): error X3013: Possible intrinsic functions are:
<scrubbed_path>(11,23-38): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)


tint executable returned error: exit status 1
