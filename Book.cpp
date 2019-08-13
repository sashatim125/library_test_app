#include "pch.h"
#include "Book.h"
#include <json_utils.h>

namespace library
{
	void to_json(nlohmann::json& j, const Book& b)
	{
		using ravendb::client::impl::utils::json_utils::set_val_to_json;

		set_val_to_json(j, "Title", b.title);
		set_val_to_json(j, "Author", b.author);
		set_val_to_json(j, "ISBN", b.ISBN);
		set_val_to_json(j, "IsLoaned", b.isLoaned);
		set_val_to_json(j, "LoanedBy", b.loanedBy);
		set_val_to_json(j, "LoanDate", b.loanDate);
	}

	void from_json(const nlohmann::json& j, Book& b)
	{
		using ravendb::client::impl::utils::json_utils::get_val_from_json;

		get_val_from_json(j, "Title", b.title);
		get_val_from_json(j, "Author", b.author);
		get_val_from_json(j, "ISBN", b.ISBN);
		get_val_from_json(j, "IsLoaned", b.isLoaned);
		get_val_from_json(j, "LoanedBy", b.loanedBy);
		get_val_from_json(j, "LoanDate", b.loanDate);
	}
}
