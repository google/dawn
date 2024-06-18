SKIP: FAILED

void X() {
}

float2 Y() {
  return (0.0f).xx;
}

void f() {
  float2 v = (0.0f).xx;
  X((0.0f).xx, v);
  X((0.0f).xx, Y());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

DXC validation failure:
hlsl.hlsl:10:3: error: no matching function for call to 'X'
  X((0.0f).xx, v);
  ^
hlsl.hlsl:1:6: note: candidate function not viable: requires 0 arguments, but 2 were provided
void X() {
     ^
hlsl.hlsl:11:3: error: no matching function for call to 'X'
  X((0.0f).xx, Y());
  ^
hlsl.hlsl:1:6: note: candidate function not viable: requires 0 arguments, but 2 were provided
void X() {
     ^

