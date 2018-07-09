#include "winsgl.h"
#include "json.h"
#include "io.h"

int ratio[128][128];

LPWSTR widen(const char *src) {
	int rt;
	LPWSTR rs;

	if (src == NULL)return NULL;

	rt = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
	rs = (LPWSTR)malloc(rt * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, src, -1, rs, rt * sizeof(wchar_t));
	return rs;
}
char *shorten(const LPWSTR src) {
	int rt;
	char* rs;

	rt = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	rs = (char*)malloc((rt + 1));
	WideCharToMultiByte(CP_ACP, 0, src, -1, rs, rt, NULL, NULL);
	return rs;
}
void find(char* lpPath, std::vector<std::string> &fileList) {
	char szFind[MAX_PATH];
	WIN32_FIND_DATA FindFileData;
	strcpy(szFind, lpPath);
	strcat(szFind, "\\*.*");
	HANDLE hFind = FindFirstFile(widen(szFind), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)  return;
	while (true) {
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (FindFileData.cFileName[0] != '.') {
				char szFile[MAX_PATH];
				strcpy(szFile, lpPath);
				strcat(szFile, "\\");
				strcat(szFile, shorten(FindFileData.cFileName));
				find(szFile, fileList);
			}
		}
		else {
			fileList.push_back(std::string(szFind).substr(0, std::string(szFind).length() - 3) + shorten(FindFileData.cFileName));
		}
		if (!FindNextFile(hFind, &FindFileData))  break;
	}
	FindClose(hFind);
}

void initDict() {
	std::ifstream fin("dict.json");
	std::string stream, tmp;

	while (!fin.eof()) {
		getline(fin, tmp);
		stream += tmp;
	}
	fin.close();

	JSON json((char *)stream.data());
	char letter[2] = { 0 };
	for (int i = 1; i < 128; i++) {
		if (i == '\n' || i == '\r' || i == '\\' || i == '\"' || i == 0x1A)continue;
		letter[0] = i;
		float *data = json[letter].array;
		if (!data)continue;
		for (int j = 0; j < 128; j++) {
			ratio[i][j] = data[j];
		}
	}
}
void writeDict() {
	std::ofstream fout("C:\\Users\\MLKJ\\Documents\\Visual Studio 2017\\Projects\\ML\\Word\\dict.json");
	if (fout.is_open()) {
		fout << '{' << std::endl;
		for (int i = 1; i < 128; i++) {
			if (i == '\n' || i == '\r' || i == '\\' || i == '\"' || i == 0x1A)continue;
			fout << '\t' << '\"' << (char)i << '\"' << ':';
			fout << '[' << ratio[i][0];
			for (int j = 1; j < 128; j++) {
				fout << ", " << ratio[i][j];
			}
			fout << "]," << std::endl;
		}

		fout << '}' << std::endl;
		fout.close();
	}
}

void learnCode(const char *filename) {
	std::ifstream fin(filename);

	char tmp;
	char next;
	if (fin.is_open()) {
		tmp = fin.get();
		next = fin.get();

		if (tmp >= 0 && next >= 0)ratio[tmp][next]++;
		while (1) {
			tmp = next;
			next = fin.get();
			if (tmp == -1 || next == -1)break;
			if (tmp < 0 || next < 0)continue;
			ratio[tmp][next]++;
		}
	}
	return;
}
void addCode(widgetObj *w, int x, int y, int status) {
	mouseClickDefault(w, x, y, status);
	if (w->status & WIDGET_SELECTED) {
		char file[256];
		selectFile(file, NULL, NULL);
		learnCode(file);
		writeDict();
	}
}
void addFolder(widgetObj *w, int x, int y, int status) {
	mouseClickDefault(w, x, y, status);
	if (w->status & WIDGET_SELECTED) {
		char dir[256];
		selectDir(dir, NULL);

		std::vector<std::string> list;
		find(dir, list);
		for (auto s : list) {
			std::string format;
			int pos;
			if((pos = s.rfind('.'))>0)format = std::string(s.begin() + s.rfind('.') + 1, s.end());
			else continue;
			if(format == "c" || format == "cpp" || format == "h" || format == "hpp"||
				format == "java" || format == "xml" || format == "cs" ||
				format == "html" || format == "js" || format == "css" ||
				format == "py" || format == "php" || format == "matlab" ||
				format == "sql" || format == "v" || format == "s" || format == "asm")
				learnCode(s.data());
		}

		writeDict();
	}
}
void layoutWidget() {
	widgetObj *file = newWidget(SG_BUTTON, (SGstring)"file");
	file->pos.x = 40;
	file->pos.y = 40;
	file->size.x = 120;
	file->size.y = 32;
	strcpy((char *)file->content, "添加代码");
	file->mouseClick = (mouseClickCall)addCode;
	registerWidget(file);
	free(file);
	widgetObj *dir = newWidget(SG_BUTTON, (SGstring)"dir");
	dir->pos.x = 180;
	dir->pos.y = 40;
	dir->size.x = 120;
	dir->size.y = 32;
	strcpy((char *)dir->content, "添加文件夹");
	dir->mouseClick = (mouseClickCall)addFolder;
	registerWidget(dir);
	free(dir);

	widgetObj *vert = newWidget(SG_SCROLLVERT, (SGstring)"vert");
	vert->pos.x = 640;
	vert->pos.y = 80;
	vert->size.y = 360;
	vert->hide = 115;
	registerWidget(vert);
	free(vert);

	widgetObj *horiz = newWidget(SG_SCROLLHORIZ, (SGstring)"horiz");
	horiz->pos.x = 40;
	horiz->pos.y = 440;
	horiz->size.x = 600;
	horiz->hide = 120;
	registerWidget(horiz);
	free(horiz);
}
void sgSetup() {
	initWindow(680, 480, "Word statistic", BIT_MAP);
	initMouse(SG_COORDINATE);

	layoutWidget();
	initDict();
}
void sgLoop() {
	setColor(255, 255, 255);
	clearScreen();
	setColor(0, 0, 0);
	for (int i = 0; i < 16; i++) {
		putLine(40, 80 + i * 24, 640, 80 + i * 24, SOLID_LINE);
	}
	for (int i = 0; i < 11; i++) {
		putLine(40 + 60 * i, 80, 40 + 60 * i,  440, SOLID_LINE);
	}

	int x = getWidgetByName("vert")->value;
	int y = getWidgetByName("horiz")->value;

	for (int i = 0; i < 9; i++) {
		putChar(i + y, 126 + 60 * i, 84);
	}
	for (int i = 0; i < 14; i++) {
		putChar(i + x, 66, 108 + i * 24);
	}

	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 14; j++) {
			putNumber(ratio[j + x][i + y], 156 + 60 * i, 108 + j * 24, 'r');
		}
	}
	return;
}