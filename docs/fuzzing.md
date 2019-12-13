# Fuzzing Dawn

## `dawn_wire_server_and_frontend_fuzzer`

The `dawn_wire_server_and_frontend_fuzzer` sets up Dawn using the Null backend, and passes inputs to the wire server. This fuzzes the `dawn_wire` deserialization, as well as Dawn's frontend validation.

## Updating the Seed Corpus

Using a seed corpus significantly improves the efficiency of fuzzing. Dawn's fuzzers use interesting testcases discovered in previous fuzzing runs to seed future runs. Fuzzing can be further improved by using Dawn tests as a example of API usage which allows the fuzzer to quickly discover and use new API entrypoints and usage patterns.

The script [update_fuzzer_seed_corpus.sh](../scripts/update_fuzzer_seed_corpus.sh) can be used to capture a trace while running Dawn tests, and merge it with all existing interesting fuzzer inputs.

To run the script:
1. Make sure gcloud is installed: https://g3doc.corp.google.com/cloud/sdk/g3doc/index.md?cl=head
2. Login with @google.com credentials: `gcloud auth login`
3. You must be in a Chromium checkout using the GN arg `use_libfuzzer=true`
4. Run `./third_party/dawn/scripts/update_fuzzer_seed_corpus.sh <out_dir> <fuzzer> <test>`.

   Example: `./third_party/dawn/scripts/update_fuzzer_seed_corpus.sh out/fuzz dawn_wire_server_and_frontend_fuzzer dawn_end2end_tests`
5. The script will print instructions for testing, and then uploading new inputs. Please, only upload inputs after testing the fuzzer with new inputs, and verifying there is a meaningful change in coverage.
