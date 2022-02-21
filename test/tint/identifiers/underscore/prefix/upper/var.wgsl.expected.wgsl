var<private> A : i32 = 1;

var<private> _A : i32 = 2;

fn f() {
  var B : i32 = A;
  var _B : i32 = _A;
}
