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

BuildRequires(pre): rpm-macros-meson
BuildRequires: meson
BuildRequires: wine-devel-tools
BuildRequires: libxcb-devel
BuildRequires: libdbus-devel
BuildRequires: /proc
BuildRequires: cmake

Requires: wine

%description
Yet Another way to use Windows audio plugins on Linux. Yabridge seamlessly supports using both 32-bit and 64-bit Windows VST2, VST3, and CLAP plugins in 64-bit Linux plugin hosts as if they were native plugins, with optional support for plugin groups to enable inter-plugin communication for VST2 plugins and quick startup times. Its modern concurrent architecture and focus on transparency allows yabridge to be both fast and highly compatible, while also staying easy to debug and maintain.

%prep
%setup

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
* Fri Jun 16 2023 Ivan Mazhukin <vanomj@altlinux.org> 5.0.5-alt1
- Initial build for Sisyphus

