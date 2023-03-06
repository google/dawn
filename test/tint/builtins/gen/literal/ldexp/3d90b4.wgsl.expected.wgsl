enable f16;

fn ldexp_3d90b4() {
  var res : vec2<f16> = ldexp(vec2<f16>(1.0h), vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_3d90b4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_3d90b4();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_3d90b4();
}
