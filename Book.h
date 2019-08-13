#pragma once
#include <string>
#include "json.hpp"
#include <optional>
#include <DateTimeOffset.h>

namespace library
{
	struct Book
	{
		std::string id{};

		std::string title{};
		std::string author{};
		std::string ISBN{};
		bool isLoaned = false;
		std::optional<std::string> loanedBy{};
		std::optional<ravendb::client::impl::DateTimeOffset> loanDate{};
	};

	void to_json(nlohmann::json& j, const Book& b);

	void from_json(const nlohmann::json& j, Book& b);
}
