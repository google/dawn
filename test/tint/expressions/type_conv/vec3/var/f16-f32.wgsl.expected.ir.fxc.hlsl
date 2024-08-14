SKIP: FAILED


static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
void f() {
  float3 v = float3(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000028AA3EDAD90(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

