SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int b = a;
  int _b = _a;
  s = (b + _b);
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'a'
  int b = a;
          ^
hlsl.hlsl:4:12: error: use of undeclared identifier '_a'
  int _b = _a;
           ^
hlsl.hlsl:5:3: error: use of undeclared identifier 's'
  s = (b + _b);
  ^

