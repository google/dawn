struct A {
  a : i32,
}

struct B {
  b : i32,
}

fn f(a : A) -> B {
  return B();
}

@compute @workgroup_size(1)
fn main() {
  _ = f(A(1));
}
