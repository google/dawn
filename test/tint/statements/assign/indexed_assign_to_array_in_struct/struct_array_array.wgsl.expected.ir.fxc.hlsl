SKIP: FAILED

struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[8][8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  InnerS v_1 = v;
  s1.a1[uniforms[0u].x][uniforms[0u].y] = v_1;
}

FXC validation failure:
C:\src\dawn\Shader@0x000002A19993CC50(18,3-23): error X3500: array reference cannot be used as an l-value; not natively addressable

