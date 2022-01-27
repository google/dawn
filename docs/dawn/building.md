# Building Dawn

## System requirements

- Linux
  - The `pkg-config` command:
    ```sh
    # Install pkg-config on Ubuntu
    sudo apt-get install pkg-config
    ```

- Mac
  - [Xcode](https://developer.apple.com/xcode/) 12.2+.
  - The macOS 11.0 SDK. Run `xcode-select` to check whether you have it.
    ```sh
    ls `xcode-select -p`/Platforms/MacOSX.platform/Developer/SDKs
    ```

## Install `depot_tools`

Dawn uses the Chromium build system and dependency management so you need to [install depot_tools] and add it to the PATH.

[install depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

## Get the code

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

## Build Dawn

Then generate build files using `gn args out/Debug` or `gn args out/Release`.
A text editor will appear asking build options, the most common option is `is_debug=true/false`; otherwise `gn args out/Release --list` shows all the possible options.

On macOS you'll want to add the `use_system_xcode=true` in most cases. (and if you're a googler please get XCode from go/xcode).

Then use `ninja -C out/Release` to build dawn and for example `./out/Release/dawn_end2end_tests` to run the tests.

