alias a = i32;

alias _a = i32;

alias b = a;

alias _b = _a;

fn f() {
  var c : b;
  var d : _b;
}
