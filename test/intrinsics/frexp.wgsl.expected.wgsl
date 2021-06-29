[[stage(compute), workgroup_size(1)]]
fn main() {
  var exponent : i32;
  let significand : f32 = frexp(1.230000019, &(exponent));
}
