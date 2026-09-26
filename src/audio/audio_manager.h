#pragma once

#include "glibmm/variant.h"
#include <functional>
#include <gst/gst.h>
#include <string>

enum class PlaybackState { STOPPED, PLAYING, PAUSED };

class AudioManager {
public:
  AudioManager();
  ~AudioManager();

  // Métodos principales de control para la UI
  bool loadFile(const std::string &filePath);
  void toggle_play(const Glib::VariantBase &parameter);
  void play();
  void pause();
  void stop();
  bool seek(int64_t seconds);

  // Obtención de información para sliders o etiquetas de tiempo
  int64_t getPositionSeconds() const;
  int64_t getDurationSeconds() const;
  PlaybackState getState() const { return m_currentState; }

  // Registrar callback opcional para eventos (Fin de archivo, Errores)
  using EOSCallback = std::function<void()>;
  void setEOSCallback(EOSCallback callback) { m_eosCallback = callback; }

private:
  GstElement *m_pipeline{nullptr};
  PlaybackState m_currentState{PlaybackState::STOPPED};
  EOSCallback m_eosCallback{nullptr};

  void cleanupPipeline();
  static gboolean busCallback(GstBus *bus, GstMessage *msg, gpointer data);
};
