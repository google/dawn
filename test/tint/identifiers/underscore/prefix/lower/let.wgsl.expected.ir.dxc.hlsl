SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int _a = a;
  int b = a;
  int _b = _a;
  s = (((a + _a) + b) + _b);
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 's'
  s = (((a + _a) + b) + _b);
  ^

