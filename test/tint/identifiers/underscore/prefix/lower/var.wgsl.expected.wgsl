var<private> a : i32 = 1;

var<private> _a : i32 = 2;

fn f() {
  var b : i32 = a;
  var _b : i32 = _a;
}
