#include "Highscore.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../../imports.h"

using json = nlohmann::json;

std::map<std::string, int> Highscore::songScores;
std::map<std::string, float> Highscore::songAccuracies;

void Highscore::saveScore(const std::string& song, int score, int diff, float accuracy) {
    std::string daSong = formatSong(song, diff);
    
    if (songScores.find(daSong) != songScores.end()) {
        if (songScores[daSong] < score) {
            setScore(daSong, score);
        }
        if (songAccuracies.find(daSong) != songAccuracies.end()) {
            if (songAccuracies[daSong] < accuracy) {
                setAccuracy(daSong, accuracy);
            }
        } else {
            setAccuracy(daSong, accuracy);
        }
    } else {
        setScore(daSong, score);
        setAccuracy(daSong, accuracy);
    }
}

void Highscore::saveWeekScore(int week, int score, int diff, float accuracy) {
    std::string daWeek = formatSong("week" + std::to_string(week), diff);
    
    if (songScores.find(daWeek) != songScores.end()) {
        if (songScores[daWeek] < score) {
            setScore(daWeek, score);
        }
    } else {
        setScore(daWeek, score);
    }
}

void Highscore::setScore(const std::string& song, int score) {
    songScores[song] = score;
    save();
}

std::string Highscore::formatSong(const std::string& song, int diff) {
    std::string daSong = song;
    
    if (diff == 0) {
        daSong += "-easy";
    } else if (diff == 2) {
        daSong += "-hard";
    }
    
    return daSong;
}

int Highscore::getScore(const std::string& song, int diff) {
    std::string formattedSong = formatSong(song, diff);
    
    if (songScores.find(formattedSong) != songScores.end()) {
        return songScores[formattedSong];
    }
    
    return 0;
}

float Highscore::getAccuracy(const std::string& song, int diff) {
    std::string formattedSong = formatSong(song, diff);
    
    if (songAccuracies.find(formattedSong) != songAccuracies.end()) {
        return songAccuracies[formattedSong];
    }
    
    return 0.0f;
}

int Highscore::getWeekScore(int week, int diff) {
    std::string weekKey = formatSong("week" + std::to_string(week), diff);
    
    if (songScores.find(weekKey) == songScores.end()) {
        setScore(weekKey, 0);
    }
    
    return songScores[weekKey];
}

void Highscore::load() {
    std::ifstream file(ASSETS_PATH "assets/data/highscores.json");
    
    if (!file.is_open()) {
        return;
    }
    
    try {
        json saveData;
        file >> saveData;
        file.close();
        
        if (saveData.contains("songScores") && saveData["songScores"].is_object()) {
            for (auto& [key, value] : saveData["songScores"].items()) {
                if (value.is_number_integer()) {
                    songScores[key] = value.get<int>();
                }
            }
        }
        
        if (saveData.contains("songAccuracies") && saveData["songAccuracies"].is_object()) {
            for (auto& [key, value] : saveData["songAccuracies"].items()) {
                if (value.is_number_float() || value.is_number_integer()) {
                    songAccuracies[key] = value.get<float>();
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading highscores: " << e.what() << std::endl;
    }
}

void Highscore::save() {    
    json saveData;
    
    json scoresJson = json::object();
    for (const auto& [key, value] : songScores) {
        scoresJson[key] = value;
    }
    saveData["songScores"] = scoresJson;
    
    json accuraciesJson = json::object();
    for (const auto& [key, value] : songAccuracies) {
        accuraciesJson[key] = value;
    }
    saveData["songAccuracies"] = accuraciesJson;
    
    std::ofstream file(ASSETS_PATH "assets/data/highscores.json");
    if (file.is_open()) {
        file << saveData.dump(4);
        file.close();
    } else {
        std::cerr << "Error: Could not save highscores to file" << std::endl;
    }
}

void Highscore::setAccuracy(const std::string& song, float accuracy) {
    songAccuracies[song] = accuracy;
    save();
}

void Highscore::printAllAccuracies() {
    std::cout << "All saved accuracies:" << std::endl;
    for (const auto& [song, accuracy] : songAccuracies) {
        std::cout << song << ": " << accuracy << std::endl;
    }
}