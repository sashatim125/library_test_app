#pragma once
#include "Book.h"
#include <IDocumentStore.h>
#include "LibraryUser.h"

namespace library
{
	class Library
	{
	private:
		const std::shared_ptr<ravendb::client::documents::IDocumentStore> _store;

	public:
		explicit Library(std::shared_ptr<ravendb::client::documents::IDocumentStore> store);

		void addUser(std::shared_ptr<LibraryUser> user);
		void addUsers(const std::vector<std::shared_ptr<LibraryUser>>& users);
		std::shared_ptr<LibraryUser> getUserById(const std::string& userId);
		void removeUserById(const std::string& userId);
		void removeUsersById(const std::vector<std::string>& userIds);
		uint64_t getNumberOfUsers();

		bool exists(const std::string& id);

		void addBook(std::shared_ptr<Book> book);
		void addBooks(const std::vector<std::shared_ptr<Book>> books);
		void removeBookById(const std::string& bookId);
		void removeBooksById(const std::vector<std::string>& bookIds);
		uint64_t getNumberOfBooks();

		std::shared_ptr<Book> getBookById(const std::string& bookId);
		std::shared_ptr<Book> getBookByISBN(const std::string& ISBN);
		std::shared_ptr<Book> getBookByTitleAndAuthor(const std::string& title, const std::string& author);
		std::vector<std::shared_ptr<Book>> getBooksByAuthor(const std::string& author);
		

		void updateBookTitle(const std::string& bookId, const std::string& newTitle);
		void updateBookAuthor(const std::string& bookId, const std::string& newAuthor);
		void updateBookTitleAndAuthor(const std::string& bookId,
			const std::string& newTitle,
			const std::string& newAuthor);

		bool tryLoanBook(const std::string& bookId, const std::string& userId);
		bool tryReturnBook(const std::string& bookId);
		void returnBooks(const std::vector<std::string>& bookIds);

		std::vector<std::shared_ptr<Book>> getAllLoanedBooks();
		bool isBookLoaned(const std::string& bookId);
		bool isBookLoanedByUser(const std::string& bookId, const std::string& userId);
		bool hasUserLoanedBook(const std::string& bookId, const std::string& userId);
	};
}
