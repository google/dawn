SKIP: FAILED


static float16_t u = float16_t(1.0h);
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x0000020556673890(2,8-16): error X3000: unrecognized identifier 'float16_t'

