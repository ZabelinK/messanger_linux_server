#include "db_manager.h"

#include <mutex>
#include <ctime>
#include <cstdio>
#include <memory>
#include <algorithm>
#include <iterator>
#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/cursor>


DataBaseManager::DataBaseManager():
  con_("user='postgres' password='12345' host='localhost' port='5432' dbname='test_userdb'")
{
   // DataBaseManager::readConfFile()
  DataBaseManager::createUsersTable();
}

DataBaseManager& DataBaseManager::instance()
{
  static DataBaseManager db_manager;
  return db_manager;
}

pqxx::result DataBaseManager::createUsersTable()
{
  return work("CREATE TABLE IF NOT EXISTS users "
             "(id SERIAL PRIMARY KEY, "
             "login VARCHAR(100), "
             "password VARCHAR(100));");
}

pqxx::result DataBaseManager::createFriendsList(DataBaseManager::id_t id)
{
  return work("CREATE TABLE IF NOT EXISTS friends_" + std::to_string(id)
              + " (friend_id INTEGER);");
}

pqxx::result DataBaseManager::createDialogTable(DataBaseManager::id_t id_1, DataBaseManager::id_t id_2)
{
  return work("CREATE TABLE IF NOT EXISTS "
              + DataBaseManager::guitDiaIdId(id_1, id_2)
              + " (msg_time timestamp without time zone, sender_id INTEGER, message TEXT);");
}

pqxx::result DataBaseManager::addUser(const std::string &login, const std::string &password)
{
  if(DataBaseManager::checkFreeLogin(login))
  {
    work("INSERT INTO users (login, password) VALUES ('"
                + login + "', '"
                + password + "');");
    return DataBaseManager::createFriendsList(getId(login));
  }
  return work(";", "SampleSelect");
}

pqxx::result DataBaseManager::deleteUser(const std::string &login)
{
  return work("DELETE FROM users WHERE login = '" + login + "';");
}

pqxx::result DataBaseManager::deleteUser(DataBaseManager::id_t id)
{
  return work("DELETE FROM users WHERE id = " + std::to_string(id) + ";");
}

pqxx::result DataBaseManager::addFriend(DataBaseManager::id_t id_user, DataBaseManager::id_t id_friend)
{
  std::vector<DataBaseManager::id_t> friends = DataBaseManager::getFriends(id_user);
  if(std::find(friends.begin(), friends.end(), id_friend) == friends.end()
     && id_user != id_friend)
  {
    DataBaseManager::createDialogTable(id_user, id_friend);
    work("INSERT INTO friends_" + std::to_string(id_friend) + " (friend_id) VALUES (" + std::to_string(id_user) + ");");
    return work("INSERT INTO friends_" + std::to_string(id_user)
              + " (friend_id) VALUES (" + std::to_string(id_friend) + ");");
  }
  else
  {
    return work(";");
  }
}

pqxx::result DataBaseManager::deleteFriend(DataBaseManager::id_t id_user,
                                           DataBaseManager::id_t id_friend)
{
  return work("DELETE FROM friends_" + std::to_string(id_user)
              + " WHERE friend_id = " + std::to_string(id_friend) + ";");
}

pqxx::result DataBaseManager::addMessage(DataBaseManager::id_t from_user,
                                         DataBaseManager::id_t to_user,
                                         const std::string &message)
{
  return work("INSERT INTO " + DataBaseManager::guitDiaIdId(from_user, to_user)
              + "(msg_time, sender_id, message) VALUES ('"
              + DataBaseManager::toStrNowTime() + "', "
              + std::to_string(from_user) + ", '" + message + "');");
}

pqxx::result DataBaseManager::selectMessages(DataBaseManager::id_t id_1,
                                             DataBaseManager::id_t id_2)
{
  return work("SELECT * FROM " + DataBaseManager::guitDiaIdId(id_1, id_2) + ";", "SampleSelect");
}

pqxx::result DataBaseManager::selectMessages(DataBaseManager::id_t id_1,
                                             DataBaseManager::id_t id_2,
                                             size_t count)
{
  return work("SELECT * FROM "
              + DataBaseManager::guitDiaIdId(id_1, id_2)
              +  " ORDER BY msg_time DESC LIMIT " + std::to_string(count) + ";", "SampleSelect");
}

DataBaseManager::id_t DataBaseManager::getId(const std::string &login)
{
  return std::atoi(work("SELECT id FROM users WHERE login = '"
                        + login + "';", "SampleSelect").begin().begin().c_str());
}

std::string DataBaseManager::getLogin(DataBaseManager::id_t id)
{
  return work("SELECT login FROM users WHERE id = "
              + std::to_string(id) + ";", "SampleSelect").begin().begin().c_str();
}

std::string DataBaseManager::getPassword(DataBaseManager::id_t id)
{
  return work("SELECT password FROM users WHERE id = "
              + std::to_string(id) + ";", "SampleSelect").begin().begin().c_str();
}

std::vector<DataBaseManager::id_t> DataBaseManager::getFriends(DataBaseManager::id_t user_id)
{
  pqxx::result ids = work("SELECT * FROM friends_" + std::to_string(user_id) + ";");
  std::vector<DataBaseManager::id_t> data;
  std::transform(ids.begin(), ids.end(), std::back_insert_iterator<std::vector<DataBaseManager::id_t>>(data), [](auto it)
  {
    return std::atoi(it.begin().c_str());
  });
  return data;
}

bool DataBaseManager::checkFreeLogin(const std::string &login)
{
  return work("SELECT id FROM users WHERE login = '" + login + "';", "SampleSelect").empty();
}

bool DataBaseManager::checkUser(const std::string &login, const std::string &password)
{
  return !work("SELECT id FROM users WHERE login = '" + login
               + "' AND password = '" + password + "';", "SampleSelect").empty();
}

DataBaseManager::~DataBaseManager()
{
  con_.disconnect();
  con_.deactivate();
}

std::string DataBaseManager::readConfFile() const
{
  std::unique_ptr<FILE, decltype(&fclose)> conf_file(fopen("db_config", "r"), &fclose);

  if(!conf_file)
  {
    throw std::logic_error("Сonfigurational file for database \"db_config\" is missing");
  }

  fseek(conf_file.get(), 0, SEEK_END);
  std::string config(ftell(conf_file.get()), ' ');
  rewind(conf_file.get());

  if(fread(&(*config.begin()), sizeof(char), config.size(), conf_file.get()) != config.size())
  {
    throw std::logic_error("Configurational file \"db_config\" changed value during the program work");
  }

  return config;
}

std::string DataBaseManager::guitDiaIdId(DataBaseManager::id_t id_1, DataBaseManager::id_t id_2) const
{
  return " dialog_" + std::to_string(std::min(id_1, id_2)) + "_" + std::to_string(std::max(id_1, id_2)) + " ";
}

std::string DataBaseManager::toStrNowTime() const
{
  std::string time_s(20, ' ');
  std::time_t t = time(nullptr);
  std::strftime(&(*time_s.begin()), 20, "%Y-%m-%d %H:%M:%S", std::localtime(&t));
  time_s.pop_back();
  return time_s;
}

pqxx::result DataBaseManager::work(const std::string &query, const std::string &type)
{
  std::lock_guard<std::mutex> lock(db_mutex);
  pqxx::work w(con_, type);
  if(type == "Sample")
  {
    pqxx::result res = w.exec(query);
    w.commit();
    return res;
  }
  return w.exec(query);
}
