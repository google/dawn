const TRUE = true;
const FALSE = false;
const_assert(true || FALSE);
const_assert(!(false && true));

@compute @workgroup_size(1)
fn f() {
  _ = TRUE;
  var x = false;
  var y = false;
  if (x && (true || y)) { }
}
