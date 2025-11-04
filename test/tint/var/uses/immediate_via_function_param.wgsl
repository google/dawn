enable chromium_experimental_immediate;

var<immediate> value: u32;

@group(0) @binding(1)
var<storage, read_write> output: u32;

fn foo(p: ptr<immediate, u32>) {
  output = *p;
}

@compute @workgroup_size(1)
fn main() {
    foo(&value);
}
