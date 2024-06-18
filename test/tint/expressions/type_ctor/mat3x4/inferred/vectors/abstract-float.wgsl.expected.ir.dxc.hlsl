SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  tint_symbol = float3x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f), float4(8.0f, 9.0f, 10.0f, 11.0f));
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol'
  tint_symbol = float3x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f), float4(8.0f, 9.0f, 10.0f, 11.0f));
  ^

