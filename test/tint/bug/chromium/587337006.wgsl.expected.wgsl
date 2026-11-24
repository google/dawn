@compute @workgroup_size(1)
fn main(@builtin(num_workgroups) n : vec3<u32>) {
  let i = firstTrailingBit(-(65537));
  let v = vec3(!(false));
  let b = !(vec3(all(v))[i]);
  switch(i) {
    default: {
    }
  }
}
