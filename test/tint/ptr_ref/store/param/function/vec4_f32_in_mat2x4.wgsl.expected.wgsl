enable chromium_experimental_full_ptr_parameters;

fn func(pointer : ptr<function, vec4<f32>>) {
  *(pointer) = vec4<f32>();
}

@compute @workgroup_size(1)
fn main() {
  var F : mat2x4<f32>;
  func(&(F[1]));
}
