#include <iostream>
#include <fstream>
#include "windows.h"
#include <tlhelp32.h>
#include <tchar.h>
#include <atlstr.h>
#include <signal.h>
#include <ctime>
#include "json/json.h"
#include "json/json-forwards.h"

using namespace std;
#define MAX_OF_PLAYED_PLAYERS 18;
#define MAX_OF_ROSTER_PLAYERS 32;
#define NOT_PLAYED -1;
#define VERSION_APP 1.3;
#define HALF_1 0;
#define HALF_2 1;
#define EX_HALF_1 2;
#define EX_HALF_2 3;

HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess);
void setPlayersStats(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
BOOL setPlayers(HANDLE pHandle, DWORD addr, __int16* arr);
BOOL setTeamScore(HANDLE pHandle, DWORD addr, int* dst);
BOOL setTeamExtraTimeScore(HANDLE pHandle, DWORD half1addr, DWORD half2addr, int* dst);
void print(int* arr, int len);
BOOL setPlayedPlayers(HANDLE pHandle, DWORD addr, int* playedPlayersHome, int* playedPlayersGuest);
void createFileWithStats();
string currentDateTime();
void setTeamName(HANDLE pHandle, DWORD addr, char* name);
void setPlayersDribbleDistances(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
void setPlayersStatsOffset68(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
void setPlayersNames(HANDLE pHandle, DWORD addr, string* arr);

__int8 matchStarted = 0;
__int8 matchStartedBuf = 0;
__int8 matchStadyBuf = 0;
__int16 playersHome[32];
__int16 playersGuest[32];
int playersHomeGoals[32] = { 0 };
int playersGuestGoals[32] = { 0 };
int playersHomeAssists[32] = { 0 };
int playersGuestAssists[32] = { 0 };
int playersHomeYellowCards[32] = { 0 };
int playersGuestYellowCards[32] = { 0 };
int playersHomeRedCards[32] = { 0 };
int playersGuestRedCards[32] = { 0 };
int playedPlayersHome[18] = { 0 };
int playedPlayersGuest[18] = { 0 };
int playersHomeDribbleDistances[32] = { 0 };
int playersGuestDribbleDistances[32] = { 0 };
int playersHomeFouls[32] = { 0 };
int playersGuestFouls[32] = { 0 };
int playersHomeIntercepts[32] = { 0 };
int playersGuestIntercepts[32] = { 0 };
int playersHomeClearedPasses[32] = { 0 };
int playersGuestClearedPasses[32] = { 0 };
int playersHomePossessionOfBall[32] = { 0 };
int playersGuestPossessionOfBall[32] = { 0 };
string playersHomeNames[32];
string playersGuestNames[32];
int teamHomeScore = 0;
int teamHomeEXScore = 0;
int teamHomePenaltiesScore = 0;
int teamGuestScore = 0;
int teamGuestEXScore = 0;
int teamGuestPenaltiesScore = 0;
string matchStartTime;
char* teamNameHome = new char[50];
char* teamNameGuest = new char[50];
wstringstream fileName;
BOOL isTeamPlayersReaded = false;
int matchMinute;
int matchStady;
std::ofstream out;

int main(int argc, char** argv) {
	HANDLE pHandle;
	TCHAR gameName [50];
	const DWORD matchStartedAddr = 0x010CF2EC;
	const DWORD playersHomeAddr = 0x010D4536;
	const DWORD playersGuestAddr = 0x010D775E;
	const DWORD playersHomeGoalsAddr = 0x01017B5C;
	const DWORD playersHomeAssistsAddr = 0x01017B60;
	const DWORD playersGuestGoalsAddr = 0x01017E50;
	const DWORD playersGuestAssistsAddr = 0x01017E54;
	const DWORD compoundsAddr = 0x01018DE8;
	const DWORD teamNameHomeAddr = 0x010D3C16;
	const DWORD teamNameGuestAddr = 0x010D6E3E;
	const DWORD playersHomeYellowCardsAddr = 0x01017B70;
	const DWORD playersGuestYellowCardsAddr = 0x01017E64;
	const DWORD playersHomeRedCardsAddr = 0x01017B71;
	const DWORD playersGuestRedCardsAddr = 0x01017E65;
	const DWORD teamHomeScoreAddr = 0x01017B38;
	const DWORD teamHomeEX1HalfScoreAddr = 0x01017B34;
	const DWORD teamHomeEX2HalfScoreAddr = 0x01017B36;
	const DWORD teamHomePenaltiesScoreAddr = 0x01017B3C;
	const DWORD teamGuestScoreAddr = 0x01017E2C;
	const DWORD teamGuestEX1HalfScoreAddr = 0x01017E28;
	const DWORD teamGuestEX2HalfScoreAddr = 0x01017E2A;
	const DWORD teamGuestPenaltiesScoreAddr = 0x01017E30;
	const DWORD matchMinuteAddr = 0x010D2986;
	const DWORD matchStadyAddr = 0x010D298C;
	const DWORD playersHomeDribbleDistancesAddr = 0x010CF660;
	const DWORD playersGuestDribbleDistancesAddr = 0x010CFDB0;
	const DWORD playersHomeFoulsAddr = 0x01017B68;
	const DWORD playersGuestFoulsAddr = 0x01017E5C;
	const DWORD playersHomeInterceptsAddr = 0x10CF65C;
	const DWORD playersGuestInterceptsAddr = 0x010CFDAC;
	const DWORD playersHomeNamesAddr = 0x03BCF688;
	const DWORD playersGuestNamesAddr = 0x03BD5F88;
	const DWORD playersHomeClearedPassesAddr = 0x10CF650;
	const DWORD playersGuestClearedPassesAddr = 0x10CFDA0;
	const DWORD playersHomePossessionOfBallAddr = 0x10CF63E;
	const DWORD playersGuestPossessionOfBallAddr = 0x10CFD8E;


	USES_CONVERSION;
	_tcscpy(gameName, A2T("PESWay.exe"));

	pHandle = OpenProcessByName(gameName, PROCESS_VM_READ);
	if (pHandle == INVALID_HANDLE_VALUE || pHandle == NULL) {
		cout << "Error by open game process" << endl;
		cout << "Error code : " << GetLastError() << endl;
		return -1;
	}
	cout << "Process opened!" << endl;

	WCHAR gameDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, gameDir, MAX_PATH);
	PathRemoveFileSpec(gameDir);

	wstringstream ss;
	ss << gameDir << L"\\stats\\";

	const wstring dir = ss.str();

	int result = CreateDirectory(dir.c_str(), NULL);
	if (result == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
		cout << "Error when create folder: code " << GetLastError() << endl;
		return -1;
	}

	ss << L"\\log\\";
	wstring logPath = ss.str();
	result = CreateDirectory(logPath.c_str(), NULL);
	if (result == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
		cout << "Error when create folder: code " << GetLastError() << endl;
		return -1;
	}

	ss.str(L"");
	ss << logPath << L"log_" << currentDateTime().c_str();
	out.open(ss.str(), std::ofstream::out | std::ofstream::app);

	BOOL allowReading = true;
	while (true) {
		Sleep(2000);

		if (!ReadProcessMemory(pHandle, (LPCVOID)matchStartedAddr, &matchStarted, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}

		__int8 buf2;
		if (!ReadProcessMemory(pHandle, (LPCVOID)matchStadyAddr, &buf2, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			out << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}
		matchStady = (int)buf2;

		if (matchStarted != 1) {
			if (matchStartedBuf == 1) {
				cout << "Match ended" << endl;
				out << "Match ended" << endl;
			}
			if (matchStartTime.length() == 0) {
				matchStartTime = currentDateTime();
			}
		}

		if ((matchStartedBuf == 0 && matchStarted == 1) || (matchStadyBuf > 0 && matchStady == 0)) {
			
			matchStartTime = currentDateTime();
			allowReading = true;
			isTeamPlayersReaded = false;
		}

		matchStartedBuf = matchStarted;
		matchStadyBuf = matchStady;

		if (!allowReading) {
			continue;
		}

		if (matchStarted != 1) {
			allowReading = false;
		}

		__int8 buf1;
		if (!ReadProcessMemory(pHandle, (LPCVOID)matchMinuteAddr, &buf1, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			out << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}

		if (matchStady <= 1) {
			matchMinute = (int)buf1;
		} else if (matchStady < 4) {
			matchMinute = (int)buf1 + 90;
		} else if (matchStady == 4) {
			matchMinute = 120;
		}
		cout << "Match minute " << matchMinute << endl;
		out << "Match minute " << matchMinute << endl;

		cout << "Match stady " << matchStady << endl;
		out << "Match stady " << matchStady << endl;

		setTeamName(pHandle, teamNameHomeAddr, teamNameHome);
		setTeamName(pHandle, teamNameGuestAddr, teamNameGuest);
		if (string(teamNameHome) == string(teamNameGuest) && string(teamNameHome) == string("")) {
			cout << "Empty team names" << endl;
			out << "Empty team names" << endl;
			continue;
		}
		if (!setPlayers(pHandle, playersHomeAddr, playersHome)) {
			continue;
		}
		if (!setPlayers(pHandle, playersGuestAddr, playersGuest)) {
			continue;
		}

		if (!setPlayedPlayers(pHandle, compoundsAddr, playedPlayersHome, playedPlayersGuest)) {
			continue;
		}

		if (!isTeamPlayersReaded) {
			setPlayersNames(pHandle, playersHomeNamesAddr, playersHomeNames);
			setPlayersNames(pHandle, playersGuestNamesAddr, playersGuestNames);
			isTeamPlayersReaded = true;
		}

		fileName.str(L"");
		fileName.clear();
		fileName << dir << string(teamNameHome).c_str() << " - " << string(teamNameGuest).c_str() << " " << matchStartTime.c_str() << ".dat";
		cout << "Record match " << string(teamNameHome) << " - " << string(teamNameGuest) << " " << matchStartTime << endl;
		out << "Record match " << string(teamNameHome) << " - " << string(teamNameGuest) << " " << matchStartTime << endl;

		if (!setTeamScore(pHandle, teamHomeScoreAddr, &teamHomeScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamHomePenaltiesScoreAddr, &teamHomePenaltiesScore)) {
			continue;
		}
		if (!setTeamExtraTimeScore(pHandle, teamHomeEX1HalfScoreAddr, teamHomeEX2HalfScoreAddr, &teamHomeEXScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamGuestScoreAddr, &teamGuestScore)) {
			continue;
		}
		if (!setTeamExtraTimeScore(pHandle, teamGuestEX1HalfScoreAddr, teamGuestEX2HalfScoreAddr, &teamGuestEXScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamGuestPenaltiesScoreAddr, &teamGuestPenaltiesScore)) {
			continue;
		}

		setPlayersStats(pHandle, playersHomeGoalsAddr, playersHomeGoals, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestGoalsAddr, playersGuestGoals, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeAssistsAddr, playersHomeAssists, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestAssistsAddr, playersGuestAssists, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeYellowCardsAddr, playersHomeYellowCards, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestYellowCardsAddr, playersGuestYellowCards, playedPlayersGuest);
		setPlayersStats(pHandle, playersHomeRedCardsAddr, playersHomeRedCards, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestRedCardsAddr, playersGuestRedCards, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeFoulsAddr, playersHomeFouls, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestFoulsAddr, playersGuestFouls, playedPlayersGuest);

		// This values updates only after halftime
		if (matchStady > 0) {
			setPlayersDribbleDistances(pHandle, playersHomeDribbleDistancesAddr, playersHomeDribbleDistances, playedPlayersHome);
			setPlayersDribbleDistances(pHandle, playersGuestDribbleDistancesAddr, playersGuestDribbleDistances, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomeInterceptsAddr, playersHomeIntercepts, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestInterceptsAddr, playersGuestIntercepts, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomeClearedPassesAddr, playersHomeClearedPasses, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestClearedPassesAddr, playersGuestClearedPasses, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomePossessionOfBallAddr, playersHomePossessionOfBall, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestPossessionOfBallAddr, playersGuestPossessionOfBall, playedPlayersGuest);
		} else {
			ZeroMemory(playersHomeDribbleDistances, 128);
			ZeroMemory(playersGuestDribbleDistances, 128);
			ZeroMemory(playersHomeIntercepts, 128);
			ZeroMemory(playersGuestIntercepts, 128);
			ZeroMemory(playersHomeClearedPasses, 128);
			ZeroMemory(playersGuestClearedPasses, 128);
			ZeroMemory(playersHomePossessionOfBall, 128);
			ZeroMemory(playersGuestPossessionOfBall, 128);
		}

		createFileWithStats();
	}
	return 0;
}

void createFileWithStats() {
	Json::Value root;
	Json::StyledWriter styledWriter;
	Json::Value playedPlayers;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;

	root["home"]["team"] = string(teamNameHome);
	root["guest"]["team"] = string(teamNameGuest);
	root["home"]["score"] = teamHomeScore;
	if (teamHomePenaltiesScore != 0 || teamGuestPenaltiesScore != 0) {
		root["home"]["penaltiesScore"] = teamHomePenaltiesScore;
		root["guest"]["penaltiesScore"] = teamGuestPenaltiesScore;
	}
	if (matchStady > 1) {
		root["home"]["extraTimeScore"] = teamHomeEXScore;
		root["guest"]["extraTimeScore"] = teamGuestEXScore;
	}
	root["guest"]["score"] = teamGuestScore;
	root["minutesPlayed"] = matchMinute;
	root["version"] = VERSION_APP;

	std::stringstream ss;

	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayersHome[i] != notPlayed) {
			root["home"]["playedPlayersPos"][i] = playedPlayersHome[i];
			root["home"]["playedPlayersIds"][i] = playersHome[playedPlayersHome[i]];
			root["home"]["playedPlayersNames"][i] = playersHomeNames[playedPlayersHome[i]];
		}
		if (playedPlayersGuest[i] != notPlayed) {
			root["guest"]["playedPlayersPos"][i] = playedPlayersGuest[i];
			root["guest"]["playedPlayersIds"][i] = playersGuest[playedPlayersGuest[i]];
			root["guest"]["playedPlayersNames"][i] = playersGuestNames[playedPlayersGuest[i]];
		}
	}
	
	for (int i = 0; i < 32; i++) {		
		if (playersHomeGoals[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["goals"][ss.str()] = playersHomeGoals[i];
			ss.str(std::string());
			ss.clear();
		}
			
		if (playersHomeAssists[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["assists"][ss.str()] = playersHomeAssists[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeYellowCards[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["yellowCards"][ss.str()] = playersHomeYellowCards[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeRedCards[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["redCards"][ss.str()] = playersHomeRedCards[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeDribbleDistances[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["dribbleDistances"][ss.str()] = playersHomeDribbleDistances[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeFouls[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["fouls"][ss.str()] = playersHomeFouls[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeIntercepts[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["intercepts"][ss.str()] = playersHomeIntercepts[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomeClearedPasses[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["clearedPasses"][ss.str()] = playersHomeClearedPasses[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersHomePossessionOfBall[i] != 0) {
			ss << playersHome[playedPlayersHome[i]];
			//ss << playersHomeNames[playedPlayersHome[i]];
			root["home"]["possessionOfBall"][ss.str()] = playersHomePossessionOfBall[i];
			ss.str(std::string());
			ss.clear();
		}
			
		if (playersGuestGoals[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["goals"][ss.str()] = playersGuestGoals[i];
			ss.str(std::string());
			ss.clear();
		}
			
		if (playersGuestAssists[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["assists"][ss.str()] = playersGuestAssists[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestYellowCards[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["yellowCards"][ss.str()] = playersGuestYellowCards[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestRedCards[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["redCards"][ss.str()] = playersGuestRedCards[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestDribbleDistances[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["dribbleDistances"][ss.str()] = playersGuestDribbleDistances[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestFouls[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["fouls"][ss.str()] = playersGuestFouls[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestIntercepts[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["intercepts"][ss.str()] = playersGuestIntercepts[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestClearedPasses[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["clearedPasses"][ss.str()] = playersGuestClearedPasses[i];
			ss.str(std::string());
			ss.clear();
		}

		if (playersGuestPossessionOfBall[i] != 0) {
			ss << playersGuest[playedPlayersGuest[i]];
			//ss << playersGuestNames[playedPlayersGuest[i]];
			root["guest"]["possessionOfBall"][ss.str()] = playersGuestPossessionOfBall[i];
			ss.str(std::string());
			ss.clear();
		}
	}
	
	std::ofstream out;
	out.open(fileName.str());
	if (out.fail()) {
		cout << "open failure : " << endl;
		out << "open failure : " << endl;
		return;
	}
	out << styledWriter.write(root) << endl;
	out.close();
	cout << "Successfully create file" << endl;
	out << "Successfully create file" << endl;
}

void setPlayersStats(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x28;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	__int8 val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, (LPCVOID)addr, &val, 1, NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void setTeamName(HANDLE pHandle, DWORD addr, char* name) {
	if (!ReadProcessMemory(pHandle, (LPCVOID)addr, name, 50, NULL)) {
		cout << "Error by reading team name - code : " << GetLastError() << endl;
		out << "Error by reading team name - code : " << GetLastError() << endl;
	}
}

BOOL setPlayedPlayers(HANDLE pHandle, DWORD addr, int* playedPlayersHome, int* playedPlayersGuest) {
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	int notPlayed = NOT_PLAYED;
	int i = 0;
	__int8* buf = new __int8[maxOfPlayedPlayers];
	DWORD offset = 0x12;
	if (!ReadProcessMemory(pHandle, (LPCVOID)addr, buf, maxOfPlayedPlayers, NULL)) {
		cout << "Error by reading home played players - code : " << GetLastError() << endl;
		out << "Error by reading home played players - code : " << GetLastError() << endl;
		return false;
	}
	int playedPlayersHomeCount = 0;
	for (i = 0; i < maxOfPlayedPlayers; i++) {
		if (buf[i] > maxOfRosterPlayers - 1 || buf[i] < notPlayed) {
			cout << "Value damaged " << buf[i] << endl;
			out << "Value damaged " << buf[i] << endl;
			return false;
		}
		if (buf[i] != notPlayed) {
			playedPlayersHomeCount++;
		}
		playedPlayersHome[i] = (int)buf[i];
	}
	if (playedPlayersHomeCount < 11) {
		cout << "Home pLayers on a field less than 11 - " << playedPlayersHomeCount << endl;
		out << "Home pLayers on a field less than 11 - " << playedPlayersHomeCount << endl;
		return false;
	}
	if (!ReadProcessMemory(pHandle, (LPCVOID)(addr + offset), buf, maxOfPlayedPlayers, NULL)) {
		cout << "Error by reading guest played players - code : " << GetLastError() << endl;
		out << "Error by reading guest played players - code : " << GetLastError() << endl;
		return false;
	}
	int playedPlayersGuestCount = 0;
	for (i = 0; i < maxOfPlayedPlayers; i++) {
		if (buf[i] > maxOfRosterPlayers || buf[i] < notPlayed) {
			cout << "Value damaged " << buf[i] <<endl;
			out << "Value damaged " << buf[i] << endl;
			return false;
		}
		if (buf[i] != notPlayed) {
			playedPlayersGuestCount++;
		}
		playedPlayersGuest[i] = int(buf[i]);
	}
	if (playedPlayersGuestCount < 11) {
		cout << "Guest pLayers on a field less than 11 - " << playedPlayersGuestCount << endl;
		out << "Guest pLayers on a field less than 11 - " << playedPlayersGuestCount << endl;
		return false;
	}
	return true;
}

BOOL setPlayers(HANDLE pHandle, DWORD addr, __int16* arr) {
	__int16 buf[32];
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	if (!ReadProcessMemory(pHandle, (LPCVOID)addr, buf, maxOfRosterPlayers * 2, NULL)) {
		cout << "Error by reading team players - code : " << GetLastError() << endl;
		out << "Error by reading team players - code : " << GetLastError() << endl;
		return false;
	}
	for (int i = 0; i < maxOfRosterPlayers; i++) {
		// Check player id
		if (buf[i] < 0 || buf[i] > 5000) {
			cout << "Value of id player damaged " << buf[i] << endl;
			out << "Value of id player damaged " << buf[i] << endl;
			return false;
		}
		arr[i] = buf[i];
	}
	return true;
}

void setPlayersNames(HANDLE pHandle, DWORD baseAddr, string* arr) {
	DWORD offset = 0x348;
	int maxNameLength = 30;
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	char* buf = new char[maxNameLength];
	DWORD addr;
	for (int i = 0; i < maxOfRosterPlayers; i++) {
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, (LPCVOID)addr, buf, maxNameLength, NULL)) {
			cout << "Error by reading team players - code : " << GetLastError() << endl;
			out << "Error by reading team players - code : " << GetLastError() << endl;
		}
		int k = 0;
		string name;
		while (buf[k] != 0) {
			name += buf[k];
			k++;
		}
		arr[i] = string(buf);
	}
}

BOOL setTeamScore(HANDLE pHandle, DWORD addr, int* dst) {
	__int8 buf;
	if (!ReadProcessMemory(pHandle, (LPCVOID)addr, &buf, 1, NULL)) {
		cout << "Error by reading team score - code : " << GetLastError() << endl;
		out << "Error by reading team score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = (int)buf;
	return true;
}

BOOL setTeamExtraTimeScore(HANDLE pHandle, DWORD half1addr, DWORD half2addr, int* dst) {
	__int8 buf;
	if (!ReadProcessMemory(pHandle, (LPCVOID)half1addr, &buf, 1, NULL)) {
		cout << "Error by reading team extra time score - code : " << GetLastError() << endl;
		out << "Error by reading team extra time score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = (int)buf;
	if (!ReadProcessMemory(pHandle, (LPCVOID)half2addr, &buf, 1, NULL)) {
		cout << "Error by reading team score - code : " << GetLastError() << endl;
		out << "Error by reading team score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = *dst + (int)buf;

	return true;
}

void setPlayersDribbleDistances(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x68;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	float val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, (LPCVOID)addr, &val, sizeof(float), NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void setPlayersStatsOffset68(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x68;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	__int8 val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, (LPCVOID)addr, &val, 1, NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void print(int* arr, int len) {
	for (int i = 0; i < len; i++) {
		if (i != len - 1) {
			printf("%d, ", arr[i]);
		}
		else {
			printf("%d", arr[i]);
		}
	}
	printf("\n");
}

string currentDateTime() {
	time_t now = time(0);
	struct tm tstruct;
	char* buf = new char[80];
	tstruct = *localtime(&now);
	strftime(buf, 80*sizeof(char), "%Y-%m-%d %H-%M-%S", &tstruct);
	string str(buf);
	return str;
}

HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		ZeroMemory(&pe, sizeof(PROCESSENTRY32));
		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		do
		{
			if (!lstrcmpi(pe.szExeFile, name))
			{
				cout << "Find process id " << pe.th32ProcessID << endl;
				out << "Find process id " << pe.th32ProcessID << endl;
				return OpenProcess(dwAccess, false, pe.th32ProcessID);
			}
		} while (Process32Next(hSnap, &pe));

	}
	return INVALID_HANDLE_VALUE;
}