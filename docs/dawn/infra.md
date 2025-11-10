# Dawn's Continuous Testing Infrastructure

Dawn uses Chromium's continuous integration (CI) infrastructure to continually run tests on changes to Dawn and provide a way for developers to run tests against their changes before submitting. CI bots continually build and run tests for batches of new changes (although in practice, it is often once per submitted CL), and Try bots build and run developers' pending changes before submission. Dawn uses two different builder setups:

1. Standalone Dawn checkouts, which directly check out Dawn and its dependencies. This is used to provide test coverage of Dawn itself.
2. Chromium checkouts, which are capable of building Dawn targets. This is used to provide coverage of Dawn tests using Chromium's versions of dependencies while also allowing Dawn-related Chromium tests to run. For example, most WebGPU CTS testing is done on Chromium builders in order to make use of a full Chromium browser.

 - [Dawn CI Builders](https://ci.chromium.org/p/dawn/g/ci/builders)
 - [Dawn Try Builders](https://ci.chromium.org/p/dawn/g/try/builders)
 - [chromium.dawn Waterfall](https://ci.chromium.org/p/chromium/g/chromium.dawn/console)

For additional information on GPU testing in Chromium, please see [[chromium/src]//docs/gpu/gpu_testing_bot_details.md](https://chromium.googlesource.com/chromium/src.git/+/main/docs/gpu/gpu_testing_bot_details.md).

## Dawn CI/Try Builders
Standalone Dawn builders are defined in the following Starlark files, which are used to generate files consumed by the builders when running [//infra/config/global/main.star](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/infra/config/global/main.star):
  - [//infra/config/global/gn_standalone_ci.star](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/infra/config/global/gn_standalone_ci.star), which defines all CI builders which use GN as their buildsystem. This encompasses the majority of Dawn's CI builders.
  - [//infra/config/global/gn_standalone_try.star](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/infra/config/global/gn_standalone_try.star), which defines al try builders which use GN as their buildsystem. This includes both manual-only trybots and the ones automatically added to all CLs. This encompasses the majority of Dawn's try builders.
  - [//infra/config/global/legacy_builders.star](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/infra/config/global/legacy_builders.star), which defines all other CI and try builders such as those that use CMake or mirror Chromium builders. Long-term, all builders in this file will either be removed or moved to appropriately named files.

There are additional `chromium/try` builders, but those are described later in this document.

These bots are defined in both buckets luci.dawn.ci and luci.dawn.try, though their ACL permissions differ. luci.dawn.ci bots will be scheduled regularly based on [[dawn]//infra/config/global/luci-scheduler.cfg](../../infra/config/global/generated/luci-scheduler.cfg). luci.dawn.try bots will be triggered on the CQ based on [[dawn]//infra/config/global/commit-queue.cfg](../../infra/config/global/generated/commit-queue.cfg).

Build status for both CI and Try builders can be seen at this [console](https://ci.chromium.org/p/dawn) which is generated from [[dawn]//infra/config/global/luci-milo.cfg](../../infra/config/global/generated/luci-milo.cfg).

## Dawn Build Recipe
There are two recipes for building Dawn/Tint, one for [GN based](https://source.chromium.org/chromium/infra/infra_superproject/+/main:build/recipes/recipes/dawn/gn_v2.py) builds and one for [CMake based](https://source.chromium.org/chromium/infra/infra_superproject/+/main:build/recipes/recipes/dawn/cmake.py) builds.

The specific instructions on how to build the project are contained in the project repos build files. These recipe files are primary concerned with coordinating how to build and test the project for CQ/CI. The high level steps of this process:
1. Check out the repo
2. Compile the project (both Dawn & Tint)
   - Depending on the specific environment this may include multiple configurations, i.e. with `dawn.node` or fuzzing enabled in addition to the default build options
3. Run various tests
   - Including unit tests, end-to-end tests, CTS, and others as appropriate for the specific build configuration
4. _Optionally_ Generate and upload fuzzer corpora
   - This is controlled by running a special set of tests and only runs on one CI builder that is run once every 24 hours

**Note:** For Googlers there is an internal doc ([go/dawn-luci-guide](go/dawn-luci-guide)) with details on how to update the build recipes and test them.

## Dawn Chromium-Based CI Waterfall Bots
The [`chromium.dawn`](https://ci.chromium.org/p/chromium/g/chromium.dawn/console) waterfall consists of the bots specified in the `chromium.dawn` section of [[chromium/src]//testing/buildbot/waterfalls.pyl](https://source.chromium.org/search/?q=file:waterfalls.pyl%20chromium.dawn). Bots named "Builder" are responsible for building top-of-tree Dawn, whereas bots named "DEPS Builder" are responsible for building Chromium's DEPS version of Dawn.

The other bots, such as "Dawn Linux x64 DEPS Release (Intel HD 630)" receive the build products from the Builders and are responsible for running tests. The Tester configuration may specify `mixins` from [[chromium/src]//testing/buildbot/mixins.pyl](https://source.chromium.org/search/?q=file:buildbot/mixins.pyl) which help specify bot test dimensions like OS version and GPU vendor. The Tester configuration also specifies `test_suites` from [[chromium/src]//testing/buildbot/test_suites.pyl](https://source.chromium.org/search/?q=file:buildbot/test_suites.pyl%20dawn_end2end_tests) which declare the tests are arguments passed to tests that should be run on the bot.

The parent builders and child testers are configured in Chromium's `//infra/config/` files. CI builders are defined [here](https://source.chromium.org/chromium/chromium/src/+/main:infra/config/subprojects/chromium/ci/chromium.dawn.star) while try builders are defined [here](https://source.chromium.org/chromium/chromium/src/+/main:infra/config/subprojects/chromium/try/tryserver.chromium.dawn.star).

## Dawn Chromium-Based Tryjobs
[This Starlark file](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/infra/config/global/legacy_builders.star) defines mirrors of Chromium trybots for use as Dawn trybots. This allows Dawn changes to be tested in tests which require Chromium before actually attempting to roll Dawn into Chromium, such as running the WebGPU CTS in a full Chromium browser. The `chromium_dawn_tryjob` entries define mirrors which are automatically added to all Dawn CLs. These correspond to builder names such as `linux-dawn-rel` or `mac-dawn-rel`. The `luci.cq_tryjob_verifier` entries define manual-only trybot mirrors. These are never automatically added, but can be manually added through Gerrit or CL footers.

## Bot Allocation

Dawn shares all of its builder capacity and test hardware with the Chrome GPU team. Pool assignments for both are defined in [this file](https://chrome-internal.googlesource.com/infradata/config/+/refs/heads/main/configs/chromium-swarm/starlark/bots/chromium/gpu.star) (Google only). A capacity dashboard for these physical machines and GCE instances can be found at go/chrome-gpu-capacity-dashboard (Google only).

**Note:** At the time of writing, it is recommended to select "Old UI" at the top right of the capacity dashboard, as the new UI has some performance issues.

While all builders for a given OS share the same pools (e.g. all Linux trybots share the same pool of Linux GCE instances), builders typically have `max_concurrent_builds` set to limit how many builds can be running at once. This is to smooth out usage spikes and help prevent the physical test hardware from being overwhelmed.
