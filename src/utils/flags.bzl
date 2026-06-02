load("@bazel_skylib//rules:common_settings.bzl", "string_flag", "bool_flag")
load("@bazel_skylib//lib:selects.bzl", "selects")

def declare_os_flag():
    """Creates the 'os' string flag that specifies the OS to target, and a pair of
    'dawn_build_is_<os>_true' and 'dawn_build_is_<os>_false' targets.

    The OS flag can be specified on the command line with '--//src/tint:os=<os>'
    """

    OSes = [
        "win",
        "linux",
        "mac",
        "other"
    ]

    string_flag(
        name = "os",
        build_setting_default = "other",
        values = OSes,
    )

    for os in OSes:
        native.config_setting(
            name = "dawn_build_is_{}_true".format(os),
            flag_values = { ":os": os },
            visibility = ["//visibility:public"],
        )
        selects.config_setting_group(
            name = "dawn_build_is_{}_false".format(os),
            match_any = [ "dawn_build_is_{}_true".format(other) for other in OSes if other != os],
            visibility = ["//visibility:public"],
        )

COPTS = [
    "-fno-rtti",
    "-fno-exceptions",
    "--std=c++20",
]
