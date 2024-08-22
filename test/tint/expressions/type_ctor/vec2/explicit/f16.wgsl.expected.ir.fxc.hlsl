SKIP: INVALID


static vector<float16_t, 2> v = vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h));
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x00000117167E80E0(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

