#version 310 es

struct S1 {
  int i;
};

struct S2 {
  S1 s1;
};

struct S3 {
  S2 s2;
};

int tint_symbol;
int f(S3 s3) {
  return s3.s2.s1.i;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol = f(S3(S2(S1(42))));
}
