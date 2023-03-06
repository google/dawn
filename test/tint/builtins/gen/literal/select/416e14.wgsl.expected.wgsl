fn select_416e14() {
  var res : f32 = select(1.0f, 1.0f, true);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_416e14();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_416e14();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_416e14();
}
