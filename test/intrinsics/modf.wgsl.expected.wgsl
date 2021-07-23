[[stage(compute), workgroup_size(1)]]
fn main() {
  let res = modf(1.230000019);
  let fract : f32 = res.fract;
  let whole : f32 = res.whole;
}
