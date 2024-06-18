SKIP: FAILED

void f() {
  int b = a;
}

DXC validation failure:
hlsl.hlsl:2:11: error: use of undeclared identifier 'a'
  int b = a;
          ^

