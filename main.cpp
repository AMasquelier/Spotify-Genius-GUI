#include <windows.h>
#include <string>
#include <iostream>
#include <winhttp.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_windows.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <algorithm>
#include <vector>

#pragma comment(lib, "winhttp.lib")
#include "timer.h"

using namespace std;

string GiveUrl(string title)
{
	string cmd = "";

	int pos = 0; // Remove "remastered" stuff
	if ((pos = title.find("[")) != -1)
	{
		int end = title.find("]");
		if (end != -1)
			title.erase(title.begin() + pos, title.begin() + end + 1);
		while (title[pos - 1] == ' ')
			title.erase(title.begin() + pos - 1);
	}

	pos = 0; // Remove "feat" stuff
	if ((pos = title.find("(feat")) != -1)
	{
		int end = title.find(")");
		if (end != -1)
			title.erase(title.begin() + pos, title.begin() + end + 1);
		while (title[pos - 1] == ' ')
			title.erase(title.begin() + pos - 1);
	}

	int last = title.length();
	if (title.find(" - ") != title.rfind(" - "))
		last = title.rfind(" - ");

	for (int i = 0; i < last; i++)
	{
		if (title[i] == ' ' || title[i] == '/') cmd += '-';
		else if (title[i] == '-') cmd.pop_back();
		else if (title[i] == '!' || title[i] == '.' || title[i] == '?' || title[i] == '\'' || title[i] == '?' || title[i] == ',' || title[i] == ';' || title[i] == '(' || title[i] == ')');
		else cmd += title[i];
	}
	return cmd + "-lyrics";
}

void OpenBrowser(string title)
{
	ShellExecute(NULL, "open", ("https://genius.com/" + GiveUrl(title)).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void SplitSongArtist(string title, string &artist, string &song)
{
	int pos = 0;
	if ((pos = title.find("[")) != -1)
	{
		int end = title.find("]");
		if (end != -1)
			title.erase(title.begin() + pos, title.begin() + end + 1);
		while (title[pos - 1] == ' ')
			title.erase(title.begin() + pos - 1);
	}

	int last = title.length();
	if (title.find(" - ") != title.rfind(" - "))
		last = title.rfind(" - ");
	title.erase(title.begin() + last, title.end());

	artist = title.substr(0, title.find(" - "));
	song = title.substr(title.find(" - ") + 2, title.length() - 1);
}

HWND spotify_hwnd;
bool already_open = false;
int pcount = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char class_name[80];
	char title[80];
	DWORD dwProcessId;
	GetClassName(hwnd, class_name, sizeof(class_name));
	GetWindowText(hwnd, title, sizeof(title));

	GetWindowThreadProcessId(hwnd, &dwProcessId);

	HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
	
	if (proc)
	{
		LPSTR path = new CHAR[MAX_PATH];
		DWORD charsCarried = MAX_PATH;
		BOOL RES = QueryFullProcessImageNameA(proc, NULL, path, &charsCarried);
		string filename = path;
		if (string(title) == "Genius") pcount++;
		if (pcount > 1) already_open = true;
		

		if (spotify_hwnd == NULL && filename.find("Spotify.exe") != string::npos)
		{
			string Title = title;
			
			if (Title != "" && string(class_name) == "Chrome_WidgetWin_0")
			{
				spotify_hwnd = hwnd;
			}
		}
		delete path;
		CloseHandle(proc);
	}

	return TRUE;
}

HWND FindSpotify()
{
	HWND hwnd = GetForegroundWindow(); 

	EnumWindows(EnumWindowsProc, 0);

	return hwnd;
}

string GetLyrics(string song)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession)
		hConnect = WinHttpConnect(hSession, L"genius.com",
			INTERNET_DEFAULT_HTTPS_PORT, 0);

	song = "/" + GiveUrl(song);
	std::wstring stemp = std::wstring(song.begin(), song.end());
	LPCWSTR url = stemp.c_str();

	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", url,
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	string data = "";
	
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			// Allocate space for the buffer.
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else
					data += string(pszOutBuffer);

				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

	bool found = false;
	bool instrumental = false;
	int beg = data.find("<div class=\"lyrics\""), end = data.find("</div>", beg);
	if (beg != -1 && end != -1)
	{
		if (data.find("[Instrumental]") != -1) instrumental = true;
		data = data.substr(beg, end - beg);
		// Clean lyrics string
		int pos = 0;
		while ((pos = data.find("<", pos)) != -1)
		{
			data.erase(data.begin() + pos, data.begin() + data.find(">", pos) + 1);
		}
		pos = 0;
		while ((pos = data.find("[", pos)) != -1)
		{
			data.erase(data.begin() + pos, data.begin() + data.find("]", pos) + 1);
		}
		pos = 0;
		while ((pos = data.find("\n\n\n", pos)) != -1)
		{
			data.erase(data.begin() + pos, data.begin() + pos + 1);
		}
		while (data[0] == '\n' || data[0] == ' ' || data[0] == '\t')
			data.erase(data.begin());

		while (data[data.length()-1] == '\n' || data[data.length() - 1] == ' ' || data[data.length() - 1] == '\t')
			data.erase(data.end()-1, data.end());

		found = true;
	}
	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	if (instrumental) data = "Instrumental";
	if (found)
		return data;
	else
		return "Lyrics not found";
}


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
}

typedef struct Color {
	int r = 0, g = 0, b = 0;
} Color;

typedef struct Theme {
	Color background, font;
} Theme;

int main()
{
	FindSpotify();
	Clock framerate_timer; framerate_timer.start();
	double framerate = 60.0;
	char wnd_title[256];
	string title, last_title;
	string lyrics;
	string song_title, artist;
	bool ls = false, s = false;
	bool open_genius = false;
	bool Keep = true;

	Init();
	al_clear_to_color(al_map_rgb(255, 0, 0));

	HWND window = al_get_win_window_handle(display);
	bool disp_win = !already_open;
	float alpha_channel = 0;

	Clock topmost;
	ALLEGRO_FONT* tfont = al_load_font("Roboto-Light.ttf", 28, 0);
	ALLEGRO_FONT* lfont = al_load_font("Roboto-Light.ttf", 20, 0);

	ALLEGRO_MOUSE_STATE mouse;
	int win_height = 120, lyrics_height = 0;


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

	float scroll_y = 100;
	bool lclic[2] = { false, false };
	bool on_app = false;
	POINT mouse_pos;
	int win_x = 1400, win_y = 30;

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
				FindSpotify();
			}
			else
			{
				framerate = 10.0;
				// Event
				ls = s;
				if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_F2) & 0x8000))
					Keep = false;
				else if (GetKeyState(VK_F2) & 0x8000)
					s = true;
				else
					s = false;
				
				
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
					if (mouse_pos.x > win_x + 56 && mouse_pos.x < win_x + 88 && mouse_pos.y > win_y + 80 && mouse_pos.y < win_y + 112)
					{
						on_play = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
						}
					}
					//skipbutton
					if (mouse_pos.x > win_x + 96 && mouse_pos.x < win_x + 120 && mouse_pos.y > win_y + 88 && mouse_pos.y < win_y + 112)
					{
						on_skip = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
						}
					}
					//uskipbutton
					if (mouse_pos.x > win_x + 30 && mouse_pos.x < win_x + 54 && mouse_pos.y > win_y + 88 && mouse_pos.y < win_y + 112)
					{
						on_uskip = true;
						if (lclic[0] && !lclic[1])
						{
							keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
						}
					}
					//uskipbutton
					if (mouse_pos.x > win_x + 400 && mouse_pos.x < win_x + 432 && mouse_pos.y > win_y + 80 && mouse_pos.y < win_y + 112)
					{
						on_theme = true;
						if (lclic[0] && !lclic[1])
						{
							act_theme++;
							act_theme %= themes.size();
							background = themes[act_theme].background;
							font_color = themes[act_theme].font;
						}
					}
				}
				if (lyrics_height > 700) // Scroll
				{
					if (mouse.z > 0)
					{
						mouse.z = 0;
						al_set_mouse_z(0);
					}

					if (mouse.z < (-lyrics_height + 580) / 100.0)
					{
						mouse.z = (-lyrics_height + 580) / 100.0;
						al_set_mouse_z((-lyrics_height + 580) / 100.0);
					}
					scroll_y = -mouse.z * 100;
					if (scroll_y > lyrics_height - 680) scroll_y = lyrics_height - 680;
				}
				
				if (!on_app && win_height != 120)
				{
					win_height = 120;
					al_resize_display(display, 500, 120);
				}
				
				GetWindowText(spotify_hwnd, wnd_title, sizeof(wnd_title));

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
					ShowWindow(window, SW_SHOW);
					SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					if (lyrics == "Lyrics not found") lyrics_height = 0;
					scroll_y = 0;
					mouse.z = 0;
					al_set_mouse_z(0);
					topmost.start();
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
							al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 20, -scroll_y + 134 + 24 * i, 0, lyrics.substr(pos, end_line - pos).c_str());
							pos = end_line + 1;
							i++;
						}
						al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 20, -scroll_y + 134 + 24 * i, 0, lyrics.substr(pos, lyrics.length() - pos).c_str());
					}
					// Header
					al_draw_filled_rectangle(0, 0, 500, 120, al_map_rgb(background.r, background.g, background.b));
					if (tfont)
					{
						al_draw_text(tfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 20, 20, 0, song_title.c_str());
						al_draw_text(lfont, al_map_rgb(font_color.r, font_color.g, font_color.b), 30, 50, 0, artist.c_str());
					}
					al_draw_line(20, 121, 480, 121, al_map_rgb(font_color.r, font_color.g, font_color.b), 1);
					
					// Buttons
					if(on_gbutton) al_draw_bitmap_region(gbutton, on_gbutton * 32, 0, 32, 32, 450, 80, 0);
					else		   al_draw_tinted_bitmap_region(gbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_gbutton * 32, 0, 32, 32, 450, 80, 0);
					al_draw_tinted_bitmap_region(playbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), playing * 64 + on_play * 32, 0, 32, 32, 56, 80, 0);
					al_draw_tinted_bitmap_region(skipbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_skip * 24, 0, 24, 24, 96, 88, 0);
					al_draw_tinted_bitmap_region(uskipbutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255), on_uskip * 24, 0, 24, 30, 24, 88, 0);
					al_draw_tinted_bitmap_region(themebutton, al_premul_rgba(font_color.r, font_color.g, font_color.b, 255),  on_theme * 24, 0, 24, 24, 408, 86, 0);
					if (on_app)
					{
						for (int i = 0; i < 10; i++)
							al_draw_line(0, 122 + i, 500, 122 + i, al_premul_rgba(background.r, background.g, background.b, 255 - i * 255 / 10.0), 1);
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
	Exit();
	return 0;
}