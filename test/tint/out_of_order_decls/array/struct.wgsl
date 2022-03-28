var<private> A : array<S, 4>;
struct S {
  m : i32,
};

@stage(fragment)
fn f() {
  A[0] = S(1);
}
