#!/bin/bash

if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
    echo "Running outside of pull request isn't supported yet"
    exit 0
fi

# Choose the commit against which to format
base_commit=$(git rev-parse $TRAVIS_BRANCH)
echo "Formatting against $TRAVIS_BRANCH a.k.a. $base_commit..."
echo

skipped_directories="(examples|generator|src/tests/(unittests|end2end)|third_party)"
# Find the files modified that need formatting
files_to_check=$(git diff --diff-filter=ACMR --name-only $base_commit | grep -E "*.(c|cpp|mm|h)$" | grep -vE "^$skipped_directories/*")
if [ -z "$files_to_check" ]; then
    echo "No modified files to format."
    exit 0
fi
echo "Checking formatting diff on these files:"
echo "$files_to_check"
echo
files_to_check=$(echo $files_to_check | tr '\n' ' ')

# Run git-clang-format, check if it formatted anything
format_output=$(scripts/git-clang-format --binary $1 --commit $base_commit --diff --style=file $files_to_check)
if [ "$format_output" == "clang-format did not modify any files" ] || [ "$format_output" == "no modified files to format" ] ; then
    exit 0
fi

# clang-format made changes, print them and fail Travis
echo "Following formatting changes needed:"
echo
echo "$format_output"
exit 1
