SKIP: FAILED


static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
C:\src\dawn\Shader@0x0000029E54930820(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

