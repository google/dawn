fn select_416e14() {
  var res : f32 = select(1.0, 1.0, bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_416e14();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_416e14();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_416e14();
}
