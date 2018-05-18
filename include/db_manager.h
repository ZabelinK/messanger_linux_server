#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <pqxx/pqxx>
#include <string>
#include <mutex>
#include <vector>

//Singltone

static std::mutex db_mutex;

class DataBaseManager
{
public:
  typedef unsigned long long id_t;

  static DataBaseManager& instance();

  pqxx::result createUsersTable();
  pqxx::result createFriendsList(id_t id);
  pqxx::result createDialogTable(id_t id_1, id_t id_2);

  pqxx::result addUser(const std::string &login, const std::string &password);
  pqxx::result deleteUser(const std::string &login);
  pqxx::result deleteUser(id_t id);

  pqxx::result addFriend(id_t id_user, id_t id_friend);
  pqxx::result deleteFriend(id_t id_user, id_t id_friend);

  pqxx::result addMessage(id_t from_user, id_t to_user, const std::string &message);

  pqxx::result selectMessages(id_t id_1, id_t id_2);
  pqxx::result selectMessages(id_t id_1, id_t id_2, size_t count);

  id_t getId(const std::string &login);
  std::string getLogin(id_t id);
  std::string getPassword(id_t id);
  std::vector<id_t> getFriends(id_t user_id);

  bool checkFreeLogin(const std::string &login);
  bool checkUser(const std::string &login, const std::string &password);

  DataBaseManager(const DataBaseManager &) = delete;
  DataBaseManager(DataBaseManager &&) = delete;
  DataBaseManager& operator=(const DataBaseManager &) = delete;
  DataBaseManager& operator=(DataBaseManager &&) = delete;

private:
  pqxx::connection con_;

  DataBaseManager();
  ~DataBaseManager();

  std::string readConfFile() const;
  std::string guitDiaIdId(id_t id1, id_t id2) const;
  std::string toStrNowTime() const;
  pqxx::result work(const std::string &query, const std::string &type = "Sample");
};

#endif // DB_MANAGER_H
