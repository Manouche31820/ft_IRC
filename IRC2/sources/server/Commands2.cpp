# include "../../headers/server/Server.hpp"

	
	void Server::execTopic(Client &cl, std::string channel, std::string newtopic){
		size_t pos = newtopic.find("""");
		std::string check = """";
		Channel *toJoin= getChanByName(channel);
		if (getChanByName(channel) == NULL)
			lp_self(cl, getReplies(cl, 442, channel));
		else{
			if ((toJoin->getTopic() == "" && newtopic.size() == 0) || (newtopic.size() == 2 && pos != std::string::npos)){
				if (toJoin->isOperator(cl.getName()) == OPERATOR){
					toJoin->setTopic("");
					std::string msg = std::string(":") + SERVER_NAME + " 332 " + " : " +  toJoin->getName() + " : " + toJoin->getTopic();
					lp_channel(cl, *toJoin, msg);
					lp_self(cl, msg);
				}
				else{
					std::string msg = std::string(":") + SERVER_NAME + " 482 " + " : " +  toJoin->getName() + " : " + "You're not channel operator\r\n";
					lp_self(cl, msg);
				}

			}
			else if (newtopic.size() > 0 && (toJoin->isOperator(cl.getName()) == OPERATOR) && channel.size() != 0){
				if (toJoin->isOperator(cl.getName()) == OPERATOR){
					toJoin->setTopic(newtopic);
					std::string msg = std::string(":") + SERVER_NAME + " 332 " + " : " +  toJoin->getName() + " : " + toJoin->getTopic();
					lp_self(cl, msg);
				}
				else{
					std::string msg = std::string(":") + SERVER_NAME + " 482 " + " : " +  toJoin->getName() + " : " + "You're not channel operator\r\n";
					lp_self(cl, msg);
				}
			}
			else if (newtopic.size() != 0 && (toJoin->isOperator(cl.getName()) != OPERATOR)){
				std::string msg = std::string(":") + SERVER_NAME + " 482 " + " : " +  toJoin->getName() + " : " + "You're not channel operator\r\n";
				lp_self(cl, msg);
			}
			else{
				std::string msg = std::string(":") + SERVER_NAME + " 332 " + " : " +  toJoin->getName() + " : " + toJoin->getTopic();
				lp_self(cl, msg);
			}

		}

	}


//rajouter l option t du MODE -----------------------------------POUR L INSTANT TOUS LES ADMINS PEUVENT CHANGER LE TOPIC---------------------------------------------------------
	void Server::channelTopic(Client &cl){
			std::vector<std::string> request = cl.getRequest().at("TOPIC");
			std::string newtopic;
			// std::cout <<  "size : " << request.size() << std::endl;
			for(std::vector<std::string>::iterator topic = request.begin(); topic != request.end(); ++topic){
				if (request.size() == 1)
					break;
				if (topic != request.begin() && topic + 1 != request.end())
					newtopic += *topic + " ";
				else if (topic + 1 == request.end())
					newtopic += *topic;
			}
			if (newtopic.size() > 0 && newtopic.at(0) == ':')
				newtopic = newtopic.substr(1, newtopic.size() - 1);
			// std::cout << "newtopic : [" << newtopic << "]" << std::endl;
			execTopic(cl, request.at(0), newtopic);
	}
	Client *Server::getClientByName(const std::string &nameOfClient)
	{
		for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it){
			if ( (*it)->getName() == nameOfClient)
				return ((*it));
		}
		// std::cout << "No corresponding channel found in _client getchanbyClient() " << std::endl; // DEBUG DELETE BEFORE PUSHING
		return (NULL);
	}

		void Server::channelPriv(Client &cl){

			Channel *toJoin = NULL;
			Client *receiver = NULL;
			std::vector<std::string> request = cl.getRequest().at("PRIVMSG");
			// std::cout <<  "request " << request.at(0) << std::endl;
			if (request.size() > 0){
				toJoin= getChanByName(cl.getRequest().at("PRIVMSG").at(0));
				receiver = getClientByName(cl.getRequest().at("PRIVMSG").at(0));
			}

			std::string newmessage;
			// std::cout <<  "size : " << request.size() << std::endl;
			for(std::vector<std::string>::iterator newmsg = request.begin(); newmsg!= request.end(); ++newmsg){
				if (request.size() == 1)
					break;
				if (newmsg != request.begin() && newmsg + 1 != request.end())
					newmessage += *newmsg + " ";
				else if (newmsg + 1 == request.end())
					newmessage += *newmsg;
			}
			if (newmessage.at(0) == ':')
				newmessage = newmessage.substr(1, newmessage.size() - 1);
			// std::cout <<  "newmessage " << newmessage << std::endl;
			if (request.size() == 0)
				lp_self(cl, getReplies(cl, 411, "PRIVMSG"));
			else if (request.size() == 1 && toJoin != NULL)
				lp_self(cl, getReplies(cl, 412));
			else if (toJoin == NULL && receiver == NULL)
				lp_self(cl, getReplies(cl, 411, "PRIVMSG"));
			else if (toJoin != NULL && receiver == NULL ){
				std::string msg = std::string(":") + cl.getName() + " PRIVMSG " + toJoin->getName() + " : " + newmessage;
				lp_channel(cl, *toJoin, msg);
			}
			else if(receiver != NULL && toJoin == NULL){
				std::string msg = std::string(":") + cl.getName() + " PRIVMSG " + ": " + newmessage;
				lp_private(cl, receiver->getName(), msg);
			}
	}

	void Server::channelQuit(Client &cl){
		std::vector<std::string> request = cl.getRequest().at("QUIT");
		std::string msg = "ERROR";
		lp_self(cl, msg);
	}

	// void	Server::kickChan(Client &cl, Channel &chan) {
	// 	if (!chan.getConnected().count(cl.getRequest().at("NICK").at(0)))
	// 		return ;
	// 	chan.setConnected().erase(cl.getRequest().at("NICK").at(0));
	// 	lp_channel(cl, chan, " PART " + chan.getName() + " :" + cl.getRequest().at("NICK").at(0) + " has been kicked.");
	// }

	void Server::channelKick(Client &cl)
	{
		// (void) cl;
		if (cl.getRequest().at("KICK").size() < 3){
			lp_self(cl, getReplies(cl, 461, "KICK"));
			return ;
		}

		std::string request = cl.getRequest().at("KICK").at(1);
		std::string newmessage;
		Channel *toKick = NULL;
		for (size_t pos = request.find(":"); pos != std::string::npos; pos = request.find(":", pos))
   			 request.erase(pos, 1);
			cl.setRequest().at("KICK").at(1) = request;
		request = cl.getRequest().at("KICK").at(2);
		for (size_t pos = request.find(":"); pos != std::string::npos; pos = request.find(":", pos))
   			 request.erase(pos, 1);
			cl.setRequest().at("KICK").at(2) = request;
		std::vector<std::string> 	listChannel = split(cl.getRequest().at("KICK").at(1), ",");
		std::vector<std::string> 	listUsers = split(cl.getRequest().at("KICK").at(2), ",");
		if (cl.getRequest().at("KICK").size() > 3){
			size_t i = 3;
			while (i != cl.getRequest().at("KICK").size()){
				if (i + 1 != cl.getRequest().at("KICK").size())
					newmessage += cl.getRequest().at("KICK").at(i) + " ";
				else
					newmessage += cl.getRequest().at("KICK").at(i);
				i++;
			}
		}
		else
			newmessage = "get kicked";
		toKick = getChanByName(listChannel.at(0));
			std::cout << "size channel : " << listChannel.size() << "size userlist :" <<  listUsers.size() << std::endl;
			if (listChannel.size() == 1 && toKick != NULL && toKick->isOperator(cl.getName()) == OPERATOR){
				std::cout << "size channel : " << listChannel.size() << "size userlist :" <<  listUsers.size() << std::endl;
					for(std::vector<std::string>::iterator it = listUsers.begin(); it != listUsers.end(); ++it){
						if(toKick->isConnected(*it) == true){
							Client *self = getClientByName(*it);
							toKick->setConnected().erase(*it);
							std::string msg = std::string(":") + cl.getName() + " KICK " + toKick->getName() + " " + self->getName() + " :" + newmessage;
							lp_private(cl, self->getName(), msg);
							std::string listConnectedUser;
							for (std::set<std::string>::iterator itUser = _channels.back().getConnected().begin(); itUser != _channels.back().getConnected().end(); ++itUser)
							{
								if (_channels.back().isOperator(*itUser) == OPERATOR) // fonction qui determine si un client est opérateur dans un channel donné.
									listConnectedUser += "@";
								listConnectedUser += *itUser + " ";
							}
							msg = std::string(":") + SERVER_NAME + std::string(" 353 ") + cl.getRequest().at("NICK").at(0) + " = " + toKick->getName() + " :" + listConnectedUser + "\r\n";
    						lp_self(cl, msg);
    						msg = std::string(":") + SERVER_NAME + std::string(" 366 ") + cl.getRequest().at("NICK").at(0) + " " + toKick->getName() + " :End of NAMES list" + "\r\n";
    						lp_self(cl, msg);
						}
						else
							lp_self(cl, getReplies(cl, 441, cl.getName(), toKick->getName()));
					}
			}
			else if (listChannel.size() > 1)
			{
				std::cout << "else1" <<std::endl;
					// ERR_CHANOPRIVSNEEDED
			}

		// if (listChannel.size() > 1)
		// {
		// 	if (listChannel.size() == listUsers.size())
		// 	{
		// 		for(std::vector<std::string>::iterator kickChan = ListChannel.begin(); kickChan != listChannel.end(); ++kickChan)
		// 		{
		// 			if ((toKick = getChanByName(kickChan->getName())) != NULL)
		// 				if (toKick.isOperator() == OPERATOR)
		// 					lp_private()
		// 				else

		// 			}
		// 			else
		// 				ERR_NOSUCHCHANNEL

		// 		}
		// }

	}