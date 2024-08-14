SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  float16_t r = (a / (b + b));
}

FXC validation failure:
C:\src\dawn\Shader@0x00000235AE34AFE0(4,3-11): error X3000: unrecognized identifier 'float16_t'
C:\src\dawn\Shader@0x00000235AE34AFE0(4,13): error X3000: unrecognized identifier 'a'

