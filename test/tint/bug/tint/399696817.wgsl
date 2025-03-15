// flags: --hlsl-shader-model 66
var<workgroup> a : atomic<i32>;
fn foo(in: u32) {
  let x = unpack4xI8(in);
  let y = unpack4xU8(in);
  let z = atomicLoad(&a);
}
