SKIP: FAILED

void foo() {
  int a = 0;
  float4 b = (0.0f).xxxx;
  float2x2 c = float2x2((0.0f).xx, (0.0f).xx);
  a = (a / 2);
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  c = (c * 2.0f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:6:12: error: cannot convert from 'float4x4' to 'float4'
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
           ^

