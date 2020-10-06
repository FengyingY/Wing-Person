# (TODO: your game's title)

Author: (TODO: your name)

## Design: 

(TODO: In two sentences or fewer, describe what is new and interesting about your game.)

## Text Drawing: 

In this game, texts are rendered at runtime. The process flow is:

1. Load text data from asset, as `std::string`, using UTF-8 format.
2. At runtime, load the TrueType fonts. Process using harfbuzz and FreeType, get an array of glyphs (in bitmap representation)
3. At runtime, draw the bitmap as an OpenGL texture.

HiDPI screen support is implemented. Subpixel rendering is not supported yet (hopefully all the monitors will go 4K+ in future, in which case subpixel rendering will be obsolete).

Line breaking is handled manually at this moment, because handling linebreaks in an i18n-compatible way takes another dedicated library. 

Font fallback is not supported now. As a result, in order to display non-latin text, we need to manually specify corresponding fonts.


## Screen Shot:

![Screen Shot](screenshot.png)

## How To Play:

(TODO: describe the controls and (if needed) goals/strategy.)

## Sources: 

- Music: Past Sadness by Kevin MacLeod
Link: https://incompetech.filmmusic.io/song/5024-past-sadness
License: http://creativecommons.org/licenses/by/4.0/

- Fonts:
  - [IBM Plex Mono](https://fonts.google.com/featured/Plex)
  - [Computer Modern](https://en.wikipedia.org/wiki/Computer_Modern)

The harfbuzz and freetype code is based on https://www.gedge.ca/code/opengl_text_rendering.cpp.txt

This game was built with [NEST](NEST.md).

