#pragma once
#include <Globals/hIncludes.h>
#include <Windows.h>
#include <WinINet.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <Json/Json.hpp>

using json = nlohmann::json;
namespace fileSys = std::filesystem;
using namespace std;

#ifdef _WIN32
std::time_t timegm(std::tm* tm) {
    std::time_t t = _mkgmtime(tm);  // Use _mkgmtime on Windows for UTC time
    return t;
}
#endif

class Updater {
private:
    string Github_Repo_Api = "https://api.github.com/repos/a2x/cs2-dumper/commits";

    const std::vector<std::pair<std::string, std::string>> Github_File_Path = {
        {"https://github.com/a2x/cs2-dumper/raw/main/output/offsets.json", "Offsets.json"},
        {"https://github.com/a2x/cs2-dumper/raw/main/output/client_dll.json", "client_dll.json"}
        // Add more files here as needed
    };

    bool FileExists(const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    }

    bool DownloadFile(const std::string& url, const std::string& localPath) {
        HINTERNET hInternet, hConnect;

        hInternet = InternetOpen(L"Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            std::cerr << " [Updater] InternetOpen failed." << std::endl;
            return false;
        }

        hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            std::cerr << " [Updater] InternetOpenUrlA failed." << std::endl;
            InternetCloseHandle(hInternet);
            return false;
        }

        std::ofstream outFile(localPath, std::ios::binary);
        if (!outFile) {
            std::cerr << " [Updater] Failed to create local file." << std::endl;
            InternetCloseHandle(hConnect);
            InternetCloseHandle(hInternet);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            outFile.write(buffer, bytesRead);
        }

        outFile.close();
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return true;
    }

    bool GetLastCommitInfo(json& commit) {
        HINTERNET hInternet, hConnect;

        hInternet = InternetOpen(L"Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            return false;
        }

        hConnect = InternetOpenUrlA(hInternet, Github_Repo_Api.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string commitData;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            commitData.append(buffer, bytesRead);
        }

        json data;
        try {
            data = json::parse(commitData);
        }
        catch (const std::exception& e) {
            std::cout << " [Updater] Failed to parse JSON response from GitHub" << std::endl;
            return false;
        }

        if (data.empty())
            return false;

        if (data.is_array()) {
            json last_commit = data[0];
            json last_commit_author = last_commit["commit"]["author"];
            commit = last_commit_author;
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return true;
    }

public:
    bool CheckAndDownload(bool prompt_update) {
        json Commit;

        // Get the last commit information from GitHub
        if (!GetLastCommitInfo(Commit)) {
            std::cout << " [Updater] Error getting last commit information from GitHub" << std::endl;
            return false;
        }

        string Last_Commit_Date = Commit["date"];
        string Last_Commit_Author_Name = Commit["name"];

        std::tm Commit_Date_Buffer = {};
        std::istringstream ss(Last_Commit_Date);
        ss >> std::get_time(&Commit_Date_Buffer, "%Y-%m-%dT%H:%M:%SZ");

        // Convert to time_t and then to a time_point
        std::time_t commit_time_t = timegm(&Commit_Date_Buffer);
        auto CommitTimePoint = std::chrono::system_clock::from_time_t(commit_time_t);

        // Format the commit time for output
        std::tm commit_time_tm;
        gmtime_s(&commit_time_tm, &commit_time_t); // Use gmtime_s for thread safety

        // Output the message with author name and date/time
        std::cout << " [Updater] Last GitHub Repository Update was made by "
            << Last_Commit_Author_Name
            << " on " << std::put_time(&commit_time_tm, "[%Y-%m-%d %H:%M:%S")
            << " (UTC)]" << std::endl;

        // Check for each file in the Github_File_Path vector
        for (const auto& file : Github_File_Path) {
            const auto& url = file.first;
            const auto& localPath = file.second;

            bool fileExists = FileExists(localPath);
            auto lastModifiedTime = fileExists ? fileSys::last_write_time(localPath) : fileSys::file_time_type{};

            // Convert file modification time to system clock time_point if file exists
            auto lastModifiedClockTime = fileExists
                ? std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    lastModifiedTime - fileSys::file_time_type::clock::now() + std::chrono::system_clock::now())
                : std::chrono::system_clock::time_point{};

            // Check if the file is outdated
            if (fileExists && lastModifiedClockTime < CommitTimePoint) {

                std::cout << " [Updater] " << localPath << " is Outdated" << std::endl;

                if (prompt_update) {
                    char response;
                    std::cout << " [Updater] Do you want to update " << localPath << " to the latest version? (Y/N): ";
                    std::cin >> response;
                    if (response == 'Y' || response == 'y') {
                        // Download the file
                        if (DownloadFile(url, localPath)) {
                            std::cout << " [Updater] Successfully downloaded the latest " << localPath << "." << std::endl;
                        }
                        else {
                            std::cout << " [Updater] Error: Failed to download " << localPath << ". Try downloading manually from " << url << std::endl;
                        }
                    }
                }

            }
            else if (!fileExists) {

                if (prompt_update) {
                    char response;
                    std::cout << " [Updater] Do you want to download the latest " << localPath << "? (Y/N): ";
                    std::cin >> response;
                    if (response == 'Y' || response == 'y') {
                        // Download the file
                        if (DownloadFile(url, localPath)) {
                            std::cout << " [Updater] Successfully downloaded the latest " << localPath << "." << std::endl;
                        }
                        else {
                            std::cout << " [Updater] Error: Failed to download " << localPath << ". Try downloading manually from " << url << std::endl;
                        }
                    }
                }

            }
            else {

                std::cout << " [Updater] " << localPath << " is Up-to-Date." << std::endl;

            }
        }
        return true;
    }

    bool UpdateOffsets() {
        for (const auto& file : Github_File_Path)
        {
            const auto& localPath = file.second;

            if (!FileExists(localPath)) {
                std::cerr << " [Updater] Offsets file not found." << std::endl;
                return false;
            }

            std::ifstream inFile(localPath);
            if (!inFile) {
                std::cerr << " [Updater] Failed to open offsets file." << std::endl;
                return false;
            }

            json Data;
            try {
                inFile >> Data; // Read JSON data
            }
            catch (const std::exception& e) {
                std::cerr << " [Updater] Failed to parse offsets JSON: " << e.what() << std::endl;
                return false;
            }

            const auto& Client = Data["client.dll"];
            const auto& Matchmaking = Data["matchmaking.dll"];
            const auto& C_BaseEntity = Data["C_BaseEntity"];
            const auto& C_CSPlayerPawn = Data["C_CSPlayerPawn"];
            const auto& CCSPlayerController = Data["CCSPlayerController"];
            const auto& C_CSPlayerPawnBase = Data["C_CSPlayerPawnBase"];
            const auto& C_BasePlayerPawn = Data["C_BasePlayerPawn"];
            const auto& C_BaseModelEntity = Data["C_BaseModelEntity"];
            const auto& CGlowProperty = Data["CGlowProperty"];
            const auto& C_CSGameRules = Data["C_CSGameRules"];
            const auto& C_PlantedC4 = Data["C_PlantedC4"];
            const auto& CGameSceneNode = Data["CGameSceneNode"];
            const auto& EntitySpottedState_t = Data["EntitySpottedState_t"];

            // Getting Offsets.json
            if (Data.contains("client.dll")) {               
                Offsets::dwEntityList = Client["dwEntityList"];
                Offsets::dwLocalPlayerPawn = Client["dwLocalPlayerPawn"];
                Offsets::dwLocalPlayerController = Client["dwLocalPlayerController"];
                Offsets::dwViewAngles = Client["dwViewAngles"];
                Offsets::dwViewMatrix = Client["dwViewMatrix"];
                Offsets::dwSensitivity = Client["dwSensitivity"];
                Offsets::dwSensitivity_sensitivity = Client["dwSensitivity_sensitivity"];
                Offsets::dwGameRules = Client["dwGameRules"];
                Offsets::dwPlantedC4 = Client["dwPlantedC4"];
                Offsets::dwGlobalVars = Client["dwGlobalVars"];
                Offsets::dwWeaponC4 = Client["dwWeaponC4"];
            }

            if (Data.contains("matchmaking.dll"))
            {                
                Offsets::dwGameTypes = Matchmaking["dwGameTypes"];
                Offsets::dwGameTypes_mapName = Matchmaking["dwGameTypes_mapName"];
            }

            // C_BaseEntity
            if (Data.contains("C_BaseEntity"))
            {           
                Offsets::m_iTeamNum = C_BaseEntity["m_iTeamNum"];
                Offsets::m_iHealth = C_BaseEntity["m_iHealth"];
                Offsets::m_pGameSceneNode = C_BaseEntity["m_pGameSceneNode"];
                Offsets::m_fFlags = C_BaseEntity["m_fFlags"];
                Offsets::m_vecAbsVelocity = C_BaseEntity["m_vecAbsVelocity"];
                Offsets::m_fFlags = C_BaseEntity["m_fFlags"];
                Offsets::m_hOwnerEntity = C_BaseEntity["m_hOwnerEntity"];
            }

            // C_CSPlayerPawn
            if (Data.contains("C_CSPlayerPawn"))
            {               
                Offsets::m_ArmorValue = C_CSPlayerPawn["m_ArmorValue"];
                Offsets::m_iShotsFired = C_CSPlayerPawn["m_iShotsFired"];
                Offsets::m_aimPunchAngle = C_CSPlayerPawn["m_aimPunchAngle"];
                Offsets::m_bIsScoped = C_CSPlayerPawn["m_bIsScoped"];
                Offsets::m_entitySpottedState = C_CSPlayerPawn["m_entitySpottedState"];
            }

            // CCSPlayerController
            if (Data.contains("CCSPlayerController"))
            {
                Offsets::m_hPlayerPawn = CCSPlayerController["m_hPlayerPawn"];
                Offsets::m_sSanitizedPlayerName = CCSPlayerController["m_sSanitizedPlayerName"];
                Offsets::m_iPing = CCSPlayerController["m_iPing"];
            }

            // C_CSPlayerPawnBase
            if (Data.contains("C_CSPlayerPawnBase"))
            {
                Offsets::m_flFlashBangTime = C_CSPlayerPawnBase["m_flFlashBangTime"];
                Offsets::m_iIDEntIndex = C_CSPlayerPawnBase["m_iIDEntIndex"];
            }

            // C_BasePlayerPawn
            if (Data.contains("C_BasePlayerPawn"))
            {
                Offsets::m_vOldOrigin = C_BasePlayerPawn["m_vOldOrigin"];
            }

            // C_BaseModelEntity
            if (Data.contains("C_BaseModelEntity"))
            {
                Offsets::m_vecViewOffset = C_BaseModelEntity["m_vecViewOffset"];
                Offsets::m_Glow = C_BaseModelEntity["m_Glow"];
            }

            // CGlowProperty
            if (Data.contains("CGlowProperty"))
            {               
                Offsets::m_glowColorOverride = CGlowProperty["m_glowColorOverride"];
                Offsets::m_bGlowing = CGlowProperty["m_bGlowing"];
            }

            // C_CSGameRules
            if (Data.contains("C_CSGameRules"))
            {
                Offsets::m_bBombPlanted = C_CSGameRules["m_bBombPlanted"];
                Offsets::m_bBombDropped = C_CSGameRules["m_bBombDropped"];
                Offsets::m_bWarmupPeriod = C_CSGameRules["m_bWarmupPeriod"];
                Offsets::m_totalRoundsPlayed = C_CSGameRules["m_totalRoundsPlayed"];
            }

            // C_PlantedC4
            if (Data.contains("C_PlantedC4"))
            {
                Offsets::m_nBombSite = C_PlantedC4["m_nBombSite"];
                Offsets::m_bHasExploded = C_PlantedC4["m_bHasExploded"];
                Offsets::m_bBeingDefused = C_PlantedC4["m_bBeingDefused"];
                Offsets::m_flDefuseLength = C_PlantedC4["m_flDefuseLength"];
            }

            // CGameSceneNode
            if (Data.contains("CGameSceneNode"))
            {
                Offsets::m_vecAbsOrigin = CGameSceneNode["m_vecAbsOrigin"];
            }

            // EntitySpottedState_t
            if (Data.contains("EntitySpottedState_t "))
            {
                Offsets::m_bSpotted = EntitySpottedState_t["m_bSpotted"];
            }

            return true;
        }
    }
};

inline Updater updater; // Global instance of Updater
