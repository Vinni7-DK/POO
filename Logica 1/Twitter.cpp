#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <exception>
#include <list>

namespace aux {
    double number(std::string text) {
        std::stringstream ss(text);
        double value {};
        if (ss >> value) {
            return value;
        }
        std::cout << "fail: (" << text << ") is not a number\n";
        return 0.0;
    }

    double operator+(std::string text) {
        return number(text);
    }

    std::vector<std::string> split(std::string line, char delimiter = ' ') {
        std::stringstream ss(line);
        std::vector<std::string> result;
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            result.push_back(token);
        }
        return result;
    }

    template <class T, class FN> std::string join(T container, std::string sep, FN fn) { 
        std::stringstream ss;
        for (auto it = container.begin(); it != container.end(); ++it) {
            ss << (it == container.begin() ? "" : sep) << fn(*it);
        }
        return ss.str();
    }

    template <class T> std::string join(T container, std::string sep = ", ") {
        return join(container, sep, [](auto item) { return item; });
    }

    std::string input() {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    template <class T> void write(T data, std::string end = "\n") {
        std::cout << data << end;
    }

    template <class DATA>
    std::string format(std::string format, DATA data) {
        char buffer[100];
        sprintf(buffer, format.c_str(), data);
        return buffer;
    }

    template<typename CONTAINER, typename FUNCTION>
    auto map(CONTAINER container, FUNCTION fn) {
        std::vector<decltype(fn(*container.begin()))> aux;
        std::transform(container.begin(), container.end(), std::back_inserter(aux), fn);
        return aux;
    }

    template<typename DATA>
    std::vector<DATA> slice(std::vector<DATA> container, int start, int end) {
        int size = container.size();
        if (start < 0) start = size + start;
        if (end < 0) end = size + end;
        std::vector<DATA> aux;
        for (int i = start; i < end; i++) {
            aux.push_back(container[i]);
        }
        return aux;
    }

    template<typename DATA>
    std::vector<DATA> slice(std::vector<DATA> container, int start = 0) {
        return slice(container, start, container.size());
    }

}


class TweetException : public std::exception {
    std::string message;
public:
    TweetException(const std::string& message) : 
        message(message) {
    }
    const char* what() const noexcept override {
        return message.c_str(); 
    }
};

class Tweet {
    int id;
    std::string username;
    std::string msg;
    std::set<std::string> likes;

    Tweet *rt {nullptr};

public:

    Tweet(int id, const std::string& username, const std::string& msg) {
        this->id = id;
        this->username = username;
        this->msg = msg; 
    }

    int getId() const {
        return this->id;
    }

    std::string getSender() const {
        return this->username;
    }

    std::string getMsg() const {
        return this->msg;
    }

    std::string str() const {
        if(!(likes.empty()))
            return std::to_string(this->id) + ":" + this->username + ": " + "(" + this->msg + ")" + " " + "[" + aux::join(likes, ", ") + "]";
        if (rt != nullptr)
            return "     " + rt->str();
        return std::to_string(this->id) + ":" + this->username + ": " + "(" + this->msg + ")";
    }

    // __like__

    void like(const std::string& username) {
        if (likes.find(username) == likes.end()) {
            likes.insert(username);
            return;
        }
        throw TweetException("fail: " + username + " voce ja curtiu essa mensagem");
    }

    std::set<std::string> getLikes() const {
        return this->likes;
    }
    
    // __retweet__

    void setRt(Tweet *rt) {
        this->rt = rt;
    }
    
    // __remover__

    bool deleted {false};
    
    void setDeleted() {
        this->deleted = true;
    }

    bool isDeleted() const {
        return this->deleted;
    }
};

std::ostream& operator<<(std::ostream& os, const Tweet& msg) {
    return os << msg.str();
}

std::ostream& operator<<(std::ostream& os, Tweet* msg) {
    return os << msg->str();
}
//++

class Inbox {

    std::map<int, Tweet*> timeline;

    std::map<int, Tweet*> myTweets;
    
public:
    Inbox() {
    }

    void storeInTimeline(Tweet* tweet) {
        timeline[tweet->getId()] = tweet;
    }

    std::vector<Tweet*> getTimeline() const {
        std::vector<Tweet*> messages;
        for (auto it = timeline.rbegin(); it != timeline.rend(); ++it) {
            if (it->second->isDeleted() == false)
                messages.push_back(it->second);
        }
        return messages;
    }

    friend std::ostream& operator<<(std::ostream& os, const Inbox& inbox) {
        return os << aux::join(inbox.getTimeline(), "\n");
    }

    // __like__
    Tweet* getTweet(int id) {
        if (timeline.find(id) != timeline.end())
            return timeline[id];
        throw TweetException("fail: tweet nao existe");
    }

    // __unfollow__

    void rmMsgsFrom(const std::string& username) {
        auto keysToRemove = std::vector<int>();
        for (auto it = timeline.begin(); it != timeline.end(); ++it) {
            if (it->second->getSender() == username)
                keysToRemove.push_back(it->first);
        }
        for (auto key : keysToRemove)
            timeline.erase(key);
    }

    // __remover__

    void storeInMyTweets(Tweet* tweet) {
        myTweets[tweet->getId()] = tweet;
    }

    std::vector<Tweet*> getMyTweets() const {
        std::vector<Tweet*> messages;
        for (auto it = myTweets.rbegin(); it != myTweets.rend(); ++it)
            messages.push_back(it->second);
        return messages;
    }
};

class User {

    std::string username;
    Inbox inbox;
    std::map<std::string, User*> followers;
    std::map<std::string, User*> following;

    // __
public:

    User(const std::string& username) : 
        username(username) {
    }

    // ' mostra o nome dos seguidos e o nome dos seguires
    // + toString(): str
    std::string str() {
        std::stringstream ss;
        ss << username << "\n" 
            << "  seguidos   [" << aux::join(aux::map(following, [](auto p) {return p.first;})) << "]\n"
            << "  seguidores [" << aux::join(aux::map(followers, [](auto p) {return p.first;})) << "]";
        return ss.str();
    }

    // __follow__
    void follow(User * other) {
        if(other == this)
            throw TweetException("You cannot follow yourself");
        if(following.count(other->username) == 1)
            throw TweetException("You are already following this user");
        following[other->username] = other;
        other->followers[username] = this;
    }

    // __twittar__

    Inbox& getInbox() {
        return inbox;
    }

    void sendTweet(Tweet * msg) {
        inbox.storeInTimeline(msg);
        inbox.storeInMyTweets(msg);
        for(auto& follower : followers)
            follower.second->getInbox().storeInTimeline(msg);
    }

    // __unfollow__

    void unfollow(std::string username) {
        if(following.count(username) == 0)
            throw TweetException("You are not following this user");
        auto other = following[username];
        
        this->inbox.rmMsgsFrom(username);
        following.erase(username);

        other->followers.erase(this->username);
    }

    // __like__

    void like(int twId) {
        inbox.getTweet(twId)->like(username);
    }

    // __remover__
    void unfollowAll() {
        // for(auto& [username, user] : following) {
        //     user->followers.erase(this->username);
        // }
        for (auto it = following.begin(); it != following.end(); ++it) {
            it->second->followers.erase(this->username);
        }
        following.clear();
    }

    void rejectAll() {
        // for(auto& [username, user] : followers) {
        //     user->following.erase(this->username);
        // }
        for (auto it = followers.begin(); it != followers.end(); ++it) {
            it->second->following.erase(this->username);
        }
        followers.clear();
    }
};


std::ostream& operator<<(std::ostream& os, User user)                  { return os << user.str(); }
std::ostream& operator<<(std::ostream& os, User* user)                 { return os << user->str(); }
std::ostream& operator<<(std::ostream& os, std::shared_ptr<User> user) { return os << user->str(); }


// class Controller {
class Controller {
    // ' armazena o id para o proximo tweet a ser criado
    // - nextTweetId: int
    int nextTweetId { 0 };

    // ' armazena todos os usuarios do sistema
    // - users: map<str, User>
    std::map<std::string, std::shared_ptr<User>> users;

    // ' armazena todos os tweets do sistema
    // - tweets: map<int, Tweet>
    std::map<int, std::shared_ptr<Tweet>> tweets;

    // __
public:

    Controller() {}

    void addUser(std::string username) {
        if (users.count(username) == 1)
            throw TweetException("fail: usuario ja existe");
        users[username] = std::make_shared<User>(username);
    }

    friend std::ostream& operator<<(std::ostream& os, const Controller& ctrl);

    // __twittar__

private:
    Tweet* createTweet(std::string username, std::string msg) {
        return new Tweet(nextTweetId++, username, msg);
    }
public:

    User* getUser(std::string username) {
        auto it = users.find(username);
        if (it == users.end())
            throw TweetException("fail: usuario nao encontrado");
        return it->second.get();
    }

    void sendTweet(std::string username, std::string msg) {
        auto it = getUser(username);
        auto tweet = createTweet(username, msg);
        this->tweets[tweet->getId()] = std::shared_ptr<Tweet>(tweet);
        it->sendTweet(tweet);
    }

    // __retweet__

    void sendRt(std::string username, int twId, std::string msg) {
        auto it = this->tweets.find(twId);
        if (it == this->tweets.end())
            throw TweetException("fail: tweet nao encontrado");
        auto user = this->getUser(username);
        auto rt = this->createTweet(username, msg);
        rt->setRt(it->second.get());
        this->tweets.insert(std::make_pair(nextTweetId, rt));
        user->sendTweet(rt);
        nextTweetId++;
        // User* user = this->getUser(username);
        // if (user != nullptr) {
        //     Tweet* tweet = this->createTweet(username, msg);
        //     auto rTw = this->tweets.find(twId);
        //     tweet->setRt(&*rTw->second);
        //     user->sendTweet(tweet);
        //     this->tweets.insert(std::pair<int, std::shared_ptr<Tweet>>(rTw->second->getId(), rTw->second));
        // }
    }

    // __remover__

    void rmUser(std::string username) {
        if (users.count(username) == 0)
            throw TweetException("fail: usuario nao existe");
        auto it = users.find(username);
        it->second->unfollowAll();
        it->second->rejectAll();
        users.erase(it);
    }
};


std::ostream& operator<<(std::ostream& os, const Controller& ctrl) {
    return os << aux::join(aux::map(ctrl.users, [](auto p) {return *p.second;}), "\n");
}


using namespace aux;
using namespace aux;

int main() {
    Controller sistema;

    while(true) {
        auto line = input();
        write("$" + line);
        auto args = split(line, ' ');
        
        try {
            if      (args[0] == "end"     ) { break;                                                                     } 
            else if (args[0] == "add"     ) { sistema.addUser(args[1]);                                                  } 
            else if (args[0] == "rm"      ) { sistema.rmUser(args[1]);                                                   }
            else if (args[0] == "show"    ) { write(sistema);                                                            }
            else if (args[0] == "follow"  ) { sistema.getUser(args[1])->follow(sistema.getUser(args[2]));                }
            else if (args[0] == "unfollow") { sistema.getUser(args[1])->unfollow(args[2]);                               }
            else if (args[0] == "twittar" ) { sistema.sendTweet(args[1], join(slice(args, 2), " "));                     }
            else if (args[0] == "like"    ) { sistema.getUser(args[1])->like((int) +(args[2]));                     }
            else if (args[0] == "timeline") { write(join(sistema.getUser(args[1])->getInbox().getTimeline(), "\n"));     } 
            else if (args[0] == "rt"      ) { sistema.sendRt(args[1], (int) +(args[2]), join(slice(args, 3), " ")); } 
            else                            { write("fail: comando invalido");                                           }
        } catch (TweetException &e) {
            std::cout << e.what() << '\n';
        }
    }
    return 0;
}



