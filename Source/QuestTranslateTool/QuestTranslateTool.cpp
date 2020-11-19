#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <chrono>

static constexpr auto TRANSLATEFILE = "translate.lua";
static constexpr auto QUESTFOLDER = "quest/";
static int cReplaced = 0;
static bool ModeFirst = false;
static std::unordered_map<std::string, std::string> TranslateMap;

auto ReadTranslateLua()
{
	std::ifstream myfile(TRANSLATEFILE);

	if (myfile.is_open()) {
		std::string line;		
		while (std::getline(myfile, line)) {
			auto pos = line.find('=');
			if (pos == std::string::npos)
				continue;

			auto GFString = line.substr(0, pos);

			if (GFString.back() == ' ')
				GFString.pop_back();

			pos = line.find('"');		
				
			if (pos != std::string::npos)
				TranslateMap.emplace(std::move(GFString), std::move(line.erase(0, pos)));
		}

		myfile.close();
	}
	else
		return false;

	return true;
}

void EditFile(const std::string& FileName)
{
	std::fstream file(FileName, std::ios::in);

	if (!file.is_open())
		return;

	int line_count = 1;
	bool IsChanged = false;
	std::string sLine;
	std::vector<std::string> vLines;

	while (std::getline(file, sLine)) {
		size_t pos = 0;
		for (const auto& x : TranslateMap) {
			const std::string *from, *to;
			if (ModeFirst) {
				from = &x.first;
				to = &x.second;
			}
			else {
				to = &x.first;
				from = &x.second;
			}
			
			while ((pos = sLine.find(*from)) != std::string::npos) {
				sLine.replace(pos, from->size(), *to);
				printf("Filename: %s, From: %s, To: %s (Line: %d)\n", FileName.c_str(), from->c_str(), to->c_str(), line_count);
				++cReplaced;
				IsChanged = true;
			}
		}

		vLines.emplace_back(std::move(sLine));
		line_count++;
	}

	file.close();

	if (IsChanged) {
		file.open(FileName, std::ios::out | std::ios::trunc);

		for (const auto& v : vLines)
			file << v << std::endl;

		file.close();
	}
}

auto SendQuestNames()
{
	const std::vector<std::string> ExtensionList{ "lua", "quest" };
	const std::vector<std::string> ExceptionList{ "locale.lua" };

	for (auto& PathIterator : std::filesystem::directory_iterator(QUESTFOLDER)) {
		std::string PathString = PathIterator.path().string();
		const std::string_view GetExtension = std::string_view(PathString).substr(PathString.find_last_of(".") + 1);
		const std::string_view GetRealName = std::string_view(PathString).substr(PathString.find_last_of("/") + 1);

		if (std::find(ExtensionList.begin(), ExtensionList.end(), GetExtension) == ExtensionList.end() || std::find(ExceptionList.begin(), ExceptionList.end(), GetRealName) != ExceptionList.end())
			continue;

		EditFile(PathString);
	}
}

void PrintVersion()
{
	constexpr auto VersionText = "V-1.2";
	constexpr auto Author = "blackdragonx61";
	printf("%s | %s\n\n", VersionText, Author);
}

template<typename T> std::string DotString(T val)
{
	constexpr int comma = 3;
	auto str = std::to_string(val);
	auto pos = static_cast<int>(str.length()) - comma;

	while (pos > 0) {
		str.insert(pos, ".");
		pos -= comma;
	}

	return str;
}

int main()
{
	if (ReadTranslateLua()) {
		printf("Select Mode:\n");
		printf("0) String to GF\n");
		printf("1) GF to String\n");

		std::cin >> ModeFirst;

		if (!TranslateMap.empty()) {
			printf("\nWork In Progress...\n\n");

			auto begin = std::chrono::steady_clock::now();
			SendQuestNames();
			auto end = std::chrono::steady_clock::now();

			if (cReplaced)
				std::cout << "\nElapsed Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]\n" << std::endl;
		}

		printf("%s String Changed.\n", DotString(cReplaced).c_str());
	}
	else
		printf("Cannot open %s\n\n", TRANSLATEFILE);

	PrintVersion();
	system("pause");
	return 0;
}