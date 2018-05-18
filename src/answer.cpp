#include "answer.h"

#include <algorithm>
#include <vector>
#include "db_manager.h"

#include <iostream>

template <typename Iter>
std::string nextToken(Iter & it)
{
  std::string token;
  while(*it != '=')
  {
    ++it;
  }
  ++it;
  while((*it != '&') && (*it != '\n'))
  {
    token += *it++;
  }

  return token;
}

std::string answer(const std::string& request)
{
  if(request.empty())
  {
    return std::string();
  }
  const std::string action_str = "Action: ";
  const std::string delimetr = "\n\n";

  auto iter_begin = std::search(request.begin(), request.end(), action_str.begin(), action_str.end());
  iter_begin += action_str.size();

  auto iter_end = iter_begin;
  while(*iter_end != '\n')
  {
    ++iter_end;
  }

  const std::string action(iter_begin, iter_end);

  std::string answ("POST / HTTP/1.1\nHost: 127.0.0.1:8000\n\n");
  DataBaseManager* db_manager(&DataBaseManager::instance());

  std::cout << "DEBUG: |" << action << "|" << std::endl;

  if(action == "Check User")
  {
    std::string login(nextToken(iter_begin));
    std::string password(nextToken(iter_begin));
    if(db_manager->checkUser(login, password))
    {
      answ += "yes\n";
    }
    else
    {
      answ += "no\n";
    }
  }
  else if(action == "Check Free Login")
  {
    std::string login(nextToken(iter_begin));
    if(db_manager->checkFreeLogin(login))
    {
      answ += "yes\n";
    }
    else
    {
      answ += "no\n";
    }
  }
  else if(action == "Get Messages C")
  {
      std::string login1(nextToken(iter_begin)), login2(nextToken(iter_begin));
      std::string count(nextToken(iter_begin));
      pqxx::result messages = db_manager->selectMessages(db_manager->getId(login1),
                                                         db_manager->getId(login2),
                                                         stoul(count));
      answ = answ + std::to_string(messages.size()) + "|";
      for(auto it_l = messages.begin(); it_l != messages.end(); ++it_l)
      {
        auto it_c = it_l.begin();
        answ = answ + it_c->c_str() + '|';
        ++it_c;
        answ = answ + db_manager->getLogin(std::atoi(it_c->c_str())) + '|';
        ++it_c;
        answ = answ + it_c->c_str() + '|';
      }
  }
  else if(action == "Get Messages T")
  {
    //TO DO
  }
  else if(action == "Reg User")
  {
    std::string login(nextToken(iter_begin));
    std::string password(nextToken(iter_begin));

    db_manager->addUser(login, password);
    answ += "success";
  }
  else if(action == "Add Friend")
  {
    std::string login(nextToken(iter_begin));
    std::string log_f(nextToken(iter_begin));

    if(!db_manager->checkFreeLogin(login) && !db_manager->checkFreeLogin(log_f))
    {
      std::vector<DataBaseManager::id_t> friends(db_manager->getFriends(db_manager->getId(login)));
      if(std::find(friends.begin(), friends.end(), db_manager->getId(log_f)) == friends.end())
      {
        db_manager->addFriend(db_manager->getId(login), db_manager->getId(log_f));
        answ += "success";
      }
      else
      {
        answ += "alr";
      }
    }
    else
    {
      answ += "bad";
    }
  }
  else if(action == "Send Message")
  {
    std::string from(nextToken(iter_begin));
    std::string to(nextToken(iter_begin));
    std::string message(nextToken(iter_begin));

    db_manager->addMessage(db_manager->getId(from), db_manager->getId(to), message);
    answ += "success";
  }
  else if(action == "Get Friends")
  {
    std::string who(nextToken(iter_begin));
    std::vector<DataBaseManager::id_t> data(db_manager->getFriends(db_manager->getId(who)));
    answ += "|";
    for(auto i = data.begin(); i != data.end(); ++i)
    {
      answ = answ + db_manager->getLogin(*i) + "|";
    }
  }
  else
  {}

  return answ;
}
