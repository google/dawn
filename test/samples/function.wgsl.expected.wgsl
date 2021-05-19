fn main() -> f32 {
  return (((2.0 * 3.0) - 4.0) / 5.0);
}

[[stage(compute), workgroup_size(2)]]
fn ep() {
}
