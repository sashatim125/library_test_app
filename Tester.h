#pragma once
#include "raven_cpp_client.h"
#include "Library.h"
#include "LibraryUser.h"
#include "Book.h"
#include <sstream>
#include <random>

namespace test
{
	class Tester
	{
	private:
		//std::ostringstream _resultStream{};

		std::shared_ptr<ravendb::client::documents::DocumentStore> _store{};

		std::vector<std::string> _booksIds{};
		std::vector<std::string> _usersIds{};

		static constexpr auto NUM_OF_BOOKS = 1000;
		static constexpr auto NUM_OF_USERS = 100;

		library::Library _library;

		std::mutex _rand_mutex{};
		std::random_device _random_device;;  //Will be used to obtain a seed for the random number engine
		std::mt19937 _generator; //Standard mersenne_twister_engine seeded with rd()

	private:
		bool runTest(const std::function<void()>& test, const std::string& testName);

		void addBooks(library::Library& lib);
		void removeBooks(library::Library& lib);
		void addUsers(library::Library& lib);
		void removeUsers(library::Library& lib);

		void loanBook(library::Library& lib, const std::string& bookId, const std::string& userId);
		void returnBook(library::Library& lib, const std::string& bookId);
		void updateBook(library::Library& lib, const std::string& bookId,
			const std::string& newAuthor,
			const std::string& newTitle);

		int get_rand(int min, int max);

	public:
		Tester(const char* urls[], size_t numOfUrls, const char* dbName);

		~Tester() = default;

		std::shared_ptr<ravendb::client::documents::IDocumentStore> getStore() const;

		void run();

		void prepare(library::Library& library);

		void runInParallel(uint32_t num_of_threads);

//		void fillResultBuffer(char* resultBuffer, uint64_t* size);
	};
}
