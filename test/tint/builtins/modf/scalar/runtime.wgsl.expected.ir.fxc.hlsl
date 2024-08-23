SKIP: FAILED

struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  modf_result_f32 v = modf(tint_symbol);
  modf_result_f32 res = v;
  float fract = res.fract;
  modf_result_f32 v_1 = v;
  float whole = v_1.whole;
}

FXC validation failure:
<scrubbed_path>(10,23-39): error X3013: 'modf': no matching 1 parameter intrinsic function
<scrubbed_path>(10,23-39): error X3013: Possible intrinsic functions are:
<scrubbed_path>(10,23-39): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)


tint executable returned error: exit status 1
