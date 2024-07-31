SKIP: FAILED


static vector<float16_t, 4> v = vector<float16_t, 4>(float16_t(0.0h), float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000001BBCCD05BA0(2,15-23): error X3000: syntax error: unexpected token 'float16_t'

