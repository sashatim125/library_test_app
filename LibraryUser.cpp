#include "pch.h"
#include "LibraryUser.h"
#include "json_utils.h"

namespace library
{
	void to_json(nlohmann::json& j, const LibraryUser& u)
	{
		using ravendb::client::impl::utils::json_utils::set_val_to_json;

		set_val_to_json(j, "FirstName", u.firstName);
		set_val_to_json(j, "LastName", u.lastName);
		set_val_to_json(j, "LoanedBooksIds", u.loanedBooksIds);
	}

	void from_json(const nlohmann::json& j, LibraryUser& u)
	{
		using ravendb::client::impl::utils::json_utils::get_val_from_json;

		get_val_from_json(j, "FirstName", u.firstName);
		get_val_from_json(j, "LastName", u.lastName);
		get_val_from_json(j, "LoanedBooksIds", u.loanedBooksIds);
	}
}
