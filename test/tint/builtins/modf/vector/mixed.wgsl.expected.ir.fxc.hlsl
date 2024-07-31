SKIP: FAILED

struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 runtime_in = float2(1.25f, 3.75f);
  modf_result_vec2_f32 res = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  modf_result_vec2_f32 v = modf(runtime_in);
  res = v;
  modf_result_vec2_f32 v_1 = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  res = v_1;
  float2 fract = res.fract;
  float2 whole = res.whole;
}

FXC validation failure:
c:\src\dawn\Shader@0x000002B4F8076C00(11,28-43): error X3013: 'modf': no matching 1 parameter intrinsic function
c:\src\dawn\Shader@0x000002B4F8076C00(11,28-43): error X3013: Possible intrinsic functions are:
c:\src\dawn\Shader@0x000002B4F8076C00(11,28-43): error X3013:     modf(float|half|min10float|min16float, out float|half|min10float|min16float ip)

