#include "pch.h"
#include "Tester.h"
#include <DocumentStore.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <random>

namespace test
{
	static void set_for_fiddler(CURL* curl)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}

	bool Tester::runTest(const std::function<void()>& test, const std::string& testName)
	{
		try
		{
			test();
		}
		catch (std::exception& e)
		{
			//_resultStream << testName;
			//_resultStream << " FAILED with ERROR: \n" << e.what() << std::endl;

			std::cout << testName;
			std::cout << " FAILED with ERROR: \n" << e.what() << std::endl;
			return false;
		}
		std::cout << testName;
		std::cout << " SUCCEEDED" << std::endl;
		return true;
	}

	void Tester::addBooks(library::Library& lib)
	{
		std::vector<std::shared_ptr<library::Book>> books{};
		books.reserve(NUM_OF_BOOKS);
		_booksIds.reserve(NUM_OF_BOOKS);

		for (auto i = 1; i <= NUM_OF_BOOKS; ++i)
		{
			auto book = std::make_shared<library::Book>();
			book->author = "Someone" + std::to_string(i);
			book->title = "Something" + std::to_string(i);
			books.push_back(book);
		}
		lib.addBooks(books);
		std::transform(books.cbegin(), books.cend(), std::back_inserter(_booksIds),
			[](const auto& book)
		{
			return book->id;
		});

		if (auto numOfBooks = lib.getNumberOfBooks();
			numOfBooks < NUM_OF_BOOKS)
		{
			std::ostringstream msg;
			msg << numOfBooks << " books were stored instead of " << NUM_OF_BOOKS;
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::removeBooks(library::Library& lib)
	{
		lib.removeBooksById(_booksIds);
		_booksIds.clear();
		if (auto numOfBooks = lib.getNumberOfBooks();
			numOfBooks != 0)
		{
			std::ostringstream msg;
			msg << numOfBooks << " books were not removed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::addUsers(library::Library& lib)
	{
		std::vector<std::shared_ptr<library::LibraryUser>> users{};
		users.reserve(NUM_OF_USERS);
		_usersIds.reserve(NUM_OF_USERS);

		for (auto i = 1; i <= NUM_OF_USERS; ++i)
		{
			auto user = std::make_shared<library::LibraryUser>();
			user->firstName = "Someone" + std::to_string(i);
			user->lastName = "Else" + std::to_string(i);
			users.push_back(user);
		}
		lib.addUsers(users);
		std::transform(users.cbegin(), users.cend(), std::back_inserter(_usersIds),
			[](const auto& user)
		{
			return user->id;
		});

		if (auto numOfUsers = lib.getNumberOfUsers();
			numOfUsers < NUM_OF_USERS)
		{
			std::ostringstream msg;
			msg << numOfUsers << " users were registered instead of " << NUM_OF_USERS;
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::removeUsers(library::Library& lib)
	{
		lib.removeUsersById(_usersIds);
		_usersIds.clear();
		if (auto numOfUsers = lib.getNumberOfUsers();
			numOfUsers != 0)
		{
			std::ostringstream msg;
			msg << numOfUsers << " users were not unregistered";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::loanBook(library::Library& lib, const std::string& bookId, const std::string& userId)
	{
		if(lib.isBookLoaned(bookId) || !lib.tryLoanBook(bookId, userId))
		{
			std::ostringstream msg{};
			msg << "loan of book " << bookId << " by user " << userId << " failed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::returnBook(library::Library& lib, const std::string& bookId)
	{
		auto userId = lib.getBookById(bookId)->loanedBy.value_or("no_user");

		if(!lib.isBookLoaned(bookId) || !lib.tryReturnBook(bookId))
		{
			std::ostringstream msg{};
			msg << "return of the book " << bookId << " by user " << userId << " failed";
			throw std::runtime_error(msg.str());
		}
	}

	void Tester::updateBook(library::Library& lib, const std::string& bookId, const std::string& newAuthor,
		const std::string& newTitle)
	{
		lib.updateBookTitleAndAuthor(bookId, newTitle, newAuthor);
	}

	int Tester::get_rand(int min, int max)
	{
		auto lock = std::lock_guard(_rand_mutex);

		std::uniform_int_distribution<> dis(min, max);
		return dis(_generator);
	}

	Tester::Tester(const char* urls[], size_t numOfUrls, const char* dbName)
		: _store(ravendb::client::documents::DocumentStore::create(
			std::vector<std::string>(urls, urls + numOfUrls), dbName))
		, _library(_store)
		, _generator(_random_device())
	{
		REGISTER_ID_PROPERTY_FOR(library::Book, id);
		REGISTER_ID_PROPERTY_FOR(library::LibraryUser, id);
		//_store->set_before_perform(set_for_fiddler);

		_store->get_conventions()->set_max_http_cache_size(10*1024);

		_store->initialize();
	}

	std::shared_ptr<ravendb::client::documents::IDocumentStore> Tester::getStore() const
	{
		return _store;
	}

	void Tester::run()
	{
		using namespace std::chrono_literals;

		static std::once_flag flag{};
		std::call_once(flag, std::bind(&Tester::prepare, this, std::placeholders::_1), _library);

		for(auto i = 0; i < 100; ++i)
		{
			auto bookId = _booksIds[get_rand(0, NUM_OF_BOOKS-1)];
			auto userId = _usersIds[get_rand(0, NUM_OF_USERS-1)];

			auto loanSucceeded = runTest([this, &bookId, &userId]() {loanBook(_library, bookId, userId); }, "LoanBook");
			std::this_thread::sleep_for(1s);

			if(loanSucceeded) runTest([this, &bookId]() {returnBook(_library, bookId); }, "ReturnBook");
			std::this_thread::sleep_for(std::chrono::milliseconds(get_rand(500, 1500)));
		}

		for (auto i = 0; i < 100; ++i)
		{
			auto bookId = _booksIds[get_rand(0, NUM_OF_BOOKS-1)];
			auto newAuthor = std::to_string(get_rand(1, 512));
			auto newTitle = std::to_string(get_rand(1, 512));

			runTest([this, &bookId, &newAuthor, &newTitle]() {updateBook(_library, bookId, newAuthor, newTitle); }, "UpdateBook");
			std::this_thread::sleep_for(std::chrono::milliseconds(get_rand(500, 1500)));
		}

		for (auto i = 0; i < 100; ++i)
		{
			auto bookId = _booksIds[get_rand(0, NUM_OF_BOOKS-1)];
			auto userId = _usersIds[get_rand(0, NUM_OF_USERS-1)];

			auto loanSucceeded = runTest([this, &bookId, &userId]() {loanBook(_library, bookId, userId); }, "LoanBook");
			std::this_thread::sleep_for(std::chrono::milliseconds(get_rand(500, 1500)));

			if(loanSucceeded) runTest([this, &bookId]() {returnBook(_library, bookId); }, "ReturnBook");
			std::this_thread::sleep_for(std::chrono::milliseconds(get_rand(500, 1500)));
		}

		for (auto i = 0; i < 100; ++i)
		{
			auto bookId = _booksIds[get_rand(0, NUM_OF_BOOKS-1)];
			auto newAuthor = std::to_string(get_rand(1, 512));
			auto newTitle = std::to_string(get_rand(1, 512));

			runTest([this, &bookId, &newAuthor, &newTitle]() {updateBook(_library, bookId, newAuthor, newTitle); }, "UpdateBook");
			std::this_thread::sleep_for(std::chrono::milliseconds(get_rand(500, 1500)));
		}
	}

	void Tester::prepare(library::Library& library)
	{
		runTest([this, &library]() {addBooks(library); }, "AddingBooks");
		runTest([this, &library]() {addUsers(library); }, "AddingUsers");
	}

	void Tester::runInParallel(uint32_t num_of_threads)
	{
		auto run_threads = std::vector<std::thread>{};
		run_threads.reserve(num_of_threads);

		for(uint32_t i = 0; i < num_of_threads; ++i)
		{
			run_threads.emplace_back([this] {run(); });
		}

		for(auto& thread : run_threads)
		{
			thread.join();
		}

		runTest([this]() {removeBooks(_library); }, "RemovingBooks");
		runTest([this]() {removeUsers(_library); }, "RemovingUsers");
	}
}
