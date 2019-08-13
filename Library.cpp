#include "pch.h"
#include "raven_cpp_client.h"
#include "Library.h"

namespace library
{
	Library::Library(std::shared_ptr<ravendb::client::documents::IDocumentStore> store)
		:_store(store)
	{}

	void Library::addUser(std::shared_ptr<LibraryUser> user)
	{
		if (!user)
		{
			throw std::invalid_argument("'user' can't be empty");
		}

		if (!user->id.empty() && exists(user->id))
		{
			return;
		}

		auto session = _store->open_session();
		session.store(user);
		session.save_changes();
	}

	void Library::addUsers(const std::vector<std::shared_ptr<LibraryUser>>& users)
	{
		auto session = _store->open_session();
		for(const auto& user : users)
		{
			if(!user)
			{
				continue;
			}
			session.store(user);
		}
		session.save_changes();
	}

	std::shared_ptr<LibraryUser> Library::getUserById(const std::string& userId)
	{
		return _store->open_session().load<LibraryUser>(userId);
	}

	void Library::removeUserById(const std::string& userId)
	{
		auto session = _store->open_session();
		session.delete_document(userId);
		session.save_changes();
	}

	void Library::removeUsersById(const std::vector<std::string>& userIds)
	{
		auto session = _store->open_session();
		for(const auto& id : userIds )
		{
			session.delete_document(id);
		}
		session.save_changes();
	}

	uint64_t Library::getNumberOfUsers()
	{
		auto res = _store->maintenance()->send(ravendb::client::documents::operations::indexes::GetCollectionStatisticsOperation());
		return res->collections.at("LibraryUsers");
	}

	bool Library::exists(const std::string& id)
	{
		return _store->open_session().advanced().exists(id);
	}

	void Library::addBook(std::shared_ptr<Book> book)
	{
		if(!book)
		{
			throw std::invalid_argument("'book' can't be empty");
		}

		if(!book->id.empty() && exists(book->id))
		{
			return;
		}

		auto session = _store->open_session();
		session.store(book);
		session.save_changes();			
	}

	void Library::addBooks(const std::vector<std::shared_ptr<Book>> books)
	{
		auto session = _store->open_session();
		for (const auto& book : books)
		{
			session.store(book);
		}
		session.save_changes();
	}

	std::shared_ptr<Book> Library::getBookById(const std::string& bookId)
	{
		return _store->open_session().load<Book>(bookId);
	}

	std::shared_ptr<Book> Library::getBookByISBN(const std::string& ISBN)
	{
		return _store->open_session().query<Book>()
			->wait_for_non_stale_results()
			->where_equals("ISBN", ISBN)
			->single_or_default();
	}

	void Library::removeBookById(const std::string& bookId)
	{
		auto session = _store->open_session();
		session.delete_document(bookId);
		session.save_changes();
	}

	void Library::removeBooksById(const std::vector<std::string>& bookIds)
	{
		auto session = _store->open_session();
		for (const auto& id : bookIds)
		{
			session.delete_document(id);
		}
		session.save_changes();
	}

	uint64_t Library::getNumberOfBooks()
	{
		auto res = _store->maintenance()->send(ravendb::client::documents::operations::indexes::GetCollectionStatisticsOperation());
		return res->collections.at("Books");
	}

	std::shared_ptr<Book> Library::getBookByTitleAndAuthor(const std::string& title, const std::string& author)
	{
		return _store->open_session().query<Book>()
			->wait_for_non_stale_results()
			->where_equals("Title", title)
			->and_also()
			->where_equals("Author", author)
			->single_or_default();
	}

	std::vector<std::shared_ptr<Book>> Library::getBooksByAuthor(const std::string& author)
	{
		return _store->open_session().query<Book>()
			->wait_for_non_stale_results()
			->where_equals("Author", author)
			->to_list();
	}

	void Library::updateBookTitle(const std::string& bookId, const std::string& newTitle)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Title", newTitle);
		session.save_changes();
	}

	void Library::updateBookAuthor(const std::string& bookId, const std::string& newAuthor)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Author", newAuthor);
		session.save_changes();
	}

	void Library::updateBookTitleAndAuthor(const std::string& bookId, const std::string& newTitle,
		const std::string& newAuthor)
	{
		auto session = _store->open_session();
		session.advanced().patch(bookId, "Title", newTitle);
		session.advanced().patch(bookId, "Author", newAuthor);
		session.save_changes();
	}

	bool Library::tryLoanBook(const std::string& bookId, const std::string& userId)
	{
		auto session = _store->open_session();
		auto book = session.load<Book>(bookId);
		if(!book)
		{
			return false;
		}
		if(book->isLoaned)
		{
			return false;
		}

		auto user = session.load<LibraryUser>(userId);
		if (!user)
		{
			return false;
		}

		user->loanedBooksIds.insert(bookId);
		book->isLoaned = true;
		book->loanedBy = userId;

		using namespace std::chrono;

		auto now = system_clock::now();
		std::time_t time = system_clock::to_time_t(now);
		std::tm date_time{};
		gmtime_s(&date_time, &time);

		book->loanDate = ravendb::client::impl::DateTimeOffset({ date_time, 0L, std::time_t(0) });

		session.save_changes();

		return true;
	}

	bool Library::tryReturnBook(const std::string& bookId)
	{
		auto session = _store->open_session();
		auto book = session.include("LoanedBy").load<Book>(bookId);

		if (!book || !book->isLoaned)
		{
			return false;
		}

		auto user = session.load<LibraryUser>(book->loanedBy.value());
		if (!user)
		{
			return false;
		}

		user->loanedBooksIds.erase(bookId);
		book->isLoaned = false;
		book->loanedBy.reset();
		book->loanDate.reset();
		session.save_changes();

		return true;
	}

	void Library::returnBooks(const std::vector<std::string>& bookIds)
	{
		auto session = _store->open_session();
		for (const auto& bookId : bookIds)
		{
			auto book = session.include("LoanedBy").load<Book>(bookId);

			if(!book)
			{
				continue;
			}

			auto user = session.load<LibraryUser>(*book->loanedBy);
			if (user)
			{
				user->loanedBooksIds.erase(bookId);
			}
			book->isLoaned = false;
			book->loanedBy.reset();
			book->loanDate.reset();
		}
		session.save_changes();
	}

	std::vector<std::shared_ptr<Book>> Library::getAllLoanedBooks()
	{
		return _store->open_session().query<Book>()
			->wait_for_non_stale_results()
			->where_equals("IsLoaned", true)
			->order_by("LoanDate")
			->to_list();
	}

	bool Library::isBookLoaned(const std::string& bookId)
	{
		auto res = _store->open_session().query<Book>()
			->where_equals("id()", bookId)
			->select_fields<bool>({ "IsLoaned" })
			->single();
		return res && *res;
	}

	bool Library::isBookLoanedByUser(const std::string& bookId, const std::string& userId)
	{
		auto res = _store->open_session().query<Book>()
			->where_equals("id()", bookId)
			->select_fields<std::string>({ "LoanedBy" })
			->single();
		return res && *res == userId;
	}

	bool Library::hasUserLoanedBook(const std::string& bookId, const std::string& userId)
	{
		auto res = _store->open_session().query<LibraryUser>()
			->where_equals("id()", userId)
			->search("LoanedBooksIds", bookId)			
			->count();
		return res > 0;
	}
}
