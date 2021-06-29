var<workgroup> arg_1 : vec2<i32>;

fn frexp_a3f940() {
  var res : vec2<f32> = frexp(vec2<f32>(), &(arg_1));
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  frexp_a3f940();
}
