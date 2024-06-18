SKIP: FAILED

[numthreads(1, 1, 1)]
void tint_symbol_3() {
  tint_symbol_2 = 0u;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol_2'
  tint_symbol_2 = 0u;
  ^

