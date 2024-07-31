struct S {
  int m;
};


static S A[4] = (S[4])0;
void f() {
  S v = {1};
  A[0] = v;
}

