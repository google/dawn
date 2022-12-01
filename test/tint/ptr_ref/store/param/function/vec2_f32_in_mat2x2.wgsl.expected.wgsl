enable chromium_experimental_full_ptr_parameters;

fn func(pointer : ptr<function, vec2<f32>>) {
  *(pointer) = vec2<f32>();
}

@compute @workgroup_size(1)
fn main() {
  var F : mat2x2<f32>;
  func(&(F[1]));
}
