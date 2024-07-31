SKIP: FAILED

struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  frexp_result_f32 v = frexp(tint_symbol);
  frexp_result_f32 res = v;
  float fract = res.fract;
  frexp_result_f32 v_1 = v;
  int exp = v_1.exp;
}

FXC validation failure:
c:\src\dawn\Shader@0x0000027061073E10(10,24-41): error X3013: 'frexp': no matching 1 parameter intrinsic function
c:\src\dawn\Shader@0x0000027061073E10(10,24-41): error X3013: Possible intrinsic functions are:
c:\src\dawn\Shader@0x0000027061073E10(10,24-41): error X3013:     frexp(float|half, out float|half exp)

