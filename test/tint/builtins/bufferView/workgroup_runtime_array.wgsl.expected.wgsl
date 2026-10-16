@group(0) @binding(0) var<storage> in : array<u32>;

@group(0) @binding(1) var<storage, read_write> out : array<u32>;

const size = 256;

var<workgroup> b : buffer<size>;

@compute @workgroup_size(1)
fn main() {
  let view = bufferView<array<u32>>(&(b), 0);
  for(var i = 0; (i < (size / 4)); i++) {
    view[i] = in[i];
  }
  workgroupBarrier();
  for(var i = 0; (i < (size / 4)); i++) {
    out[i] = view[i];
  }
}
