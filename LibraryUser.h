#pragma once
#include <string>
#include <unordered_set>
#include "json.hpp"

namespace library
{
	struct LibraryUser
	{
		std::string id;
		std::string firstName{};
		std::string lastName{};

		std::unordered_set<std::string> loanedBooksIds{};
	};

	void to_json(nlohmann::json& j, const LibraryUser& u);

	void from_json(const nlohmann::json& j, LibraryUser& u);
}
