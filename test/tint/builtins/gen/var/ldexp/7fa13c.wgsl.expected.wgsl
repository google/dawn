enable f16;

fn ldexp_7fa13c() {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = vec4<i32>(1i);
  var res : vec4<f16> = ldexp(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_7fa13c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_7fa13c();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_7fa13c();
}
