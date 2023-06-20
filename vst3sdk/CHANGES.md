# Changes

To provide a better building experience for yabridge, this fork simply strips
out all parts of the VST3 SDK we do not need. The VSTGUI and documentation
submodules have been removed. Documentation can still be found
[here](https://developer.steinberg.help/display/VST) on Steinberg's website and
[here](https://steinbergmedia.github.io/vst3_doc/vstsdk/index.html) on GitHub.

This now temporarily also includes a Meson build definition until Meson allows
mixing CMake subprojects for native and cross compiled targets.
