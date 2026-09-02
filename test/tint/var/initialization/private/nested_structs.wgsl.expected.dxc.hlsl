struct S1 {
  int i;
};

struct S2 {
  S1 s1;
};

struct S3 {
  S2 s2;
};


static const S3 v = {{{int(42)}}};
static S3 P = v;
RWByteAddressBuffer v_1 : register(u0);
[numthreads(1, 1, 1)]
void main() {
  v_1.Store(0u, asuint(P.s2.s1.i));
}

