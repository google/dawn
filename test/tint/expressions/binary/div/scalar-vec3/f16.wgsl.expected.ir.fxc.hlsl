SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(4.0h);
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> r = (a / b);
}

FXC validation failure:
c:\src\dawn\Shader@0x000001B18E98F7D0(4,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x000001B18E98F7D0(4,13): error X3000: unrecognized identifier 'a'

