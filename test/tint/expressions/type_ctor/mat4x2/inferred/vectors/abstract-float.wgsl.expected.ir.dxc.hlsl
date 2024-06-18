SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  tint_symbol = float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f));
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol'
  tint_symbol = float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f));
  ^

