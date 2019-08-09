#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <winhttp.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_windows.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <algorithm>
#include <string>
using namespace std;

static bool already_open = false;
static int pcount = 0;
static string lyrics_data;
static string cover_url;

std::string GiveUrl(std::string title);

void OpenBrowser(std::string title);

void SplitSongArtist(std::string title, std::string &artist, std::string &song);

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

HWND FindSpotify();

string GetLyrics(string song);

void GetCover(string filename);

typedef struct Color {
	int r = 0, g = 0, b = 0;
} Color;
typedef struct fColor {
	float r = 0, g = 0, b = 0;
} fColor;

typedef struct Theme {
	Color background, font;
} Theme;


Color *border_color(ALLEGRO_BITMAP* cover);