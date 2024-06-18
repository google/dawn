SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  int b = a;
  int b__ = a__;
  s = (b + b__);
}

DXC validation failure:
hlsl.hlsl:3:11: error: use of undeclared identifier 'a'
  int b = a;
          ^
hlsl.hlsl:4:13: error: use of undeclared identifier 'a__'
  int b__ = a__;
            ^
hlsl.hlsl:5:3: error: use of undeclared identifier 's'
  s = (b + b__);
  ^

