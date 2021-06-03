fn select_416e14() {
  var res : f32 = select(1.0, 1.0, bool());
}

[[stage(vertex)]]
fn vertex_main() {
  select_416e14();
}

[[stage(fragment)]]
fn fragment_main() {
  select_416e14();
}

[[stage(compute)]]
fn compute_main() {
  select_416e14();
}
