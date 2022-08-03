SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 2> v2 = vector<float16_t, 2>(((float16_t(1.0h) + float16_t(2.0h))).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>(((float16_t(1.0h) + float16_t(2.0h))).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>(((float16_t(1.0h) + float16_t(2.0h))).xxxx);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002661C202C10(7,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002661C202C10(8,10-18): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000002661C202C10(9,10-18): error X3000: syntax error: unexpected token 'float16_t'

