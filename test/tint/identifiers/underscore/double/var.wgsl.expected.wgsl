var<private> a : i32 = 1;

var<private> a__ : i32 = 2;

fn f() {
  var b : i32 = a;
  var b__ : i32 = a__;
}
