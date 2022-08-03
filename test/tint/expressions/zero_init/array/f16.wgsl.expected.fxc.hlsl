SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v[4] = (float16_t[4])0;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001C7D06B43D0(7,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\expressions\Shader@0x000001C7D06B43D0(7,13): error X3000: unrecognized identifier 'v'

