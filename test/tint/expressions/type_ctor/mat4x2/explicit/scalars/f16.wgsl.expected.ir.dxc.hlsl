SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  tint_symbol = m;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol'
  tint_symbol = m;
  ^
hlsl.hlsl:3:17: error: use of undeclared identifier 'm'
  tint_symbol = m;
                ^

