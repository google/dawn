@group(0) @binding(0) var<storage, read> v : buffer;

@group(0) @binding(1) var<storage, read_write> out : vec4u;

@fragment
fn main() {
  let offset = 16u;
  let p = bufferView<vec4u>(&(v), offset);
  out = *(p);
}
