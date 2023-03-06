fn length_becebf() {
  var arg_0 = vec4<f32>(0.0f);
  var res : f32 = length(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_becebf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_becebf();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_becebf();
}
