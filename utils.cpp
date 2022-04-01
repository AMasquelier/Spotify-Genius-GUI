#include "utils.h"


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
	ShellExecuteA(NULL, "open", ("https://genius.com/" + GiveUrl(title)).c_str(), NULL, NULL, SW_SHOWNORMAL);
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

HWND sp_hwnd;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char class_name[80];
	char title[80];
	DWORD dwProcessId;
	GetClassNameA(hwnd, class_name, sizeof(class_name));
	GetWindowTextA(hwnd, title, sizeof(title));

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


		if (sp_hwnd == NULL && filename.find("Spotify.exe") != string::npos)
		{
			string Title = title;

			if (Title != "" && string(class_name) == "Chrome_WidgetWin_0")
			{
				sp_hwnd = hwnd;
			}
		}
		delete path;
		CloseHandle(proc);
	}

	return TRUE;
}

HWND FindSpotify()
{
	sp_hwnd = NULL;
	HWND hwnd = GetForegroundWindow();

	EnumWindows(EnumWindowsProc, 0);

	return sp_hwnd;
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


	// Find cover
	int beg = data.find("<meta content=\"https://images.genius.com/"), end = data.find("\" property=\"og:image\" />", beg);
	if (beg != -1 && end != -1)
	{
		cover_url = data.substr(beg, end - beg);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		GetCover(cover_url);
	}
	beg = data.find("<meta content=\"https://images.rapgenius.com/"); end = data.find("\" property=\"og:image\" />", beg);
	if (beg != -1 && end != -1)
	{
		cover_url = data.substr(beg, end - beg);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		GetCover(cover_url);
	}
	beg = data.find("<meta content=\"http://images.genius.com/"); end = data.find("\" property=\"og:image\" />", beg);
	if (beg != -1 && end != -1)
	{
		cover_url = data.substr(beg, end - beg);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		cover_url.erase(cover_url.begin(), cover_url.begin() + cover_url.find("/") + 1);
		GetCover(cover_url);
	}

	bool found = false;
	bool instrumental = false;
	beg = data.find("<div data-lyrics-container="); end = data.find("</div>", beg);

	if (beg != -1 && end != -1)
	{
		if (data.find("[Instrumental]") != -1) instrumental = true;
		data = data.substr(beg, end - beg);
		// Clean lyrics string
		int pos = 0;
		while ((pos = data.find("<", pos)) != -1)
		{
			if (data.find("br/>", pos) == pos + 1) {
				data.insert(pos, "\n");
				pos+=1;
			}
			data.erase(data.begin() + pos, data.begin() + data.find(">", pos) + 1);
		}
		pos = 0;
		while ((pos = data.find("&#x27", pos)) != -1)
		{
			data.erase(data.begin() + pos, data.begin() + data.find(";", pos) + 1);
			data.insert(pos, "'");
			pos += 1;
		}
		pos = 0;
		while ((pos = data.find("&quot", pos)) != -1)
		{
			data.erase(data.begin() + pos, data.begin() + data.find(";", pos) + 1);
			data.insert(pos, "'");
			pos += 1;
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

		while (data[data.length() - 1] == '\n' || data[data.length() - 1] == ' ' || data[data.length() - 1] == '\t')
			data.erase(data.end() - 1, data.end());

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
	lyrics_data = data;
	if (found)
		return data;
	else
		return "Lyrics not found";
}

void GetCover(string filename)
{
	FILE * pFile;
	pFile = fopen("cover.png", "w+b");

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
		hConnect = WinHttpConnect(hSession, L"images.genius.com",
			INTERNET_DEFAULT_HTTPS_PORT, 0);

	std::wstring stemp = std::wstring(filename.begin(), filename.end());
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
	if (bResults)
		do
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			pszOutBuffer = new char[dwSize + 1];

			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
					dwSize, &dwDownloaded))
				{
					printf("Error %u in WinHttpReadData.\n",
						GetLastError());
				}
				else
					fwrite(pszOutBuffer, (size_t)dwDownloaded, (size_t)1, pFile); 

				delete[] pszOutBuffer;
			}

		} while (dwSize > 0);

		fclose(pFile);
}

float relative_luminance(fColor c)
{
	return 0.2126 * c.r + 0.715 * c.g + 0.0722 * c.b;
}

float contrast_ratio(fColor a, fColor b)
{
	float c = (relative_luminance(a) + 0.05) / (relative_luminance(b) + 0.05);
	if (c < 1) return 1 / c;
	else return c;
}

float get_hue(fColor color)
{
	float R = color.r, G = color.g, B = color.b;
	float max = fmax(R, fmax(G, B)), min = fmin(R, fmin(G, B));
	float H = 0;
	if (max == min) return 0;
	if (max == R) H = 60 * (G - B) / (max - min);
	if (max == G) H = 60 * (B - R) / (max - min);
	if (max == B) H = 60 * (R - G) / (max - min);
	if (H < 0) return H + 360;
	
	return H;
}

float d_hue(float a, float b)
{
	if (a < b)
		return fmin(fabs(a - b), fabs(a + 360 - b));
	else 
		return fmin(fabs(a - b), fabs(b + 360 - a));
}

float get_value(fColor color)
{
	return fmax(color.r, fmax(color.g, color.b));
}

fColor improve_contrast(fColor background, fColor color)
{
	if (fabs(get_value(background) < 0.5))
	{
		while ((contrast_ratio(color, background) < 5 || fabs(get_value(background) - get_value(color)) < 0.7) && color.r < 0.9 && color.g < 0.9 && color.b < 0.9)
		{
			color.r += 0.05; color.g += 0.05; color.b += 0.05;
		}
	}
	else
	{
		while ((contrast_ratio(color, background) < 5 || fabs(get_value(background) - get_value(color)) < 0.5) && color.r > 0.05 && color.g > 0.05 && color.b > 0.05)
		{
			color.r -= 0.05; color.g -= 0.05; color.b -= 0.05;
		}
	}
	return color;
}

typedef struct Point {
	int x = 0, y = 0;
} Point;

Color *border_color(ALLEGRO_BITMAP* cover)
{
	ALLEGRO_COLOR color;
	ALLEGRO_LOCKED_REGION* lregion = al_lock_bitmap(cover, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	int w = al_get_bitmap_width(cover);
	int h = al_get_bitmap_height(cover);

	float r = 0, g = 0, b = 0;
	int n_sample = 100;

	std::vector<fColor> colors;
	std::vector<fColor> etalons;
	std::vector<int> counts;
	std::vector<Point> points;

	Color* ret = new Color[2];
	float contrast_limit = 2;

	// Check borders for background
	

	// Check some sample of the image to find the different colors in the image
	points.clear();
	for (int i = 0; i < w; i += w / n_sample)
		for (int j = 0; j < h; j += h / n_sample)
			points.push_back({ i, j });

	for (int i = 0; i < points.size(); i++)
	{
		color = al_get_pixel(cover, points[i].x, points[i].y);
		bool new_color = true;
		for (int i = 0; i < colors.size() && new_color; i++)
		{
			fColor c = { color.r, color.g, color.b };
			float contrast = contrast_ratio(c, etalons[i]);
			float hue1 = get_hue(c), hue2 = get_hue(etalons[i]);
			float dvalue = fabs(get_value(c) - get_value(etalons[i]));
			if (contrast < contrast_limit && d_hue(hue1, hue2) < 60 && dvalue < 0.3)
			{
				colors[i].r = (colors[i].r * counts[i] + color.r) / (counts[i] + 1);
				colors[i].g = (colors[i].g * counts[i] + color.g) / (counts[i] + 1);
				colors[i].b = (colors[i].b * counts[i] + color.b) / (counts[i] + 1);
				new_color = false;
				counts[i]++;
			}
		}
		if (new_color)
		{
			colors.push_back({ color.r, color.g, color.b });
			etalons.push_back({ color.r, color.g, color.b });
			counts.push_back(1);
		}
	}

	// Choose the most present color for the background
	int background = 0, font = 0;
	for (int i = 0; i < counts.size(); i++)
	{

		if (counts[i] > counts[background])
			background = i;
	}
	ret[0] = { int(colors[background].r * 255), int(colors[background].g * 255), int(colors[background].b * 255) };

	// Choose the best contrasting color for the font
	for (int i = 0; i < counts.size(); i++)
	{
		float dhue = fabs(get_hue(colors[background]) - get_hue(colors[i]));
		float dhue_act = fabs(get_hue(colors[background]) - get_hue(colors[font]));
		float contrast = contrast_ratio(colors[background], colors[i]);
		float contrast_act = contrast_ratio(colors[background], colors[font]);
		if ((counts[i] * contrast * dhue > counts[font] * contrast_act * dhue_act && i != background) || font == background)
			font = i;
	}

	colors[font] = improve_contrast(colors[background], colors[font]);

	
	if (background == font) ret[1] = { -1, -1, -1 };
	else ret[1] = { int(colors[font].r * 255), int(colors[font].g * 255), int(colors[font].b * 255) };

	al_unlock_bitmap(cover);
	return ret;

}