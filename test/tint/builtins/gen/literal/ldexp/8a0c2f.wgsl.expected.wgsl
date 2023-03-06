enable f16;

fn ldexp_8a0c2f() {
  var res : vec4<f16> = ldexp(vec4<f16>(1.0h), vec4(1));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_8a0c2f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_8a0c2f();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_8a0c2f();
}
