SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int a__ = a;
  int b = a;
  int b__ = a__;
  s = (((a + a__) + b) + b__);
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 's'
  s = (((a + a__) + b) + b__);
  ^

