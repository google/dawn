SKIP: FAILED

RWByteAddressBuffer buf : register(u1, space0);

int g() {
  return 0;
}

int f() {
  [loop] while (true) {
    g();
    break;
  }
  const int o = g();
  return 0;
}

[numthreads(1, 1, 1)]
void main() {
  [loop] while (true) {
    if ((buf.Load(0u) == 0u)) {
      break;
    }
    int s = f();
    buf.Store(0u, asuint(0u));
  }
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000025FB94834E0(8,10-21): error X3531: can't unroll loops marked with loop attribute

