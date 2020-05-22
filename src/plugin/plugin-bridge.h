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

#pragma once

#include <vestige/aeffectx.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/process/child.hpp>
#include <mutex>
#include <thread>

#include "../common/logging.h"
#include "configuration.h"
#include "utils.h"

/**
 * This handles the communication between the Linux native VST plugin and the
 * Wine VST host. The functions below should be used as callback functions in an
 * `AEffect` object.
 */
class PluginBridge {
   public:
    /**
     * Initializes the Wine VST bridge. This sets up the sockets for event
     * handling.
     *
     * @param host_callback The callback function passed to the VST plugin by
     *   the host.
     *
     * @throw std::runtime_error Thrown when the VST host could not be found, or
     *   if it could not locate and load a VST .dll file.
     */
    PluginBridge(audioMasterCallback host_callback);

    // The four below functions are the handlers from the VST2 API. They are
    // called through proxy functions in `plugin.cpp`.

    /**
     * Handle an event sent by the VST host. Most of these opcodes will be
     * passed through to the winelib VST host.
     */
    intptr_t dispatch(AEffect* plugin,
                      int opcode,
                      int index,
                      intptr_t value,
                      void* data,
                      float option);
    /**
     * Ask the VST plugin to process audio for us. If the plugin somehow does
     * not support `processReplacing()` and only supports the old `process()`
     * function, then this will be handled implicitely in
     * `Vst2Bridge::handle_process_replacing()`.
     */
    void process_replacing(AEffect* plugin,
                           float** inputs,
                           float** outputs,
                           int sample_frames);
    float get_parameter(AEffect* plugin, int index);
    void set_parameter(AEffect* plugin, int index, float value);

    /**
     * The configuration for this instance of yabridge. Set based on the values
     * from a `yabridge.toml`, if it exists.
     *
     * @see Configuration::load_for
     */
    Configuration config;

    /**
     * The path to the .dll being loaded in the Wine VST host.
     */
    const boost::filesystem::path vst_plugin_path;
    /**
     * Whether the plugin is 64-bit or 32-bit.
     */
    const PluginArchitecture vst_plugin_arch;
    /**
     * The path to the host application (i.e. a path to either
     * `yabridge-host.exe` or `yabridge-host-32.exe`). The host application will
     * be chosen depending on the architecture of the VST plugin .dll file.
     */
    const boost::filesystem::path vst_host_path;

    /**
     * This AEffect struct will be populated using the data passed by the Wine
     * VST host during initialization and then passed as a pointer to the Linux
     * native VST host from the Linux VST plugin's entry point.
     */
    AEffect plugin;

   private:
    /**
     * Write output from an async pipe to the log on a line by line basis.
     * Useful for logging the Wine process's STDOUT and STDERR streams.
     *
     * @param pipe The pipe to read from.
     * @param buffer The stream buffer to write to.
     * @param prefix Text to prepend to the line before writing to the log.
     */
    void async_log_pipe_lines(patched_async_pipe& pipe,
                              boost::asio::streambuf& buffer,
                              std::string prefix = "");

    /**
     * Launch the Wine VST host to host the plugin. When using plugin groups,
     * this will first try to connect to the plugin group's socket (determined
     * based on group name, Wine prefix and architecture). If that fails, it
     * will launch a new, detached group host process. This will likely outlive
     * this plugin instance if multiple instances of yabridge using the same
     * plugin group are in use. In the event that two yabridge instances are
     * initialized at the same time and both instances spawn their own group
     * host process, then the later one will simply terminate gracefully after
     * it fails to listen on the socket.
     */
    void launch_vst_host();

    /**
     * Format and log all relevant debug information during initialization.
     */
    void log_init_message();

    boost::asio::io_context io_context;
    boost::asio::local::stream_protocol::endpoint socket_endpoint;
    boost::asio::local::stream_protocol::acceptor socket_acceptor;

    // The naming convention for these sockets is `<from>_<to>_<event>`. For
    // instance the socket named `host_vst_dispatch` forwards
    // `AEffect.dispatch()` calls from the native VST host to the Windows VST
    // plugin (through the Wine VST host).

    /**
     * The socket that forwards all `dispatcher()` calls from the VST host to
     * the plugin. This is also used once at startup to populate the values of
     * the `AEffect` object.
     */
    boost::asio::local::stream_protocol::socket host_vst_dispatch;
    /**
     * Used specifically for the `effProcessEvents` opcode. This is needed
     * because the Win32 API is designed to block during certain GUI
     * interactions such as resizing a window or opening a dropdown. Without
     * this MIDI input would just stop working at times.
     */
    boost::asio::local::stream_protocol::socket host_vst_dispatch_midi_events;
    boost::asio::local::stream_protocol::socket vst_host_callback;
    /**
     * Used for both `getParameter` and `setParameter` since they mostly
     * overlap.
     */
    boost::asio::local::stream_protocol::socket host_vst_parameters;
    boost::asio::local::stream_protocol::socket host_vst_process_replacing;

    /**
     * Whether we're done accepting sockets. The plugin may hang indefinitely if
     * the Wine process fails to start, since then nothing will connect to our
     * sockets. While we're waiting for our sockets we'll periodically poll the
     * Wine process to see if it's still running, and terminate the socket
     * accepting if it is not.
     */
    std::atomic_bool finished_accepting_sockets;

    /**
     * The thread that handles host callbacks.
     */
    std::thread host_callback_handler;

    /**
     * A binary semaphore to prevent race conditions from the dispatch function
     * being called by two threads at once. See `send_event()` for more
     * information.
     */
    std::mutex dispatch_mutex;
    std::mutex dispatch_midi_events_mutex;
    /**
     * A similar semaphore as the `dispatch_*` semaphores in the rare case that
     * `getParameter()` and `setParameter()` are being called at the same time
     * since they use the same socket.
     */
    std::mutex parameters_mutex;

    /**
     * The callback function passed by the host to the VST plugin instance.
     */
    audioMasterCallback host_callback_function;

    /**
     * The logging facility used for this instance of yabridge. See
     * `Logger::create_from_env()` for how this is configured.
     *
     * @see Logger::create_from_env
     */
    Logger logger;

    /**
     * The version of Wine currently in use. Used in the debug output on plugin
     * startup.
     */
    const std::string wine_version;

    boost::asio::streambuf wine_stdout_buffer;
    boost::asio::streambuf wine_stderr_buffer;
    /**
     * The STDOUT stream of the Wine process we can forward to the logger.
     */
    patched_async_pipe wine_stdout;
    /**
     * The STDERR stream of the Wine process we can forward to the logger.
     */
    patched_async_pipe wine_stderr;
    /**
     * Runs the Boost.Asio `io_context` thread for logging the Wine process
     * STDOUT and STDERR messages.
     */
    std::thread wine_io_handler;

    /**
     * The Wine process hosting the Windows VST plugin.
     *
     * @see launch_vst_host
     */
    boost::process::child vst_host;
    /**
     * The PID of the vst host process. Needed for checking whether the group
     * host is still active if we are connecting to an already running group
     * host instance.
     *
     * TODO: Remove this after encapsulating the minor differences in individual
     *       and group host handling
     */
    pid_t vst_host_pid;
    /**
     * A thread that waits for the group host to have started and then ask it to
     * host our plugin. This is used to defer the request since it may take a
     * little while until the group host process is up and running. This way we
     * don't have to delay the rest of the initialization process.
     *
     * TODO: Remove this after encapsulating the minor differences in individual
     *       and group host handling
     * TODO: Replace this with inotify to prevent delays and to reduce wasting
     *       resources
     */
    std::thread group_host_connect_handler;

    /**
     * A scratch buffer for sending and receiving data during `process` and
     * `processReplacing` calls.
     */
    std::vector<uint8_t> process_buffer;

    /**
     * The VST host can query a plugin for arbitrary binary data such as
     * presets. It will expect the plugin to write back a pointer that points to
     * that data. This vector is where we store the chunk data for the last
     * `effGetChunk` event.
     */
    std::vector<uint8_t> chunk_data;
    /**
     * The VST host will expect to be returned a pointer to a struct that stores
     * the dimensions of the editor window.
     */
    VstRect editor_rectangle;

    /**
     * Sending MIDI events sent to the host by the plugin using
     * `audioMasterProcessEvents` function has to be done during the processing
     * function. If they are sent during any other time or from another thread,
     * then the host will just discard them. Because we're receiving our host
     * callbacks on a separate thread, we have to temporarily store any events
     * we receive so we can send them to the host at the end of
     * `process_replacing()`.
     */
    std::vector<DynamicVstEvents> incoming_midi_events;
    /**
     * Mutex for locking the above event queue, since recieving and processing
     * now happens in two different threads.
     */
    std::mutex incoming_midi_events_mutex;
};
