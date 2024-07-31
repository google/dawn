SKIP: FAILED

struct InnerS {
  int v;
};

struct S1 {
  InnerS s2;
};

struct OuterS {
  S1 a1[8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  InnerS v_1 = v;
  s1.a1[uniforms[0u].x].s2 = v_1;
}

FXC validation failure:
c:\src\dawn\Shader@0x00000119B40F6BC0(22,3-23): error X3500: array reference cannot be used as an l-value; not natively addressable

