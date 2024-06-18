SKIP: FAILED

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1 = tint_symbol;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol_1'
  tint_symbol_1 = tint_symbol;
  ^
hlsl.hlsl:3:19: error: use of undeclared identifier 'tint_symbol'
  tint_symbol_1 = tint_symbol;
                  ^

