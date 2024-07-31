SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  float16_t v = (a / b);
  float16_t v_1 = floor(v);
  float16_t r = ((a - (((v < float16_t(0.0h))) ? (ceil(v)) : (v_1))) * b);
}

FXC validation failure:
c:\src\dawn\Shader@0x000001B14E4A0130(4,3-11): error X3000: unrecognized identifier 'float16_t'
c:\src\dawn\Shader@0x000001B14E4A0130(4,13): error X3000: unrecognized identifier 'a'

