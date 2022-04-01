#include "utils.h"

#pragma comment(lib, "winhttp.lib")
#include "timer.h"
#include "math.h"

ALLEGRO_DISPLAY *display;
void Exit()
{
	al_destroy_display(display);
}

void Init()
{

	if (!al_init())
		cout << "error : al_init()" << endl;

	al_set_new_display_flags(ALLEGRO_NOFRAME);
	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_SUGGEST);
	display = al_create_display(500, 120);
	if (!display)
		cout << "error : display creation" << endl;

	al_init_font_addon();
	if (!al_init_ttf_addon())
		cout << "error : al_init_ttf_addon()" << endl;

	if (!al_init_primitives_addon())
		cout << "error : al_primitives_addon()" << endl;

	if (!al_install_mouse())
		cout << "error : al_install_mouse()" << endl;

	if (!al_init_image_addon())
		cout << "error : al_init_image_addon()" << endl;

	al_set_window_position(display, 1400, 30);
	al_set_window_title(display, "Genius");

	HWND winhandle;
	HICON icon;

	icon = LoadIcon(GetModuleHandle(NULL), L"MAINICON");
	if (icon) {
			
		winhandle = al_get_win_window_handle(display);
		SetClassLongPtr(winhandle, GCLP_HICON, (LONG_PTR)icon);
		SetClassLongPtr(winhandle, GCLP_HICONSM, (LONG_PTR)icon);
	}
}

HWND spotify_hwnd;

int main()
{
	FindSpotify();
	Clock framerate_timer; framerate_timer.start();
	double framerate = 60.0;
	char *wnd_title = new char[256];;
	string title, last_title;
	string lyrics;
	string song_title, artist;
	bool ls = false, s = false, shift = false;
	bool open_genius = false;
	bool Keep = true;
	float mzx = 0, mzy = 0;

	Init();
	al_clear_to_color(al_map_rgb(255, 0, 0));


	HWND window = al_get_win_window_handle(display);
	bool disp_win = !already_open;
	float alpha_channel = 0;

	Clock topmost;
	ALLEGRO_FONT* tfont = al_load_font("Roboto-Light.ttf", 28, 0);
	ALLEGRO_FONT* lfont = al_load_font("Roboto-Light.ttf", 20, 0);

	ALLEGRO_MOUSE_STATE mouse;
	int win_height = 120, lyrics_height = 0, lyrics_width = 0;


	vector<Theme> themes;
	int act_theme = 0;


	
	themes.push_back({ { 40, 40, 40 }, { 230, 230, 230 } }); // Dark
	themes.push_back({ { 240, 240, 240 }, { 20, 20, 20 } }); // Light
	themes.push_back({ { 40, 45, 60 }, { 240, 220, 80 } }); // Lemonade
	themes.push_back({ { 20, 25, 35 }, { 215, 170, 190 } }); // Obsidian
	themes.push_back({ { 34, 34, 51 }, { 170, 204, 255 } }); // Deep blue
	themes.push_back({ { 45, 47, 51 }, { 230, 230, 230 } }); // Charcoal
	themes.push_back({ { 150, 64, 60 }, { 230, 230, 230 } }); // Rust
	themes.push_back({ { 37, 120, 133 }, { 230, 230, 230 } }); // Salted sea
	themes.push_back({ { 229, 221, 200 }, { 39, 23, 28 } }); // Old 
	themes.push_back({ { 0, 0, 0 }, { 255, 255, 255 } }); // Custom 

	Color background = themes[act_theme].background;
	Color font_color = themes[act_theme].font;

	ALLEGRO_BITMAP*gbutton = al_load_bitmap("Button.png");
	bool on_gbutton = false;
	ALLEGRO_BITMAP*playbutton = al_load_bitmap("Play.png");
	bool on_play = false;
	bool playing = false;
	ALLEGRO_BITMAP*skipbutton = al_load_bitmap("Skip.png");
	bool on_skip = false;
	ALLEGRO_BITMAP*uskipbutton = al_load_bitmap("Uskip.png");
	bool on_uskip = false;
	ALLEGRO_BITMAP*themebutton = al_load_bitmap("Theme.png");
	bool on_theme = false;

	ALLEGRO_BITMAP*cover = al_load_bitmap("cover.png");

	float scroll_x = 0, scroll_y = 100;
	bool lclic[2] = { false, false };
	bool on_app = false;
	POINT mouse_pos;
	int win_x = 1400, win_y = 30;
	int x_shift = 0;

	act_theme = themes.size()-1;
	cout << act_theme << endl;
	ShowWindow(window, SW_HIDE);
	while (Keep)
	{
		Loop_begining:
		if (framerate_timer.duration() >= 1000000.0 / framerate)
		{
			framerate_timer.start();
			// Search Spotify window
			if (spotify_hwnd == NULL)
			{
				framerate = 1.0;
				spotify_hwnd = FindSpotify();
			}
			else
			{
				framerate = 10.0;
				// Event
				ls = s;
				if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_F2) & 0x8000))
					Keep = false;
				else s = (GetKeyState(VK_F2) & 0x8000);

				shift = (GetKeyState(VK_SHIFT) & 0x8000);

				if (cover) x_shift = 105;
				else x_shift = 0;

				
				al_get_mouse_state(&mouse);
				GetCursorPos(&mouse_pos);
				on_gbutton = false;
				on_play = false;
				on_app = false;
				on_skip = false;
				on_uskip = false;
				on_theme = false;
				lclic[1] = lclic[0];
				lclic[0] = mouse.buttons & 1;
				if (mouse_pos.x > win_x && mouse_pos.x < win_x + 500 && mouse_pos.y > win_y && mouse_pos.y < win_y + win_height)
				{
					topmost.start();
					on_app = true;
					if (win_height != min(800, 120 + lyrics_height))
					{
						win_height = min(800, 120 + lyrics_height);
						al_resize_display(display, 500, win_height);
					}
					
					//gbutton
					if (mouse_pos.x > win_x + 450 && mouse_pos.x < win_x + 482 && mouse_pos.y > win_y + 80 && mouse_pos.y < win_y + 112)
					{
						on_gbutton = true;
						if(lclic[0] && !lclic[1])
							OpenBrowser(title);
					}
					//playbutton
					if (mouse_pos.x > win_x + x_shift + 56 && mouse_pos.x < win_x + x_shift + 88 && mouse_pos.y > win_y + 80 && mouse_pos.y < win_y + 112)
					{
						on_play = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
						}
					}
					//skipbutton
					if (mouse_pos.x > win_x + x_shift + 96 && mouse_pos.x < win_x + x_shift + 120 && mouse_pos.y > win_y + 88 && mouse_pos.y < win_y + 112)
					{
						on_skip = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
						}
					}
					//uskipbutton
					if (mouse_pos.x > win_x + x_shift + 30 && mouse_pos.x < win_x + x_shift + 54 && mouse_pos.y > win_y + 88 && mouse_pos.y < win_y + 112)
					{
						on_uskip = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
						}
					}
					//themebutton
					if (mouse_pos.x > win_x + 400 && mouse_pos.x < win_x + 432 && mouse_pos.y > win_y + 80 && mouse_pos.y < win_y + 112)
					{
						on_theme = true;
						if (lclic[0] && !lclic[1])
						{
							act_theme++;
							act_theme %= themes.size();
							if (act_theme == themes.size()-1) // Adaptative theme
							{
								if (!cover)
								{
									act_theme++;
									act_theme %= themes.size();
								}
								background = themes[act_theme].background;
								font_color = themes[act_theme].font;
							}
							else
							{
								background = themes[act_theme].background;
								font_color = themes[act_theme].font;
							}
						}
					}
				}
				if (lyrics_height > 690) // Scroll
				{
					mzy += mouse.z*100;
					mzy = fmin(0, mzy);
					mzy = fmax(-lyrics_height + 580, mzy);

					scroll_y = -mzy;
					if (scroll_y > lyrics_height - 680) scroll_y = lyrics_height - 680;
					
				}
				if (shift && lyrics_width > 500) {
					mzx += mouse.z * 100;
					mzx = fmin(0, mzx);
					mzx = fmax(-lyrics_width + 500, mzx);
					scroll_x = -mzx;

					if (scroll_x > lyrics_width - 500) scroll_x = lyrics_width - 500;
				}
				mouse.z = 0;
				al_set_mouse_z(0);

				
				if (!on_app && win_height != 120)
				{
					win_height = 120;
					al_resize_display(display, 500, 120);
				}
				
				GetWindowTextA(spotify_hwnd, wnd_title, 256);

				if (topmost.duration() > 3000000 && disp_win) // Only display 3s
				{
					ShowWindow(window, SW_HIDE);
					disp_win = false;
				}
				if (string(wnd_title).length() == 0) // If spotify is closed
				{
					spotify_hwnd = NULL;
					ShowWindow(window, SW_HIDE);
					disp_win = false;
					goto Loop_begining;
				}
				if (s && !ls && title != "") // If pushed f2
				{
					//OpenBrowser(title);
					topmost.start();
					ShowWindow(window, SW_SHOW);
					SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					disp_win = true;
				}
				// Detect if it's paused or playing
				if (string(wnd_title) == "Spotify" || string(wnd_title) == "Spotify Premium")
				{
					playing = false;
				}
				else playing = true;

				if (string(wnd_title) != last_title && string(wnd_title) != "Spotify" && string(wnd_title) != "Spotify Premium")
				{
					title = wnd_title;
					lyrics = GetLyrics(title);
					
					last_title = wnd_title;
					title = wnd_title;
					disp_win = true;
					int nb_lines = std::count(lyrics.begin(), lyrics.end(), '\n');
					
					

					SplitSongArtist(title, artist, song_title);
					

					lyrics_height = 70 + nb_lines * 24, 700;

					int pos = 0, end_line = 0;
					int i = 0, m = 0;

					while ((end_line = lyrics.find("\n", pos)) != -1)
					{
						m = max(m, lyrics.substr(pos, end_line - pos).size());
						pos = end_line + 1;
					}
					lyrics_width = max(500, m * 10);
					cout << lyrics_width << endl;

					ShowWindow(window, SW_SHOW);
					SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					if (lyrics == "Lyrics not found") lyrics_height = 0;
					scroll_y = 0;
					mouse.z = 0;
					al_set_mouse_z(0);
					topmost.start();

					if (cover) al_destroy_bitmap(cover);
					al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR);
					cover = al_load_bitmap("cover.png");
					if (cover)
					{
						remove("cover.png");
						Color* new_theme = border_color(cover);

						themes[themes.size()-1].background = new_theme[0];
						if (new_theme[1].r != -1) themes[themes.size() - 1].font = new_theme[1];
						
						if (act_theme == themes.size() - 1)
						{
								
							background = themes[act_theme].background;
							font_color = themes[act_theme].font;
						}

						delete new_theme;
						
					}
					else if (act_theme == themes.size() - 1)
					{
						background = themes[0].background;
						font_color = themes[0].font;
					}

				}

				// Display
				if (disp_win)
				{
					al_clear_to_color(al_map_rgba(background.r, background.g, background.b, int(alpha_channel)));

					// Lyrics
					if (lfont)
					{
						int pos = 0, end_line = 0;
						int i = 0;
						
						while ((end_line = lyrics.find("\n", pos)) != -1)
						{
							
							al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 20 - scroll_x, -scroll_y + 134 + 24 * i, 0, lyrics.substr(pos, end_line - pos).c_str());
							pos = end_line + 1;
							i++;
						}
						al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 20- scroll_x, -scroll_y + 134 + 24 * i, 0, lyrics.substr(pos, lyrics.length() - pos).c_str());
					}
					// Header
					al_draw_filled_rectangle(0, 0, 500, 120, al_map_rgb(background.r, background.g, background.b));
					if (tfont)
					{
						al_draw_text(tfont, al_map_rgb(font_color.r, font_color.g, font_color.b), x_shift + 20, 10, 0, song_title.c_str());
						al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), x_shift + 30, 40, 0, artist.c_str());
						for (int i = 0; i < 20; i++)
							al_draw_line(490 - i, 0, 490 - i, 120, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 20.0), 1);
						al_draw_filled_rectangle(490, 0, 500, 120, al_map_rgb(background.r, background.g, background.b));
					}
					al_draw_line(20, 121, 480, 121, al_map_rgb(font_color.r, font_color.g, font_color.b), 1);
					
					if (cover)
					{
						al_draw_scaled_bitmap(cover, 0, 0, al_get_bitmap_width(cover), al_get_bitmap_height(cover), 20, 10, 100, 100, 0);
						for (int i = 0; i < 20; i++)
						{
							al_draw_line(120 - i, 10, 120 - i, 110, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 20.0), 1);
							//al_draw_line(20 + i, 10, 20 + i, 110, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
							//al_draw_line(20 + i, 10 + i, 120 - i, 10 + i, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
							//al_draw_line(20 + i, 110 - i, 120 - i, 110 - i, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
						}
					}
					// Buttons
					if(on_gbutton) al_draw_bitmap_region(gbutton, on_gbutton * 32, 0, 32, 32, 450, 80, 0);
					else		   al_draw_tinted_bitmap_region(gbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_gbutton * 32, 0, 32, 32, 450, 80, 0);
					al_draw_tinted_bitmap_region(playbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), playing * 64 + on_play * 32, 0, 32, 32, x_shift + 56, 80, 0);
					al_draw_tinted_bitmap_region(skipbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_skip * 24, 0, 24, 24, x_shift + 96, 88, 0);
					al_draw_tinted_bitmap_region(uskipbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_uskip * 24, 0, 24, 30, x_shift + 24, 88, 0);
					al_draw_tinted_bitmap_region(themebutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255),  on_theme * 24, 0, 24, 24, 408, 86, 0);
					if (on_app)
					{
						for (int i = 0; i < 10; i++)
							al_draw_line(0, 122 + i, 500, 122 + i, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
					}

					// Side
					if (on_app && lyrics_height > 0)
					{
						for (int i = 0; i < 20; i++)
							al_draw_line(0 + i, 121, 0 + i, win_height, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 20.0), 1);

						for (int i = 0; i < 20; i++)
							al_draw_line(500 - i, 121, 500 - i, win_height, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 20.0), 1);
						al_draw_filled_rectangle(0, win_height - 20, 500, win_height, al_map_rgb(background.r, background.g, background.b));
					}

					// Footer
					if (on_app && lyrics_height > 0)
					{
						for (int i = 0; i < 10; i++)
							al_draw_line(0, win_height - 20 - i, 500, win_height - 20 - i, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
						al_draw_filled_rectangle(0, win_height - 20, 500, win_height, al_map_rgb(background.r, background.g, background.b));
					}
					al_flip_display();
				}
			}
			
		}
		else Clock::sleep(1000.0 / framerate - framerate_timer.duration() * 0.001);
	}
	if (gbutton) al_destroy_bitmap(gbutton);
	if (playbutton) al_destroy_bitmap(playbutton);
	if (skipbutton) al_destroy_bitmap(skipbutton);
	if (uskipbutton) al_destroy_bitmap(uskipbutton);
	if (themebutton) al_destroy_bitmap(themebutton);
	if (cover) al_destroy_bitmap(cover);
	if (tfont) al_destroy_font(tfont);
	if (lfont) al_destroy_font(lfont);


	Exit();
	return 0;
}