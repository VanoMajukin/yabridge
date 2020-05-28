// yabridge: a Wine VST bridge
// Copyright (C) 2020  Robbert van der Helm
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "boost-fix.h"

#include <iostream>

// Generated inside of build directory
#include <src/common/config/config.h>
#include <src/common/config/version.h>

#include "bridges/group.h"
#include "bridges/vst2.h"

/**
 * This works very similar to the host application defined in
 * `individual-host.cpp`, but instead of just loading a single plugin this will
 * act as a daemon that can host multiple 'grouped' plugins. This works by
 * allowing the `libyabridge.so` instance to connect this this process over a
 * socket to ask this process to host a VST `.dll` file using a provided socket.
 * After that initialization step both the regular individual plugin host and
 * this group plugin host will function identically on both the plugin and the
 * Wine VST host side.
 *
 * The explicit calling convention is needed to work around a bug introduced in
 * Wine 5.7: https://bugs.winehq.org/show_bug.cgi?id=49138
 */
int __cdecl main(int argc, char* argv[]) {
    // Instead of directly hosting a plugin, this process will receive a UNIX
    // domain socket endpoint path that it should listen on to allow yabridge
    // instances to spawn plugins in this process.
    if (argc < 2) {
        std::cerr << "Usage: "
#ifdef __i386__
                  << yabridge_group_host_name_32bit
#else
                  << yabridge_group_host_name
#endif
                  << " <unix_domain_socket>" << std::endl;

        return 1;
    }

    const std::string group_socket_endpoint_path(argv[1]);

    std::cerr << "Initializing yabridge group host version "
              << yabridge_git_version
#ifdef __i386__
              << " (32-bit compatibility mode)"
#endif
              << std::endl;

    try {
        GroupBridge bridge(group_socket_endpoint_path);

        // Blocks the main thread until all plugins have exited
        bridge.handle_incoming_connections();
    } catch (const boost::system::system_error& error) {
        // If another process is already listening on the socket, we'll just
        // print a message and exit quietly. This could happen if the host
        // starts multiple yabridge instances that all use the same plugin group
        // at the same time.
        // TODO: Check if this is printed on the right stream
        std::cerr << "Another process is already listening on this group's "
                     "socket, connecting to the existing process:"
                  << std::endl;
        std::cerr << error.what() << std::endl;

        return 0;
    }
}