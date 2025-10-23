alias ArrayExplicitStride = array<i32, 2>;

alias ArrayImplicitStride = array<i32, 2>;

@compute @workgroup_size(1)
fn foo() {
  var explicitStride : ArrayExplicitStride;
  var implictStride : ArrayImplicitStride;
  implictStride = explicitStride;
}
