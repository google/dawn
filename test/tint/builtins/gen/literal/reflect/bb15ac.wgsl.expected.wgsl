enable f16;

fn reflect_bb15ac() {
  var res : vec2<f16> = reflect(vec2<f16>(1.0h), vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_bb15ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_bb15ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_bb15ac();
}
