SKIP: FAILED

void foo() {
  a = (a / 2);
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  c = (c * 2.0f);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:2:3: error: use of undeclared identifier 'a'
  a = (a / 2);
  ^
hlsl.hlsl:2:8: error: use of undeclared identifier 'a'
  a = (a / 2);
       ^
hlsl.hlsl:3:3: error: use of undeclared identifier 'b'
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  ^
hlsl.hlsl:3:8: error: use of undeclared identifier 'b'
  b = (b * float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
       ^
hlsl.hlsl:4:3: error: use of undeclared identifier 'c'
  c = (c * 2.0f);
  ^
hlsl.hlsl:4:8: error: use of undeclared identifier 'c'
  c = (c * 2.0f);
       ^

