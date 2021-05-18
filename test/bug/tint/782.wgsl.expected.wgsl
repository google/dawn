type ArrayExplicitStride = [[stride(4)]] array<i32, 2>;

type ArrayImplicitStride = array<i32, 2>;

fn foo() {
  var explicit : ArrayExplicitStride;
  var implict : ArrayImplicitStride;
  implict = explicit;
}
