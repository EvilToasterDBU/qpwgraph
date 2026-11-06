# qpwgraph - A PipeWire Graph Qt GUI Interface

![Screenshot](src/images/qpwgraph_screenshot-4.png)

  **qpwgraph** is a graph manager dedicated to [PipeWire](https://pipewire.org),
  using the [Qt C++ framework](https://qt.io), based and pretty much like the
  same of [QjackCtl](https://qjackctl.sourceforge.io).

  Source code repository: https://github.com/EvilToasterDBU/qpwgraph

  Based on the upstream project at https://gitlab.freedesktop.org/rncbc/qpwgraph,
  by Rui Nuno Capela <rncbc@rncbc.org>.

  The redesign branch adds a snap grid, filters PipeWire to audio ports by
  default, supports saved user-defined audio pass-through nodes, and lets you
  collapse each node's input or output port group.

## Prerequisites

  **qpwgraph** software prerequisites for building are a C++20 compiler
  (_g++_), the [Qt C++ framework](https://qt.io) (_qt6-qtbase-devel_ or
  _qt5-qtbase-devel_) and of course the [PipeWire API](https://pipewire.org)
  C development libraries and headers (_pipewire-devel_).

  By default, the graph displays PipeWire audio streams only. Non-audio
  PipeWire ports and ALSA MIDI support are excluded. ALSA MIDI can be enabled
  at build time with `-DCONFIG_ALSA_MIDI=ON`; this requires the [ALSA](https://www.alsa-project.org)
  development libraries and headers (_alsa-devel_).

## Building

  **qpwgraph** uses the [CMake](https://cmake.org) build system, version
  3.15 or newer.

  On the source distribution top directory:

    cmake [-DCMAKE_INSTALL_PREFIX=<prefix>] -B build
    cmake --build build [--parallel <Njobs>]

  After successful build you may test run it immediately as follows:

    build/src/qpwgraph

  If you may install it permanently, then run, optionally as root:

    [sudo] cmake --install build

  Note that the default installation path (\<_prefix_\>) is `/usr/local` .


  Enjoy.

## Packages

  The GitHub Actions workflow builds Debian (`.deb`), Fedora (`.rpm`), Arch
  Linux (`.pkg.tar.zst`), and AppImage packages when the redesign branch is
  updated or a `v*` tag is pushed. Workflow runs can also be started manually;
  their packages are available as downloadable workflow artifacts.

## Documentation

  * [User Manual](docs/qpwgraph-user_manual.md)
  * [How To Use The Patchbay](docs/qpwgraph_patchbay-user_manual.md)

## License

  **qpwgraph** is free, open-source software, distributed under the terms of
  the GNU General Public License ([GPL](https://www.gnu.org/copyleft/gpl.html))
  version 2 or later.

## Copyright

  Copyright (C) 2021-2026, rncbc aka Rui Nuno Capela. All rights reserved.

