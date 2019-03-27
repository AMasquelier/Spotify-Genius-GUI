#include <windows.h>
#include <string>
#include <iostream>
#include <winhttp.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_windows.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <algorithm>
#pragma comment(lib, "winhttp.lib")
#include "timer.h"

using namespace std;

string GiveUrl(string title)
{
	string cmd = "";

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

	for (int i = 0; i < last; i++)
	{
		if (title[i] == ' ' || title[i] == '/') cmd += '-';
		else if (title[i] == '-') cmd.pop_back();
		else if (title[i] == '!' || title[i] == '.' || title[i] == '?' || title[i] == '\'' || title[i] == '?' || title[i] == ',' || title[i] == ';' || title[i] == '(' || title[i] == ')');
		else cmd += title[i];
	}
	return cmd + "-lyrics";
}

string Makecmd(string title)
{
	return ("start https://genius.com/" + GiveUrl(title));
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
	int beg = data.find("<div class=\"lyrics\""), end = data.find("</div>", beg);
	if (beg != -1 && end != -1)
	{
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
	display = al_create_display(500, 320);
	if (!display)
		cout << "error : display creation" << endl;

	al_init_font_addon();
	if (!al_init_ttf_addon())
		cout << "error : al_init_ttf_addon()" << endl;

	if (!al_init_primitives_addon())
		cout << "error : al_primitives_addon()" << endl;

	al_set_window_position(display, 1400, 30);
	al_set_window_title(display, "Genius");
}

int main()
{
	FindSpotify();
	Clock framerate_timer; framerate_timer.start();
	double framerate = 60.0;
	char wnd_title[256];
	string title, last_title;
	string lyrics;
	bool ls = false, s = false;
	bool open_genius = false;
	bool Keep = true;

	Init();
	al_clear_to_color(al_map_rgb(255, 0, 0));

	HWND window = al_get_win_window_handle(display);
	bool disp_win = false;
	float alpha_channel = 0;

	Clock topmost;
	ALLEGRO_FONT* tfont = al_load_font("GeosansLight.ttf", 28, 0);
	ALLEGRO_FONT* lfont = al_load_font("GeosansLight.ttf", 20, 0);

	while (Keep)
	{
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
				framerate = 20.0;
				// Event
				ls = s;
				if ((GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_F2) & 0x8000))
					Keep = false;
				else if (GetKeyState(VK_F2) & 0x8000)
					s = true;
				else
					s = false;

				if (wnd_title != nullptr) last_title = wnd_title;
				GetWindowText(spotify_hwnd, wnd_title, sizeof(wnd_title));

				if (topmost.duration() > 3000000 && disp_win) // Only display 3s
				{
					ShowWindow(window, SW_HIDE);
					disp_win = false;
				}
				if (string(wnd_title).length() == 0) // If spotify is closed
					spotify_hwnd = NULL;

				if (s && !ls && title != "") // If pushed f2
				{
					system(Makecmd(title).c_str());
					open_genius = false;
				}
				if (string(wnd_title) != last_title && string(wnd_title) != "Spotify")
				{
					title = wnd_title;
					disp_win = true;
					lyrics = GetLyrics(title);
					int nb_lines = std::count(lyrics.begin(), lyrics.end(), '\n');

					ShowWindow(window, SW_SHOW);
					SetWindowPos(window, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					al_resize_display(display, 500, 100 + nb_lines * 24);
					topmost.start();
				}
				else if (string(wnd_title) != last_title && string(wnd_title) == "Spotify")
				{
				}

				// Display
				al_clear_to_color(al_map_rgba(40, 40, 40, int(alpha_channel)));

				if (tfont)
				{
					al_draw_text(tfont, al_map_rgb(230, 230, 230), 20, 10, 0, title.c_str());
				}
				if (disp_win) 
					al_draw_line(20, 50, 480, 50, al_map_rgb(230, 230, 230), 1);
				if (lfont)
				{
					int pos = 0, end_line = 0;
					int i = 0;
					while ((end_line = lyrics.find("\n", pos)) != -1)
					{
						al_draw_text(lfont, al_map_rgb(230, 230, 230), 20, 64 + 24 * i, 0, lyrics.substr(pos, end_line - pos).c_str());
						pos = end_line + 1;
						i++;
					}
				}
				
				al_flip_display();
			}
			
		}
		else Clock::sleep(1000.0 / framerate - framerate_timer.duration() * 0.001);
	}
	Exit();
	return 0;
}