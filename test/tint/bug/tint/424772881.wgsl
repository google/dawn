@group(0) @binding(0) var<storage, read_write> result: vec3<u32>;

var<workgroup> wgvar: vec3<bool>;

@compute @workgroup_size(1)
fn main() {
  let v = wgvar;
  wgvar = v;

  let e = wgvar[0];
  wgvar[1] = e;

  workgroupBarrier();

  result = vec3<u32>(wgvar);
}
