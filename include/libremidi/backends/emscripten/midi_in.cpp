#if defined(__EMSCRIPTEN__)
  #include <libremidi/backends/emscripten/midi_access.hpp>
  #include <libremidi/backends/emscripten/midi_in.hpp>
  #include <libremidi/detail/midi_stream_decoder.hpp>

  #include <chrono>

namespace libremidi
{
LIBREMIDI_INLINE midi_in_emscripten::midi_in_emscripten(
    input_configuration&& conf, emscripten_input_configuration&& apiconf)
    : configuration{std::move(conf), std::move(apiconf)}
{
}

LIBREMIDI_INLINE midi_in_emscripten::~midi_in_emscripten()
{
  // Close a connection if it exists.
  midi_in_emscripten::close_port();
}

LIBREMIDI_INLINE libremidi::API midi_in_emscripten::get_current_api() const noexcept
{
  return libremidi::API::WEBMIDI;
}

LIBREMIDI_INLINE bool midi_in_emscripten::open_port(int portNumber, std::string_view)
{
  auto& midi = webmidi_helpers::midi_access_emscripten::instance();

  if (portNumber < 0 || portNumber >= midi.input_count())
  {
    error<no_devices_found_error>(
        this->configuration, "midi_in_emscripten::open_port: no MIDI output sources found.");
    return false;
  }

  midi.open_input(portNumber, *this);
  portNumber_ = portNumber;
  return true;
}

LIBREMIDI_INLINE bool
midi_in_emscripten::open_port(const libremidi::input_port& p, std::string_view nm)
{
  return open_port(p.port, nm);
}

LIBREMIDI_INLINE bool midi_in_emscripten::open_virtual_port(std::string_view)
{
  warning(configuration, "midi_in_emscripten::open_virtual_port: unsupported.");
  return false;
}

LIBREMIDI_INLINE void midi_in_emscripten::close_port()
{
  auto& midi = webmidi_helpers::midi_access_emscripten::instance();

  midi.close_input(portNumber_, *this);
}

LIBREMIDI_INLINE void midi_in_emscripten::set_client_name(std::string_view)
{
  warning(configuration, "midi_in_emscripten::set_client_name: unsupported.");
}

LIBREMIDI_INLINE void midi_in_emscripten::set_port_name(std::string_view)
{
  warning(configuration, "midi_in_emscripten::set_port_name: unsupported.");
}

LIBREMIDI_INLINE int64_t midi_in_emscripten::absolute_timestamp() const noexcept
{
  return system_ns();
}

LIBREMIDI_INLINE void
midi_in_emscripten::on_input(double ts, unsigned char* begin, unsigned char* end)
{
  static constexpr timestamp_backend_info timestamp_info{
      .has_absolute_timestamps = true,
      .absolute_is_monotonic = true,
      .has_samples = false,
  };
  const auto to_ns = [=] { return 1e6 * ts; };

  m_processing.on_bytes({begin, end}, m_processing.timestamp<timestamp_info>(to_ns, 0));
}

}
#endif
