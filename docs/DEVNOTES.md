# DEV NOTES

# Why dev notes?

Well this place is thinked as a long list of commands and tips while I develope this music player.

### COMMANDS

I like Zed Editor, not too much for it's IA features, but it's a cool editor, and I can tell GTK libs is really clunky, and to include it on the clangd context I found used this command:

```bash
pkg-config --cflags gtkmm-4.0 | tr ' ' '\n' | grep '^-I' | sed 's/-I/"/;s/$/",/'

# Output                                        [***]
"/usr/include/gtkmm-4.0",
"/usr/lib/gtkmm-4.0/include",
"/usr/include/pangomm-2.48",
...
```

This gives you a JSON like output of all the paths of the libs neeeded to run gtkmm effectively on clangd, you just need to create a .zed/settings.json like this:

```json
{
  "lsp": {
    "clangd": {
      "initialization_options": {
        "fallbackFlags": [
          // this is really important to give
          // access to clangd to the project files
          "-I/your/path/to/riplay/src",
          "-I/your/path/to/riplay/src/views",
          "-I/your/path/to/riplay/src/controllers",
          "-I/your/path/to/riplay/src/models",

          // All the paths of above command
          "-I/usr/include/gtkmm-4.0",
          "-I/usr/lib/gtkmm-4.0/include",
          "-I/usr/include/pangomm-2.48",
          ...
        ],
      },
    },
  },
}
```

With this configuration you will have every LSP commpletation and no errors for files not found

> ![NOTE]
> Paths related to the project needs to be updated if some dir/file is created
