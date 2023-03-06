enable f16;

fn ceil_4bca2a() {
  var res : vec4<f16> = ceil(vec4<f16>(1.5h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_4bca2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_4bca2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_4bca2a();
}
