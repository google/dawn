SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int B = A;
  int _B = _A;
  s = (B + _B);
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'A'
  int B = A;
          ^
hlsl.hlsl:4:12: error: use of undeclared identifier '_A'
  int _B = _A;
           ^
hlsl.hlsl:5:3: error: use of undeclared identifier 's'
  s = (B + _B);
  ^

