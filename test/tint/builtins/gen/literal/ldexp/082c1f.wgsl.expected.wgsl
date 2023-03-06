enable f16;

fn ldexp_082c1f() {
  var res : f16 = ldexp(1.0h, 1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_082c1f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_082c1f();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_082c1f();
}
