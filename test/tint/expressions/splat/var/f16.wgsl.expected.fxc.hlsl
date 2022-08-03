SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v = (float16_t(1.0h) + float16_t(2.0h));
  vector<float16_t, 2> v2 = vector<float16_t, 2>((v).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>((v).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>((v).xxxx);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001FDC0B44610(7,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001FDC0B44610(7,13): error X3000: unrecognized identifier 'v'

