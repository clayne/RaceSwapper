#pragma once
#include "Configuration.h"
#include "Utils.h"

// TODO: Use merge mapper API when looking up form IDs from the config files for VR users
void ConfigurationDatabase::Initialize() {
	logger::info("Reading config APIs...");
	// TODO: Change this path later?
	constexpr auto path = L"Data/SKSE/Plugins/RaceSwap";
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		logger::info("Parsing file {}", entry.path().string().c_str());
		std::fstream config;
		config.open(entry.path(), std::ios::in);
		if (!config.is_open()) {
			logger::error("Couldn't open file {}", entry.path().string().c_str());
			continue;
		}
		std::string line;
		while (std::getline(config, line)) {
			auto configEntry = ConfigurationEntry::ConstructNewEntry(line);
			if (configEntry) {
				entries.push_back(configEntry);
			}
		}
	}

	logger::info("Config APIs fully parsed!");
}

ConfigurationEntry* PickRandomWeightedEntry(std::vector<std::pair<std::uint32_t, ConfigurationEntry*>> a_entries, RE::TESNPC* a_npc)
{
	if (a_entries.empty()) {
		return nullptr;	
	}
	if (a_entries.size() == 1) {
		return a_entries[0].second;
	}
	std::vector<std::uint32_t> weights(a_entries.size(), 0);
	weights[0] = a_entries[0].first;
	for (std::uint32_t i = 1; i < a_entries.size(); i++) {
		weights[i] = a_entries[i].first + weights[i-1]; 
	}

	auto seed = utils::HashForm(a_npc);
	srand((int) seed);
	auto index = std::upper_bound(weights.begin(), weights.end(), rand() % weights.back()) - weights.begin();
	return a_entries[index].second;
}

AppearanceConfiguration* ConfigurationDatabase::GetConfigurationForNPC(RE::TESNPC* a_npc) {
	std::vector<std::pair<std::uint32_t, ConfigurationEntry*>> npcSwapEntries;
	std::vector<std::pair<std::uint32_t, ConfigurationEntry*>> raceSwapEntries;
	for (auto entry : entries) {
		if (entry->MatchesNPC(a_npc)) {
			if (entry->entryData.otherNPC) {
				npcSwapEntries.push_back({ entry->entryData.weight, entry });
			} else if (entry->entryData.otherRace) {
				raceSwapEntries.push_back({ entry->entryData.weight, entry });
			}
		}
	}

	if (!npcSwapEntries.empty()) {
		auto config = new AppearanceConfiguration{ 0 };
		config->otherNPC = PickRandomWeightedEntry(npcSwapEntries, a_npc)->entryData.otherNPC;
		return config;
	}

	if (!raceSwapEntries.empty()) {
		auto config = new AppearanceConfiguration{ 0 };
		config->otherRace = PickRandomWeightedEntry(raceSwapEntries, a_npc)->entryData.otherRace;
		return config;
	}

	return nullptr;
}
