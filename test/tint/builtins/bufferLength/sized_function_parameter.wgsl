var<workgroup> v : buffer<64>;

@group(0) @binding(0) var<storage, read_write> out : u32;

fn foo(p : ptr<workgroup, buffer<64>>) {
  out = bufferLength(p);
}

@compute @workgroup_size(1)
fn main() {
  foo(&v);
}
