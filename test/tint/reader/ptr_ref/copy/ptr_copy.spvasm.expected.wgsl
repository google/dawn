@compute @workgroup_size(1u, 1u, 1u)
fn main() {
  var v : u32;
  let v_1 = &(v);
  let v_2 = v_1;
}
