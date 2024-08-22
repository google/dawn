SKIP: FAILED

struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  frexp_result_vec2_f32 v = frexp(tint_symbol);
  frexp_result_vec2_f32 res = v;
  float2 fract = res.fract;
  frexp_result_vec2_f32 v_1 = v;
  int2 exp = v_1.exp;
}

FXC validation failure:
C:\src\dawn\Shader@0x0000027F6B32FC90(10,29-46): error X3013: 'frexp': no matching 1 parameter intrinsic function
C:\src\dawn\Shader@0x0000027F6B32FC90(10,29-46): error X3013: Possible intrinsic functions are:
C:\src\dawn\Shader@0x0000027F6B32FC90(10,29-46): error X3013:     frexp(float|half, out float|half exp)

