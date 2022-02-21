var<private> A : array<T, 4>;

type T = i32;

@stage(fragment)
fn f() {
  A[0] = 1;
}
