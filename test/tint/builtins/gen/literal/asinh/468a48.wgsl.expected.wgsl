enable f16;

fn asinh_468a48() {
  var res : f16 = asinh(1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_468a48();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_468a48();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_468a48();
}
