@group(0) @binding(0) var<storage, read_write> v : buffer;

@group(0) @binding(1) var<storage, read_write> out : u32;

fn foo(p : ptr<storage, buffer, read_write>) {
  out = bufferLength(p);
}

@compute @workgroup_size(1)
fn main() {
  foo(&v);
}
