SKIP: INVALID


static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x00000233D37BF8D0(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

