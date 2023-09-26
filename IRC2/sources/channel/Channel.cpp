# include "../../headers/channel/Channel.hpp"

	Channel::Channel(const std::string & channelname, const std::string password) 
		: _channelName(channelname), _nbrClients(0), /*_rights(STANDARD),*/
		_channelPassword(password), _connectedUsers(), _userList(), _adminList(),  
		_topic("Mon topic est les poulets ont flambes."), _invitedUsers(){
	}

	Channel::~Channel(void){}

	//void Channel::setRights(const Channel::ch_mod & rights){ _rights = rights; }
	std::set<std::string> & Channel::setConnected(void){ return(_connectedUsers);}
	std::map<std::string, Channel::ch_rights> & Channel::setList(void){ return(_userList);}
	void Channel::setNbr(size_t nbrClients){ _nbrClients = nbrClients;}
	void Channel::setTopic(std::string topic) { _topic = topic; }
	void Channel::setInvite(std::string user) { _invitedUsers.insert(user);}
	bool Channel::inviteStatus(std::string username) {
		for (std::set<std::string>::iterator invit = _invitedUsers.begin(); invit != _invitedUsers.end(); ++invit)
		{
			if(username == *invit)
				return(true);
		}
		return (false);
	}
	void Channel::setChannelPass(std::string channelPass){ _channelPassword = channelPass;};
	void Channel::addConnectedUser(std::string username){ _connectedUsers.insert(username);} 
	const std::string & Channel::getName(void) const { return(_channelName);}
	const std::string & Channel::getPassword(void) const { return(_channelPassword);}
	std::string Channel::getTopic(void) const { return (_topic);}
	const std::set<std::string> & Channel::getConnected(void) const{ return(_connectedUsers);}
	const std::set<std::string> & Channel::getInvitedUsers(void) const { return _invitedUsers;}
	const std::map<std::string, Channel::ch_rights> & Channel::getList(void) const{ return(_userList);}
	//const Channel::ch_mod & Channel::getRights(void) { return (_rights);}
	void Channel::setChanAdmin(std::string username){ (_adminList.push_back(username));}
    const std::list<std::string> & Channel::getChanAdmin(void) const{ return(_adminList);}
    const std::string & Channel::getChannelPass(void) const{ return (_channelPassword);}
    int	 Channel::isOperator(std::string User) const
	{
		// parcourir la liste des admins et vérifier si User en fait partie.
		for (std::list<std::string>::const_iterator itAdmin = this->_adminList.begin(); itAdmin != this->_adminList.end(); ++itAdmin)
		{
			if (User == *itAdmin)
				return (OPERATOR);
		}
		return (REGULAR);
	} // fonction qui determine si un client (User ) est opérateur dans un channel donné (this).
	
	size_t Channel::getNbr(void) const{ return(_nbrClients);}


	void 	Channel::setMode(int mode, int action, int limit, std::string arg) // actio SET/UNSET
	{
		(void)arg;
		(void)limit;
		(void)action;
		if (mode == INVITE_ONLY_MODE)
		{
			this->invite_only = action;
		}
		if (mode == TOPIC_RESTRICT_MODE)
		{
			this->topic_restriction = action;
		}
		if (mode == PASSWORD)
		{
			if (action == UNSET)
				this->_channelPassword = "";
			else
				this->_channelPassword = arg;
		}
		if (mode == LIMIT)
		{
			if (action == UNSET)
				_nbrClients = 0;
			if (action == SET)
				_nbrClients = limit;
		}
		if (mode == PRIVILEGE)
		{
			std::list<std::string>::iterator it;
			for (it = _adminList.begin(); it != _adminList.end(); it++)
			{
				if (*it == arg)
					_adminList.erase(it);
			}
		}
	}

	int 	Channel::getMode(int mode, std::string arg) // actio SET/UNSET
	{
		(void)arg;
		if (mode == INVITE_ONLY_MODE)
			return (this->invite_only);
		if (mode == LIMIT)
			return (this->_nbrClients);
		return (-1);
	}

	bool	Channel::isConnected(std::string user) const{
		for (std::set<std::string>::const_iterator itCo = _connectedUsers.begin(); itCo != _connectedUsers.end(); ++itCo)
		{
			if (user == *itCo)
				return (true);
		}
		return (false);
	}
