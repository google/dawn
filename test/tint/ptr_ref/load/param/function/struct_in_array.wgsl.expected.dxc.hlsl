struct str {
  int i;
};


str func(inout str pointer) {
  str v = pointer;
  return v;
}

[numthreads(1, 1, 1)]
void main() {
  str F[4] = (str[4])0;
  str r = func(F[2u]);
}

