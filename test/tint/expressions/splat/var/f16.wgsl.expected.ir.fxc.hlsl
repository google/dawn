SKIP: FAILED


void f() {
  float16_t v = float16_t(3.0h);
  vector<float16_t, 2> v2 = vector<float16_t, 2>((v).xx);
  vector<float16_t, 3> v3 = vector<float16_t, 3>((v).xxx);
  vector<float16_t, 4> v4 = vector<float16_t, 4>((v).xxxx);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000027E6544DF80(3,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x0000027E6544DF80(3,13): error X3000: unrecognized identifier 'v'

