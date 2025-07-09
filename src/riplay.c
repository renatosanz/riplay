#include <gtk/gtk.h>
#include <gtk_utils.h>
#include <riplay.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <mpg123.h>

GtkApplication *app;
GtkWindow *win;

#define PROJECT_NAME "riplay"
char *filename = NULL;

static int on_activate(GApplication *app, char *hint) {
  win = openSingleWindow(app, filename);
  return 0;
}

static void on_open_file(GApplication *app, GFile **files, int n_files,
                         char *hint) {
  if (n_files == 1) {
    on_activate(app, filename);
  } else {
    for (int i = 0; i < n_files; i++) {
      char *path = g_file_get_path(files[i]);
      printf("Abriendo archivo: %s\n", path);
      // Aquí tu lógica para reproducir el archivo
      g_free(path);
    }
  }
}

int main_gtk(int argc, char **argv) {
  gtk_init();

  if (argc == 2) {
    filename = argv[1];
    g_print("%s", filename);
  }

  app = gtk_application_new("org.riprtx.riplay", G_APPLICATION_HANDLES_OPEN);
  gtk_window_set_default_icon_name(PROJECT_NAME);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  g_signal_connect(app, "open", G_CALLBACK(on_open_file), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

int decodeMP3(const char *filename) {
  mpg123_init();

  mpg123_handle *mh = mpg123_new(NULL, NULL);
  if (mh == NULL) {
    fprintf(stderr, "Error al crear el manejador mpg123\n");
    return -1;
  }

  if (mpg123_open(mh, filename) != MPG123_OK) {
    fprintf(stderr, "Error al abrir el archivo MP3\n");
    mpg123_delete(mh);
    return -1;
  }

  // Obtener información del archivo
  long rate;
  int channels, encoding;
  mpg123_getformat(mh, &rate, &channels, &encoding);

  printf("Archivo MP3:\n");
  printf("  Tasa de muestreo: %ld Hz\n", rate);
  printf("  Canales: %d\n", channels);
  printf("  Formato de codificación: %d\n", encoding);

  // Buffer para decodificación
  unsigned char *buffer;
  size_t buffer_size = mpg123_outblock(mh);
  buffer = (unsigned char *)malloc(buffer_size);

  size_t done;
  int err;
  do {
    err = mpg123_read(mh, buffer, buffer_size, &done);
    // Aquí puedes procesar los datos decodificados en 'buffer'
    // printf("Decodificados %zu bytes\n", done);
  } while (err == MPG123_OK && done > 0);

  free(buffer);
  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  return 0;
}

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define FORMAT AUDIO_S16SYS

int playMP3(const char *filename) {
  // Inicializar mpg123
  mpg123_init();

  mpg123_handle *mh = mpg123_new(NULL, NULL);
  if (mh == NULL) {
    fprintf(stderr, "Error al crear el manejador mpg123\n");
    return -1;
  }

  // Abrir el archivo MP3
  if (mpg123_open(mh, filename) != MPG123_OK) {
    fprintf(stderr, "Error al abrir el archivo MP3\n");
    mpg123_delete(mh);
    return -1;
  }

  // Configurar SDL2 para audio
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
    mpg123_close(mh);
    mpg123_delete(mh);
    return -1;
  }

  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;

  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.format = FORMAT;
  want.channels = CHANNELS;
  want.samples = 4096;
  want.callback = NULL;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  if (dev == 0) {
    fprintf(stderr, "Error al abrir dispositivo de audio: %s\n",
            SDL_GetError());
    SDL_Quit();
    mpg123_close(mh);
    mpg123_delete(mh);
    return -1;
  }

  // Configurar decodificador
  long rate;
  int channels, encoding;
  if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
    fprintf(stderr, "Error al obtener formato del MP3\n");
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    mpg123_close(mh);
    mpg123_delete(mh);
    return -1;
  }

  // Iniciar reproducción
  SDL_PauseAudioDevice(dev, 0);

  // Buffer para audio decodificado
  unsigned char *buffer;
  size_t buffer_size = mpg123_outblock(mh);
  buffer = (unsigned char *)malloc(buffer_size);
  if (buffer == NULL) {
    fprintf(stderr, "Error al asignar memoria para el buffer\n");
    SDL_CloseAudioDevice(dev);
    SDL_Quit();
    mpg123_close(mh);
    mpg123_delete(mh);
    return -1;
  }

  // Decodificar y reproducir
  size_t done;
  int err;
  do {
    err = mpg123_read(mh, buffer, buffer_size, &done);
    SDL_QueueAudio(dev, buffer, done);
    SDL_Delay(10); // Pequeña pausa para evitar sobrecarga
  } while (err == MPG123_OK && done > 0);

  // Esperar a que termine la reproducción
  while (SDL_GetQueuedAudioSize(dev) > 0) {
    SDL_Delay(100);
  }

  // Limpieza
  free(buffer);
  SDL_CloseAudioDevice(dev);
  SDL_Quit();
  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  return 0;
}

int main(int argc, char **argv) {
  const char *filename =
      NULL; // Replace "example.mp3" with the actual filename.

  if (argc == 2) {
    filename = argv[1];
    g_print("%s", filename);
  }

  if (filename && playMP3(filename) == 0) {
    printf("MP3 file read successfully.\n");
  } else {
    printf("Failed to read MP3 file.\n");
  }

  return 0;
}
