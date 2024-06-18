SKIP: FAILED

[numthreads(1, 1, 1)]
void f() {
  tint_symbol = float4x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f), float3(6.0f, 7.0f, 8.0f), float3(9.0f, 10.0f, 11.0f));
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'tint_symbol'
  tint_symbol = float4x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f), float3(6.0f, 7.0f, 8.0f), float3(9.0f, 10.0f, 11.0f));
  ^

