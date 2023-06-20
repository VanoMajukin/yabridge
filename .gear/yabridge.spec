Name: yabridge
Version: 5.0.5
Release: alt1

Summary: Bridge to use Windows audio plugins on Linux

License: GPL-3.0
Group: Sound
Url: https://github.com/robbert-vdh/yabridge

# Source-git: https://github.com/robbert-vdh/yabridge.git
Source: %name-%version.tar
Source1: yabridgectl

# Source2-url: https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-22-1.tar.gz
Source2: asio-1.22.1.tar

# Source3-url: https://github.com/fraillt/bitsery/archive/refs/tags/v5.2.2.tar.gz
Source3: bitsery-v5.2.2.tar

# Source4-url: https://github.com/free-audio/clap/archive/refs/tags/1.1.7.tar.gz
Source4: clap-1.1.7.tar

# Source5-url: https://github.com/Naios/function2/archive/refs/tags/4.2.0.tar.gz
Source5: function2-4.2.0.tar

# Source6-url: https://github.com/gulrak/filesystem/archive/refs/tags/v1.5.12.tar.gz
Source6: filesystem-v1.5.12.tar

# Source7-url: https://github.com/marzer/tomlplusplus/archive/refs/tags/v3.3.0.tar.gz
Source7: tomlplusplus-v3.3.0.tar

# Source8-url: https://github.com/robbert-vdh/vst3sdk/archive/refs/tags/v3.7.7_build_19-patched.tar.gz
Source8: vst3sdk-v3.7.7_build_19-patched.tar

# Source9-url: https://github.com/steinbergmedia/vst3_base/archive/refs/tags/v3.7.7_build_19.tar.gz
Source9: vst3_base-v3.7.7_build_19.tar

# Source10-url: https://github.com/steinbergmedia/vst3_pluginterfaces/archive/refs/tags/v3.7.7_build_19.tar.gz
Source10:vst3_pluginterfaces-v3.7.7_build_19.tar

# Source11-url: https://github.com/steinbergmedia/vst3_public_sdk/archive/refs/tags/v3.7.7_build_19.tar.gz
Source11:vst3_public_sdk-v3.7.7_build_19.tar

BuildRequires(pre): rpm-macros-meson
BuildRequires: meson
BuildRequires: wine-devel-tools
BuildRequires: libxcb-devel
BuildRequires: libdbus-devel
BuildRequires: /proc
BuildRequires: cmake

Requires: wine

%description
Yet Another way to use Windows audio plugins on Linux. Yabridge seamlessly supports using 64-bit Windows VST2, VST3, and CLAP plugins in 64-bit Linux plugin hosts as if they were native plugins, with optional support for plugin groups to enable inter-plugin communication for VST2 plugins and quick startup times. Its modern concurrent architecture and focus on transparency allows yabridge to be both fast and highly compatible, while also staying easy to debug and maintain.

%prep
%setup -a2 -a3 -a4 -a5 -a6 -a7 -a8 -a9 -a10 -a11
cp -rf subprojects/packagefiles/* subprojects/

%build

meson setup build --buildtype=release --cross-file=cross-wine.conf --unity=on --unity-size=4000
ninja -C build

%install
cd build

install -dm755 %buildroot/usr/bin
install yabridge-host.exe{,.so} %buildroot/usr/bin
install %SOURCE1 %buildroot/usr/bin

install -dm755 %buildroot/usr/lib
install libyabridge{,-chainloader}-{vst2,vst3,clap}.so %buildroot/usr/lib

%files
%_bindir/yabridge-*
%_bindir/yabridgectl
%_libexecdir/libyabridge-*

%changelog
* Tue Jun 20 2023 Ivan Mazhukin <vanomj@altlinux.org> 5.0.5-alt1
- Initial build for Sisyphus
