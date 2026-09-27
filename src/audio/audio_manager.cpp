#include "audio_manager.h"
#include "glib.h"
#include "glibmm/variant.h"
#include "gst/gstelement.h"
#include <cstdio>
#include <iostream>

AudioManager::AudioManager() {
  // Inicializar GStreamer (puedes pasar nullptr si no recibes argc/argv)
  if (!gst_is_initialized()) {
    gst_init(nullptr, nullptr);
  }
}

AudioManager::~AudioManager() { cleanupPipeline(); }

void AudioManager::cleanupPipeline() {
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);
    m_pipeline = nullptr;
  }
  m_currentState = PlaybackState::STOPPED;
}

void AudioManager::reset() { cleanupPipeline(); }

bool AudioManager::loadFile(const std::string &filePath) {
  cleanupPipeline();

  // 'playbin' gestiona automáticamente la detección de formato y decodificación
  m_pipeline = gst_element_factory_make("playbin", "audio-player");
  if (!m_pipeline) {
    std::cerr << "[AudioManager] Error: No se pudo crear el elemento playbin."
              << std::endl;
    return false;
  }

  // Convertir ruta local a URI válida
  std::string uri;
  if (filePath.rfind("file://", 0) == 0) {
    uri = filePath;
  } else {
    gchar *realUri = gst_filename_to_uri(filePath.c_str(), nullptr);
    if (realUri) {
      uri = realUri;
      g_free(realUri);
    } else {
      uri = "file://" + filePath;
    }
  }

  g_object_set(m_pipeline, "uri", uri.c_str(), NULL);

  // Configurar monitoreo de eventos en el Bus
  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
  gst_bus_add_watch(bus, &AudioManager::busCallback, this);
  gst_object_unref(bus);

  // Pre-cargar el stream (State Ready/Paused) para consultar metadata si es
  // necesario
  gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
  m_currentState = PlaybackState::PAUSED;

  return true;
}

void AudioManager::toggle_play(const Glib::VariantBase &parameter) {
  if (!m_pipeline) {
    g_print("No file loaded, nothing to toggle\n");
    return;
  }

  GstState current_state;
  GstState pending_state;
  GstStateChangeReturn ret =
      gst_element_get_state(m_pipeline, &current_state, &pending_state, 0);

  if (ret != GST_STATE_CHANGE_FAILURE) {
    if (current_state == GST_STATE_PAUSED) {
      g_print("Music paused -> playing now!\n");
      play();
    } else {
      g_print("Music playing -> paused!\n");
      pause();
    }
  }
}

void AudioManager::play() {
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    m_currentState = PlaybackState::PLAYING;
  }
}

void AudioManager::pause() {
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
    m_currentState = PlaybackState::PAUSED;
  }
}

void AudioManager::stop() {
  if (m_pipeline) {
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    m_currentState = PlaybackState::STOPPED;
  }
}

bool AudioManager::seek(int64_t seconds) {
  if (!m_pipeline)
    return false;

  gint64 targetNanoseconds = seconds * GST_SECOND;

  // Se realiza el seek en nanosegundos con flags de flush para respuesta rápida
  gboolean success = gst_element_seek_simple(
      m_pipeline, GST_FORMAT_TIME,
      static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
      targetNanoseconds);

  return success != 0;
}

int64_t AudioManager::getPositionSeconds() const {
  if (!m_pipeline)
    return 0;

  gint64 currentPosition = 0;
  if (gst_element_query_position(m_pipeline, GST_FORMAT_TIME,
                                 &currentPosition)) {
    return currentPosition / GST_SECOND;
  }
  return 0;
}

int64_t AudioManager::getDurationSeconds() const {
  if (!m_pipeline)
    return 0;

  gint64 duration = 0;
  if (gst_element_query_duration(m_pipeline, GST_FORMAT_TIME, &duration)) {
    return duration / GST_SECOND;
  }
  return 0;
}

gboolean AudioManager::busCallback(GstBus *bus, GstMessage *msg,
                                   gpointer data) {
  auto *manager = static_cast<AudioManager *>(data);

  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_EOS:
    manager->m_currentState = PlaybackState::STOPPED;
    if (manager->m_eosCallback) {
      manager->m_eosCallback();
    }
    break;

  case GST_MESSAGE_ERROR: {
    GError *err = nullptr;
    gchar *debugInfo = nullptr;
    gst_message_parse_error(msg, &err, &debugInfo);

    std::cerr << "[AudioManager] Error GStreamer: " << err->message
              << std::endl;

    g_clear_error(&err);
    g_free(debugInfo);
    manager->stop();
    break;
  }
  default:
    break;
  }
  return TRUE;
}
