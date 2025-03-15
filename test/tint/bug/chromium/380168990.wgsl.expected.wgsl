const one = 1i;

const two = 2i;

const two_minus_minus_one = (two - -(one));

const two_minus_minus_minus_one = (two - -(-(one)));

const two_minus_minus_minus_minus_one = (two - -(-(-(one))));

const two_minus_minus_minus_minus_minus_one = (two - -(-(-(-(one)))));

const two_minus_minus_minus_minus_minus_minus_one = (two - -(-(-(-(-(one))))));

const two_minus_minus_minus_minus_minus_minus_minus_one = (two - -(-(-(-(-(-(one)))))));

const two_minus_minus_minus_minus_minus_minus_minus_minus_one = (two - -(-(-(-(-(-(-(one))))))));

const two_minus_minus_minus_minus_minus_minus_minus_minus_minus_one = (two - -(-(-(-(-(-(-(-(one)))))))));

const two_minus_minus_minus_minus_minus_minus_minus_minus_minus_minus_one = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const_assert (two_minus_minus_one == 3i);

const_assert (two_minus_minus_minus_one == 1i);

const_assert (two_minus_minus_minus_minus_one == 3i);

const_assert (two_minus_minus_minus_minus_minus_one == 1i);

const_assert (two_minus_minus_minus_minus_minus_minus_one == 3i);

const_assert (two_minus_minus_minus_minus_minus_minus_minus_one == 1i);

const_assert (two_minus_minus_minus_minus_minus_minus_minus_minus_one == 3i);

const_assert (two_minus_minus_minus_minus_minus_minus_minus_minus_minus_one == 1i);

const_assert (two_minus_minus_minus_minus_minus_minus_minus_minus_minus_minus_one == 3i);

const two_4_minus_one_with_spaces_0 = (two - -(-(-(one))));

const two_4_minus_one_with_spaces_1 = (two - -(-(-(one))));

const two_10_minus_one_with_spaces_0 = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const two_10_minus_one_with_spaces_1 = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const two_10_minus_one_with_spaces_2 = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const two_10_minus_one_with_spaces_3 = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const two_10_minus_one_with_spaces_4 = (two - -(-(-(-(-(-(-(-(-(one))))))))));

const_assert (two_4_minus_one_with_spaces_0 == 3i);

const_assert (two_4_minus_one_with_spaces_1 == 3i);

const_assert (two_10_minus_one_with_spaces_0 == 3i);

const_assert (two_10_minus_one_with_spaces_1 == 3i);

const_assert (two_10_minus_one_with_spaces_2 == 3i);

const_assert (two_10_minus_one_with_spaces_3 == 3i);

const_assert (two_10_minus_one_with_spaces_4 == 3i);

@fragment
fn main() {
  var a = (two_minus_minus_minus_minus_minus_minus_minus_minus_minus_minus_one - -(-(-(-(-(-(-(-(-(-(-(-(-(-(one)))))))))))))));
}
