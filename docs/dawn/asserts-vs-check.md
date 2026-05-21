# Dawn: Checking asserts [Bug 442860471](https://crbug.com/442860471)

petermcneeley@

Apr 15, 2026

*There has been an influx of security bugs for Dawn due to a capability phase shift in AI-assisted vulnerability discovery.*

In Dawn, often these are triggered by going **outside known assumptions** or they become real security issues when further **assumptions are invalidated**.

## Fix: `DAWN_UNREACHABLE`

A good example of this is the assumption that all switches on enums are valid enums. Some minor vulnerability can lead to full execution control via the undefined behavior of a switch table that jumps to an invalid enum. Example <https://crbug.com/499159695>

To resolve this, all `DAWN_UNREACHABLE`s now check at runtime that unexpected fallthroughs (switch and function) will do a full `CHECK` (as in immediately crash/ICE in release).

This induces ~ 2 instructions per switch and is deemed to be acceptable.

See CL: <https://dawn-review.git.corp.google.com/c/dawn/+/301675>

## Fix: `DAWN_ASSERT` to `DAWN_CHECK`

There are other cases where an assumption would have caught a security issue but does not because the assumption in code is `DAWN_ASSERT`, not a check.

Example <https://crbug.com/500609033>

The conclusion is certain `DAWN_ASSERT`s will be promoted to release `DAWN_CHECK`s.

This is **more controversial** than the unreachable case as some of these checks can be expensive and some can be in hot code paths (paths that are called many times per frame/submit).

For the initial phase, `DAWN_ASSERT`s that will be promoted are:

* Cheap 1-2 instruction checks (null/bounds/error)
* Avoid checks that would be found via null page
    * There is no point in checking a null pointer if the very next line it will trigger a null page segfault (which is not considered a Chrome vulnerability)
* Hot code paths are allowed for checking memory bounds

CL: <https://dawn-review.git.corp.google.com/c/dawn/+/302235>


## Fix: `DAWN_ASSERT` to `DAWN_RELEASE_ASSUME`

Previously, in release, `DAWN_ASSERT` was converted into `__builtin_assume`. This is likely a surprise to most developers and can lead to dangerous outcomes.

This `assume` tells the compiler that a specific expression holds. The compiler then can do code generation based on this assumption.
If the assumption turns out to have been false, the result can be undefined behavior (UB).

Here is an example of `DAWN_ASSERT` being converted to an `assume` and then removing mandatory bounds checks. (This was very BAD)
CL: <https://dawn-review.git.corp.google.com/c/dawn/+/260462>


In conclusion, we should move away from an assert implying an assume, as this can lead to unintended consequences.
The macro `DAWN_ASSERT` will simply assert in debug and be a no-op in release.
For the cases where the intention really was to inform the compiler about assumptions we should use `DAWN_RELEASE_ASSUME`.

## Drawback(s)

The main drawback here is when do we get to remove these checks? **The answer is likely never!**

However, it is likely that within the next year we will be able to ask advanced AI security models what would be the impact of removing specific checks. It might even be that every future change in Dawn will get evaluated for security by an AI model.
