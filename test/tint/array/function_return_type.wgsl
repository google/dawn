fn f1() -> array<f32, 4> {
  return array<f32, 4>();
}

fn f2() -> array<array<f32, 4>, 3> {
  return array<array<f32, 4>, 3>(f1(), f1(), f1());
}

fn f3() -> array<array<array<f32, 4>, 3>, 2> {
  return array<array<array<f32, 4>, 3>, 2>(f2(), f2());
}

@compute @workgroup_size(1)
fn main() {
  let a1 : array<f32, 4> = f1();
  let a2 : array<array<f32, 4>, 3> = f2();
  let a3 : array<array<array<f32, 4>, 3>, 2> = f3();
}
