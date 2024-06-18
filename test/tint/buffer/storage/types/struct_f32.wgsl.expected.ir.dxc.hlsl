SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  S t = tint_symbol;
  tint_symbol_1 = t;
}

DXC validation failure:
hlsl.hlsl:3:3: error: unknown type name 'S'
  S t = tint_symbol;
  ^
hlsl.hlsl:4:3: error: use of undeclared identifier 'tint_symbol_1'
  tint_symbol_1 = t;
  ^

