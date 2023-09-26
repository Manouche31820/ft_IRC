# include "../../headers/server/Server.hpp"

	Server::Server(const std::string & addr, const unsigned short port, const std::string password) : Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), 
		_addr(addr), _password(password), _port(port), _admin("admin") {
			static const int    optval = true;
			setSocket(AF_INET, _addr.c_str(), _port);
			setSockOpt(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
			bindSocket();
			listenSocket(MAX_LISTEN);
		}

	Server::~Server(){
		for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			delete *it;
	}

	std::vector<std::string>	Server::split(std::string line, std::string delimiteur) {
		std::vector<std::string> split;
		size_t	pos;
		while ((pos = line.find(delimiteur)) != std::string::npos) {
			split.push_back(line.substr(0, pos));
			line.erase(0, pos + delimiteur.size());
		}
		if (!line.empty())
			split.push_back(line);
		return (split);
	}
	std::string Server::getCurrentDate()
	{
	    std::time_t now = std::time(0);

	    std::tm* localTime = std::localtime(&now);
	    char buffer[100];
	    std::strftime(buffer, sizeof(buffer), "%a %b %d %T %Y", localTime);
	    return std::string(buffer);
	}

	void Server::getRequest(const char * str, std::map<std::string, std::vector<std::string> > &request, 
		const Client &cl)
	{
		static const char	*saveMethod[] = {"SERVER", "CAP", "NICK", "REAL", "USER", "PASS", NULL};
		// static const char	*command[] = {"/JOIN", "/TOPIC", "/PRIVMSG", "/QUIT", "/KICK", "/INVITE", "/MODE", NULL};
		std::string 		line(str);

		put_line(line);
		if (cl.getStatus() != Client::disconnected) {
			for (iterator_map it = request.begin(); it != request.end(); ++it) {
				int	i = 0;
				for (; saveMethod[i]; ++i)
					if (it->first == saveMethod[i])
						break ;
				if (!saveMethod[i]) {
					request.erase(it->first);
					it = request.begin();
					if (request.empty())
						break ;
				}
			}
		}
		else{
			request.clear();
		}
		std::vector<std::string> line_split = split(line, "\r\n");
		for (std::vector<std::string>::const_iterator itLineSplit = line_split.begin()
			; itLineSplit != line_split.end(); ++itLineSplit) {
			std::vector<std::string> line_split_with_space = split(*itLineSplit, " ");
			std::string key = line_split_with_space.at(0);
			line_split_with_space.assign(line_split_with_space.begin() + 1, line_split_with_space.end());
			request.insert(std::make_pair(key, line_split_with_space));
		}
		std::cout << " ---------------- BEGIN OF REQUEST --------------------" << std::endl;
		for (iterator_map it = request.begin(); it != request.end(); ++it) {
			std::cout << "["; put_line(it->first); std::cout << "]:";
			for (std::vector<std::string>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				std::cout << " ["; put_line(*it2); std::cout << "]";
			}
			std::cout << std::endl;
		}	
		std::cout << " ---------------- END OF REQUEST -------------------- \n" << std::endl;
	}
	// :127.0.0.1 464 * :Authentication failed. Connection refused.\r\n
	// "001" : Renvoyé lorsqu'un Client s'est correctement connecté et est le premier message de bienvenue.

	bool	Server::check_connection_error( Client &cl ) {
		// PASSWORD CHECK
		if (!_password.empty()) {
			if (!cl.getRequest().count("PASS"))
				return (lp_self(cl, getReplies(cl, 461, "PASS")), false);
			if (cl.getRequest().at("PASS").at(0) != _password)
				return (lp_self(cl, getReplies(cl, 464)), false);
		}
		else if (cl.getRequest().count("PASS"))
			return (lp_self(cl, getReplies(cl, 462, "PASS")), false);
		// USER CHECK
		if (!cl.getRequest().count("NICK"))
			return (lp_self(cl, getReplies(cl, 431)), false);
		if (cl.getRequest().at("NICK").at(0).size() > 50
			|| cl.getRequest().at("NICK").at(0).find_first_not_of(NICKNAME_VALID_CHAR) != std::string::npos)
			return (lp_self(cl, getReplies(cl, 432, cl.getRequest().at("NICK").at(0))), false);
		for (std::vector<Client *>::iterator itClient = _clients.begin(); itClient != _clients.end(); ++itClient) {
			if ((*itClient)->getStatus() == Client::connected) {
				if ((*itClient)->getRequest().at("NICK").at(0) == cl.getRequest().at("NICK").at(0))
					return (lp_self(cl, getReplies(cl, 433, cl.getRequest().at("NICK").at(0))), false);
			}
		}
		return (true);
	}

	bool	Server::connection( Client &cl ) {
		if (cl.getStatus() == Client::connected)
			return (true);
		if (check_connection_error(cl) == false)
			return (false);
		cl.setStatus(Client::connected);
		if (cl.getRequest().at("NICK").at(0) == _admin)
			cl.setRights(Client::operators);
		lp_self(cl, getReplies(cl, 001, cl.getRequest().at("NICK").at(0) + "!" + cl.getRequest().at("USER").at(0) + "@" + cl.getRequest().at("USER").at(2)));
		lp_self(cl, getReplies(cl, 002, cl.getRequest().at("USER").at(2), "10.2"));
		lp_self(cl, getReplies(cl, 003, getCurrentDate()));
		lp_self(cl, getReplies(cl, 004, cl.getRequest().at("USER").at(2), "10.2", "o", "itkol"));
		// for(std::vector<std::string>::const_iterator msg = cl.getMessage().begin(); msg != cl.getMessage().end(); ++msg)
			 // put_line(*msg);
		return (false);
	}

	void Server::createRequest( Client &cl ) {
		static const char *str[] = {"JOIN", "TOPIC", "PRIVMSG", "QUIT", "KICK", "INVITE", "MODE", NULL};
		if (connection(cl)) {
			for (iterator_map it = cl.getRequest().begin(); it != cl.getRequest().end(); ++it) {
				int i;
				for (i = 0; str[i]; ++i)
					if (it->first == str[i])
						break ;
				switch (i) {
					case 0: channelJoin(cl); break ;
					case 1: channelTopic(cl); break ;
					case 2: channelPriv(cl); break ;
                    case 3: channelQuit(cl); break ;
                    case 4: channelKick(cl); break ;
					// case 2: channelPriv(cl, ret); break ;
					// case 3: std::cout << str[i] << std::endl; break ;
					// case 4: std::cout << str[i] << std::endl; break ;
					// case 5: std::cout << str[i] << std::endl; break ;
					// case 6: std::cout << str[i] << std::endl; break ;
					default:
						break ;
						// std::cerr << "Invalid request: " << it->first << " - " << it->second << std::endl;
				};
			}
		}
	}


	void Server::addClient(Client * clients){
		_clients.push_back(clients);
	}

	const std::vector<Client *> & Server::getClient(void) const{
		return(_clients);
	}

	const std::string & Server::getPassword(void) const{
		return(_password);
	}
